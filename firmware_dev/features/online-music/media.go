package main

import (
	"context"
	"errors"
	"fmt"
	"github.com/guohuiyuan/music-lib/kugou"
	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/netease"
	"github.com/guohuiyuan/music-lib/provider"
	"github.com/guohuiyuan/music-lib/qq"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"time"
)

type lyricLine struct {
	Millis int
	Text   string
}

func parseLyrics(raw string) []lyricLine {
	var out []lyricLine
	offset := 0
	for _, line := range strings.Split(raw, "\n") {
		if strings.HasPrefix(line, "[offset:") {
			offset, _ = strconv.Atoi(strings.TrimSuffix(strings.TrimPrefix(strings.TrimSpace(line), "[offset:"), "]"))
			continue
		}
		var times []int
		for strings.HasPrefix(line, "[") {
			end := strings.IndexByte(line, ']')
			if end < 0 {
				break
			}
			stamp := strings.SplitN(line[1:end], ":", 2)
			if len(stamp) == 2 {
				m, e := strconv.Atoi(stamp[0])
				sec, e2 := strconv.ParseFloat(stamp[1], 64)
				if e == nil && e2 == nil && m >= 0 && m < 1000 && sec >= 0 && sec < 60 {
					times = append(times, m*60000+int(sec*1000))
				}
			}
			line = line[end+1:]
		}
		// Providers return verbatim LRC: interior/end word timestamps are markup,
		// not text. The first stamp remains this line's playback time.
		var plain strings.Builder
		for len(line) > 0 {
			if line[0] == '[' || line[0] == '<' {
				closing := byte(']')
				if line[0] == '<' {
					closing = '>'
				}
				if end := strings.IndexByte(line, closing); end >= 0 && strings.Contains(line[1:end], ":") {
					line = line[end+1:]
					continue
				}
			}
			plain.WriteByte(line[0])
			line = line[1:]
		}
		text := clean(plain.String())
		if text == "" {
			continue
		}
		for _, t := range times {
			out = append(out, lyricLine{t, text})
			if len(out) >= 600 {
				break
			}
		}
		if len(out) >= 600 {
			break
		}
	}
	for i := range out {
		out[i].Millis += offset
		if out[i].Millis < 0 {
			out[i].Millis = 0
		}
	}
	sort.SliceStable(out, func(i, j int) bool { return out[i].Millis < out[j].Millis })
	return out
}
func (s *server) fetchLyrics(song model.Song, source string, rev uint64) {
	select {
	case s.lyricGate <- struct{}{}:
		defer func() { <-s.lyricGate }()
	case <-time.After(15 * time.Second):
		return
	}
	// Each lyric job gets an immutable credential snapshot. It never holds the
	// interactive provider gate or races a login's cookie update.
	bcookie, _ := os.ReadFile(filepath.Join(s.credentials, source+".cookie"))
	cookie := strings.TrimSpace(string(bcookie))
	var p provider.MusicProvider
	switch source {
	case "netease":
		p = netease.New(cookie)
	case "qq":
		p = qq.New(cookie)
	case "kugou":
		p = kugou.New(cookie)
	default:
		return
	}
	raw, e := p.GetLyrics(&song)
	lines := parseLyrics(raw)
	var b strings.Builder
	fmt.Fprintf(&b, "%d\n", rev)
	if e != nil || len(lines) == 0 {
		b.WriteString("0\t暂无歌词\n")
	} else {
		for _, l := range lines {
			fmt.Fprintf(&b, "%d\t%s\n", l.Millis, l.Text)
		}
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.player.Revision == rev {
		atomicWrite(filepath.Join(s.dir, "lyrics"), []byte(b.String()))
	}
}
func (s *server) publishQueue(seq uint64) {
	s.mu.Lock()
	p := s.player
	s.mu.Unlock()
	var b strings.Builder
	fmt.Fprintf(&b, "%d\tqueue\t%d\t当前播放队列\n@\t%d\tfalse\n", seq, p.Revision, p.Index)
	for i, t := range p.Queue {
		fmt.Fprintf(&b, "%d\t%s\t%s\n", i, clean(t.Name), clean(t.Artist))
	}
	atomicWrite(filepath.Join(s.dir, "response"), []byte(b.String()))
	urls := make([]string, len(p.Queue))
	for i, song := range p.Queue {
		urls[i] = song.Cover
	}
	s.startBrowseArt("queue", p.Revision, urls)
}
func (s *server) queuePlay(rev uint64, index int) error {
	s.mu.Lock()
	p := s.player
	s.mu.Unlock()
	if p.Revision != rev {
		return errors.New("播放队列已更新，请重开队列")
	}
	return s.startTrack(p.Queue, p.Source, p.List, index, p.Control, 0)
}

// Exiting the platform relinquishes only its own renderer stream.
func (s *server) exitPlayback() error {
	s.cancelPrefetch()
	s.mu.Lock()
	p := s.player
	s.mu.Unlock()
	if p.URI != "" {
		ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
		defer cancel()
		f, e := soapFields(ctx, p.Control, "GetPositionInfo")
		if e == nil && f["TrackURI"] == p.URI {
			if e = soap(ctx, p.Control, "Stop", ""); e != nil {
				return e
			}
		}
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	s.player = playbackState{Revision: p.Revision + 1, State: "STOPPED", Mode: p.Mode}
	if s.artCancel != nil {
		s.artCancel()
	}
	s.artKey = ""
	for _, name := range []string{"active", "nowplaying", "lyrics", "cover.revision"} {
		os.Remove(filepath.Join(s.dir, name))
	}
	return nil
}

// Progress fallback counts frames consumed by ALSA, never advances on wall time.
func nativePCMFrames() (uint64, int, bool) {
	p := nativePCMObservation()
	return p.Frames, p.Rate, p.Valid
}
