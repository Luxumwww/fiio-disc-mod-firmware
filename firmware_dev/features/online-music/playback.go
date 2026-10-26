package main

import (
	"context"
	"encoding/xml"
	"errors"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"io"
	"math/rand"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

type playlistContext struct {
	ID, Title string
	Page      int
	Total     int
	More      bool
}
type playbackState struct {
	Queue                        []model.Song
	Source                       string
	List                         playlistContext
	Index                        int
	Control                      string
	Song                         model.Song
	State, Mode, Format, Message string
	Position, Duration           int
	Rate, Bits                   int
	Revision                     uint64
	SeenPlaying                  bool
	AutoRetries                  int
	URI                          string
	StoppedCount                 int
	StartedAt                    time.Time
	ForeignURI                   string
	ForeignCount                 int
	PCMFrames                    uint64
	PCMSeconds                   float64
	PCMTrigger                   string
	SeenPCM                      bool
	ClosedPCMCount               int
}

func cloneSong(a model.Song) model.Song {
	b := a
	b.Extra = map[string]string{}
	for k, v := range a.Extra {
		b.Extra[k] = v
	}
	return b
}
func (s *server) songURL(source string, song *model.Song) (string, error) {
	s.mu.Lock()
	q := s.quality
	s.mu.Unlock()
	return s.songURLQuality(source, song, q)
}
func (s *server) songURLQuality(source string, song *model.Song, q string) (string, error) {
	*song = cloneSong(*song)
	song.Extra["disc_quality"] = q
	if source == "netease" {
		song.Extra["netease_level"] = q
	}
	if s.providers[source] == nil {
		return "", errors.New("不支持的平台")
	}
	return s.providers[source].GetDownloadURL(song)
}
func audioFormat(b []byte) (string, int, int) {
	if len(b) >= 4 && string(b[:4]) == "fLaC" {
		rate, bits := 0, 0
		if len(b) >= 26 && b[4]&127 == 0 {
			rate = int(b[18])<<12 | int(b[19])<<4 | int(b[20])>>4
			bits = ((int(b[20])&1)<<4 | int(b[21])>>4) + 1
		}
		return "flac", rate, bits
	}
	if len(b) >= 3 && string(b[:3]) == "ID3" || len(b) >= 2 && b[0] == 255 && b[1]&224 == 224 {
		return "mp3", 0, 0
	}
	if len(b) >= 4 && string(b[:4]) == "OggS" {
		return "ogg", 0, 0
	}
	if len(b) >= 12 && string(b[4:8]) == "ftyp" {
		return "m4a", 0, 0
	}
	if len(b) >= 12 && string(b[:4]) == "RIFF" && string(b[8:12]) == "WAVE" {
		return "wav", 0, 0
	}
	if len(b) >= 4 && string(b[:4]) == "MAC " {
		return "ape", 0, 0
	}
	return "", 0, 0
}
func (s *server) probeAudio(ctx context.Context, u string) (string, int, int, error) {
	req, e := http.NewRequestWithContext(ctx, "GET", u, nil)
	if e != nil {
		return "", 0, 0, e
	}
	req.Header.Set("Range", "bytes=0-63")
	req.Header.Set("Accept-Encoding", "identity")
	resp, e := s.client.Do(req)
	if e != nil {
		return "", 0, 0, e
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 && resp.StatusCode != 206 {
		return "", 0, 0, fmt.Errorf("音频地址返回 %d", resp.StatusCode)
	}
	b, e := io.ReadAll(io.LimitReader(resp.Body, 64))
	if e != nil {
		return "", 0, 0, e
	}
	ext, rate, bits := audioFormat(b)
	if ext == "" {
		return "", 0, 0, errors.New("未取得可识别的音频，可能是权限或格式问题")
	}
	return ext, rate, bits, nil
}
func (s *server) startTrack(queue []model.Song, source string, list playlistContext, index int, control string, seq uint64) error {
	s.cancelPrefetch()
	if index < 0 || index >= len(queue) {
		return errors.New("没有可播放歌曲")
	}
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return errors.New("正在处理，请稍后")
	}
	if b, e := os.ReadFile(filepath.Join(s.dir, "request")); e == nil && strings.Contains(string(b), "\texit") {
		return errors.New("在线播放已退出")
	}
	song := queue[index]
	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	s.mu.Lock()
	q := s.quality
	initialRevision := s.player.Revision
	s.mu.Unlock()
	resolvedAt := time.Now()
	u, ext, rate, bits, e := s.resolvePlaybackAudio(ctx, source, &song, q)
	s.recordTiming("resolve_audio", resolvedAt, e)
	if e != nil {
		return e
	}
	song.Ext = ext
	if seq != 0 && !s.currentRequest(seq) {
		return errors.New("已取消选歌")
	}
	if b, e := os.ReadFile(filepath.Join(s.dir, "request")); e == nil && strings.Contains(string(b), "\texit") {
		return errors.New("在线播放已退出")
	}
	s.mu.Lock()
	unchanged := s.player.Revision == initialRevision
	s.mu.Unlock()
	if !unchanged {
		return errors.New("播放状态已更新，请重试")
	}
	controlAt := time.Now()
	control, e = s.ensureControl(ctx, control)
	s.recordTiming("verify_control", controlAt, e)
	if e != nil {
		return e
	}
	if seq != 0 && !s.currentRequest(seq) {
		return errors.New("已取消选歌")
	}
	if b, e := os.ReadFile(filepath.Join(s.dir, "request")); e == nil && strings.Contains(string(b), "\texit") {
		return errors.New("在线播放已退出")
	}
	s.mu.Lock()
	if s.player.Revision != initialRevision {
		s.mu.Unlock()
		return errors.New("播放状态已更新，请重试")
	}
	s.streamID++
	id := s.streamID
	s.rememberStreamLocked(id, u, song, source, q)
	s.mu.Unlock()
	pcmStart, _, _ := nativePCMFrames()
	proxy := fmt.Sprintf("http://127.0.0.1:%d/audio/%d.%s", s.port, id, ext)
	mime := map[string]string{"flac": "audio/flac", "mp3": "audio/mpeg", "m4a": "audio/mp4", "ogg": "audio/ogg", "wav": "audio/wav", "ape": "audio/ape"}[ext]
	meta := `<DIDL-Lite xmlns="urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:upnp="urn:schemas-upnp-org:metadata-1-0/upnp/"><item id="online" parentID="0" restricted="1"><dc:title>` + xmlText(song.Name) + `</dc:title><upnp:artist>` + xmlText(song.Artist) + `</upnp:artist><upnp:class>object.item.audioItem.musicTrack</upnp:class><res protocolInfo="http-get:*:` + mime + `:*" duration="` + fmt.Sprintf("%d:%02d:%02d", song.Duration/3600, song.Duration/60%60, song.Duration%60) + `">` + xmlText(proxy) + `</res></item></DIDL-Lite>`
	setAt := time.Now()
	e = soap(ctx, control, "SetAVTransportURI", "<CurrentURI>"+xmlText(proxy)+"</CurrentURI><CurrentURIMetaData>"+xmlText(meta)+"</CurrentURIMetaData>")
	s.recordTiming("set_uri", setAt, e)
	if e != nil {
		return e
	}
	if seq != 0 && !s.currentRequest(seq) {
		return errors.New("已取消选歌")
	}
	playAt := time.Now()
	e = soap(ctx, control, "Play", "<Speed>1</Speed>")
	s.recordTiming("native_play", playAt, e)
	if rendererConnectionDropped(e) {
		recoveryAt := time.Now()
		control, e = s.recoverRendererPlay(ctx, control, proxy)
		s.recordTiming("native_play_recover", recoveryAt, e)
	}
	if e != nil {
		return e
	}
	s.mu.Lock()
	mode := s.player.Mode
	if mode == "" {
		mode = "order"
	}
	rev := s.player.Revision + 1
	s.player = playbackState{Queue: append([]model.Song(nil), queue...), Source: source, List: list, Index: index, Control: control, Song: song, State: "PLAYING", Mode: mode, Format: strings.ToUpper(ext), Duration: song.Duration, Rate: rate, Bits: bits, Revision: rev, URI: proxy, StartedAt: time.Now(), PCMFrames: pcmStart}
	atomicWrite(filepath.Join(s.dir, "active"), []byte("1\n"))
	s.mu.Unlock()
	s.publishPlayback()
	go s.fetchCover(song.Cover, rev)
	go s.fetchLyrics(song, source, rev)
	go s.maybePrefetch()
	return nil
}
func (s *server) control(action, value string) error {
	if action == "next" || action == "previous" || action == "quality" || action == "mode" {
		s.cancelPrefetch()
	}
	if action == "next" || action == "previous" || action == "toggle" {
		if _, e := os.Stat(filepath.Join(s.dir, "active")); e != nil {
			return errors.New("在线播放已结束，请重新选歌")
		}
	}
	switch action {
	case "mode":
		if value != "order" && value != "shuffle" && value != "repeat" {
			return errors.New("无效播放顺序")
		}
		s.mu.Lock()
		s.player.Mode = value
		s.mu.Unlock()
		return nil
	case "quality":
		if value != "auto" && value != "standard" && value != "exhigh" && value != "lossless" && value != "hires" {
			return errors.New("无效音质")
		}
		s.mu.Lock()
		old, p := s.quality, s.player
		s.quality = value
		s.mu.Unlock()
		if len(p.Queue) > 0 {
			if _, e := os.Stat(filepath.Join(s.dir, "active")); e == nil {
				if e = s.startTrack(p.Queue, p.Source, p.List, p.Index, p.Control, 0); e != nil {
					s.mu.Lock()
					s.quality = old
					s.mu.Unlock()
					return e
				}
			}
		}
		return atomicWrite(filepath.Join(s.credentials, "quality"), []byte(value))
	case "next":
		return s.advance(1, false)
	case "previous":
		return s.advance(-1, false)
	case "toggle":
		s.mu.Lock()
		p := s.player
		s.mu.Unlock()
		if len(p.Queue) == 0 {
			return errors.New("没有正在播放的歌曲")
		}
		action, args := "Pause", ""
		if p.State != "PLAYING" {
			action, args = "Play", "<Speed>1</Speed>"
		}
		ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
		defer cancel()
		if e := soap(ctx, p.Control, action, args); e != nil {
			return e
		}
		s.mu.Lock()
		s.player.State = "PAUSED_PLAYBACK"
		if action == "Play" {
			s.player.State = "PLAYING"
		}
		s.player.SeenPlaying = false
		s.player.StoppedCount = 0
		s.mu.Unlock()
		return nil
	}
	return errors.New("未知播放操作")
}
func (s *server) advance(direction int, automatic bool) error {
	s.mu.Lock()
	p := s.player
	s.mu.Unlock()
	if len(p.Queue) == 0 {
		return errors.New("没有播放队列")
	}
	index := p.Index + direction
	if automatic && p.Mode == "repeat" {
		index = p.Index
	} else if p.Mode == "shuffle" && len(p.Queue) > 1 {
		index = rand.Intn(len(p.Queue) - 1)
		if index >= p.Index {
			index++
		}
	}
	queue := p.Queue
	list := p.List
	targetPage := list.Page
	if p.Mode == "shuffle" && list.ID != "" && list.Total > 1 {
		current := (list.Page-1)*trackPageSize + p.Index
		chosen := rand.Intn(list.Total - 1)
		if chosen >= current {
			chosen++
		}
		targetPage = chosen/trackPageSize + 1
		index = chosen % trackPageSize
	}
	if targetPage != list.Page || index >= len(queue) || index < 0 {
		page := list.Page + direction
		if targetPage != list.Page {
			page = targetPage
		}
		if list.ID != "" && page >= 1 && (targetPage != list.Page || direction < 0 || list.More) {
			_, ok := s.providers[p.Source].(pagedSongs)
			if !ok {
				return errors.New("不支持队列分页")
			}
			select {
			case s.gate <- struct{}{}:
			default:
				return errors.New("正在处理，请稍后")
			}
			songs, total, e := s.playlistChunk(p.Source, list.ID, page)
			<-s.gate
			if e != nil {
				return e
			}
			if len(songs) > trackPageSize {
				return errors.New("平台未按页返回歌曲")
			}
			if len(songs) == 0 {
				return errors.New("已到歌单末尾")
			}
			queue = songs
			list.Page = page
			list.Total = total
			list.More = len(songs) == trackPageSize
			if total > 0 {
				list.More = page*trackPageSize < total
			}
			if targetPage == p.List.Page {
				index = 0
			}
			if direction < 0 && targetPage == p.List.Page {
				index = len(queue) - 1
			}
		} else {
			if automatic {
				return errors.New("队列已播放完")
			}
			if index < 0 {
				index = 0
			} else {
				index = len(queue) - 1
			}
			if index == p.Index {
				return errors.New("已到队列边界")
			}
		}
	}
	return s.startTrack(queue, p.Source, list, index, p.Control, 0)
}
func soapFields(ctx context.Context, control, action string) (map[string]string, error) {
	body := `<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:` + action + ` xmlns:u="urn:schemas-upnp-org:service:AVTransport:1"><InstanceID>0</InstanceID></u:` + action + `></s:Body></s:Envelope>`
	req, e := http.NewRequestWithContext(ctx, "POST", control, strings.NewReader(body))
	if e != nil {
		return nil, e
	}
	req.Header.Set("SOAPACTION", `"urn:schemas-upnp-org:service:AVTransport:1#`+action+`"`)
	req.Header.Set("Content-Type", "text/xml")
	resp, e := (&http.Client{Timeout: time.Second}).Do(req)
	if e != nil {
		return nil, e
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		return nil, errors.New("播放器状态暂不可读")
	}
	d := xml.NewDecoder(io.LimitReader(resp.Body, 32768))
	fields := map[string]string{}
	key := ""
	for {
		t, e := d.Token()
		if e == io.EOF {
			break
		}
		if e != nil {
			return nil, e
		}
		switch v := t.(type) {
		case xml.StartElement:
			key = v.Name.Local
		case xml.CharData:
			fields[key] += string(v)
		}
	}
	for k, value := range fields {
		fields[k] = strings.TrimSpace(value)
	}
	return fields, nil
}
func seconds(v string) int {
	p := strings.Split(v, ":")
	if len(p) != 3 {
		return 0
	}
	h, _ := strconv.Atoi(p[0])
	m, _ := strconv.Atoi(p[1])
	f, _ := strconv.ParseFloat(p[2], 64)
	return h*3600 + m*60 + int(f)
}

// SetURI/Play acknowledgments can precede the renderer's observable URI.
// Do not relinquish a new selection on one stale observation.
func (s *server) observeRendererURI(revision uint64, uri string, now time.Time) bool {
	s.mu.Lock()
	defer s.mu.Unlock()
	p := &s.player
	if p.Revision != revision {
		return false
	}
	if uri == "" || p.URI == "" || uri == p.URI {
		p.ForeignURI, p.ForeignCount = "", 0
		return true
	}
	if now.Sub(p.StartedAt) < 5*time.Second {
		return false
	}
	if p.ForeignURI != uri {
		p.ForeignURI, p.ForeignCount = uri, 1
	} else {
		p.ForeignCount++
	}
	if p.ForeignCount >= 3 {
		os.Remove(filepath.Join(s.dir, "active"))
	}
	return false
}
func (s *server) pollPlayback() {
	if time.Since(s.lastTransportPoll) < time.Second {
		return
	}
	s.lastTransportPoll = time.Now()
	s.mu.Lock()
	p := s.player
	s.mu.Unlock()
	if len(p.Queue) == 0 {
		return
	}
	if _, e := os.Stat(filepath.Join(s.dir, "active")); e != nil {
		return
	}
	ctx, cancel := context.WithTimeout(context.Background(), 1500*time.Millisecond)
	defer cancel()
	f, e := soapFields(ctx, p.Control, "GetTransportInfo")
	if e != nil {
		return
	}
	position, positionErr := soapFields(ctx, p.Control, "GetPositionInfo")
	if positionErr == nil && !s.observeRendererURI(p.Revision, position["TrackURI"], time.Now()) {
		return
	}
	state := f["CurrentTransportState"]
	pcm := s.observePCM()
	ended := false
	s.mu.Lock()
	if s.player.Revision != p.Revision {
		s.mu.Unlock()
		return
	}
	s.player.State = state
	if pcm.Closed && s.player.SeenPCM && state != "PAUSED_PLAYBACK" {
		s.player.ClosedPCMCount++
	} else {
		s.player.ClosedPCMCount = 0
	}
	if state == "PLAYING" {
		s.player.SeenPlaying = true
		s.player.StoppedCount = 0
	} else if state == "STOPPED" && s.player.SeenPlaying {
		s.player.StoppedCount++
		ended = s.player.StoppedCount >= 2
		if ended {
			s.player.SeenPlaying = false
			s.player.StoppedCount = 0
		}
	} else if state == "PAUSED_PLAYBACK" {
		s.player.StoppedCount = 0
	}
	if s.player.ClosedPCMCount >= 2 {
		ended = true
		s.player.SeenPlaying = false
		s.player.SeenPCM = false
		s.player.ClosedPCMCount = 0
	}
	s.mu.Unlock()
	if ended {
		if e := s.advance(1, true); e != nil {
			s.mu.Lock()
			if strings.Contains(e.Error(), "正在处理") {
				s.player.SeenPlaying = true
			} else if e.Error() != "队列已播放完" && s.player.AutoRetries < 2 {
				s.player.AutoRetries++
				s.player.SeenPlaying = true
			}
			s.player.Message = e.Error()
			s.mu.Unlock()
		}
	}
	if f, e := position, positionErr; e == nil {
		s.mu.Lock()
		if s.player.Revision != p.Revision {
			s.mu.Unlock()
			s.publishPlayback()
			return
		}
		pos := seconds(f["RelTime"])
		if pcm.Valid && s.player.Rate == 0 {
			s.player.Rate = pcm.Rate
		}
		updatePCMClock(&s.player, pcm, state)
		if pos == 0 {
			pos = int(s.player.PCMSeconds)
		}
		s.player.Position = pos
		if duration := seconds(f["TrackDuration"]); duration > 0 {
			s.player.Duration = duration
		}
		s.mu.Unlock()
	}
	s.publishPlayback()
}

// Network searches/URL negotiation may block the request worker. Keep the
// playback clock and natural-end observation independent of those requests.
func (s *server) playbackLoop(ctx context.Context) {
	tick := time.NewTicker(500 * time.Millisecond)
	defer tick.Stop()
	for {
		select {
		case <-ctx.Done():
			return
		case <-tick.C:
			s.pollPlayback()
		}
	}
}
func (s *server) publishPlayback() {
	s.mu.Lock()
	defer s.mu.Unlock()
	p, q := s.player, s.quality
	s.downloadMu.Lock()
	download := s.downloadStatus
	s.downloadMu.Unlock()
	// Append milliseconds without changing the existing nine-field IPC prefix.
	fmtstr := "%d\t%s\t%s\t%d\t%d\t%s\t%s\t%d\t%d\t%d\n%s\n%s\n%s\n%s\n%s\n"
	b := fmt.Sprintf(fmtstr, p.Revision, p.State, p.Mode, p.Position, p.Duration, q, p.Format, p.Rate, p.Bits, playbackMillis(p), clean(p.Song.Name), clean(p.Song.Artist), clean(p.Song.Album), clean(p.Message), clean(download))
	// Serialize the snapshot and IPC rename with controls and exit. Otherwise
	// two writers share nowplaying.next, or a stale snapshot reappears after exit.
	if p.URI != "" {
		atomicWrite(filepath.Join(s.dir, "nowplaying"), []byte(b))
	}
}

func playbackMillis(p playbackState) int {
	// Native non-zero SOAP positions remain authoritative. PCM supplies the
	// fractional part only when it agrees with the published integer position.
	if p.PCMSeconds >= 0 && int(p.PCMSeconds) == p.Position {
		return int(p.PCMSeconds * 1000)
	}
	return p.Position * 1000
}

func checkQuality(q, ext string, rate, bits int) error {
	if (q == "lossless" || q == "hires") && ext != "flac" && ext != "wav" && ext != "ape" {
		return errors.New("该歌曲未返回所选无损音源，请选择自动或其他音质")
	}
	if q == "hires" && ext == "flac" && rate <= 48000 && bits <= 16 {
		return errors.New("该歌曲未返回 Hi-Res 音源，请选择无损或自动")
	}
	return nil
}
