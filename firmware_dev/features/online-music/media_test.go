package main

import (
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestLyricTimestampsOffsetsAndBounds(t *testing.T) {
	word := parseLyrics("[00:02.00]温盏[00:02.50]三月杏雨[00:03.00]\n")
	if len(word) != 1 || word[0].Text != "温盏三月杏雨" || word[0].Millis != 2000 {
		t.Fatal(word)
	}
	lines := parseLyrics("[offset:-500]\n[01:02.50][01:03.00]鼓点\n[00:01.00]开始\n[ti:title]\n[9999:90]bad\n")
	if len(lines) != 3 || lines[0].Millis != 500 || lines[1].Millis != 62000 || lines[2].Millis != 62500 || lines[2].Text != "鼓点" {
		t.Fatal(lines)
	}
	if len(parseLyrics(strings.Repeat("[00:01]x\n", 1000))) != 600 {
		t.Fatal("unbounded lyric list")
	}
}
func TestPlatformExitDoesNotStopOtherRenderer(t *testing.T) {
	stopped := 0
	uri := "http://phone/foreign.mp3"
	r := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if strings.Contains(r.Header.Get("SOAPACTION"), "#Stop") {
			stopped++
		}
		fmt.Fprintf(w, "<Envelope><TrackURI>%s</TrackURI></Envelope>", uri)
	}))
	defer r.Close()
	s := newServer(t.TempDir(), 1)
	s.player = playbackState{Revision: 2, URI: "http://127.0.0.1/audio/1.flac", Control: r.URL, Queue: []model.Song{{ID: "one"}}}
	active := filepath.Join(s.dir, "active")
	os.WriteFile(active, []byte("1"), 0600)
	if e := s.exitPlayback(); e != nil || stopped != 0 {
		t.Fatal(e, stopped)
	}
	if _, e := os.Stat(active); !os.IsNotExist(e) || len(s.player.Queue) != 0 {
		t.Fatal("ownership retained")
	}
	s.player = playbackState{Revision: 3, URI: "http://127.0.0.1/audio/2.flac", Control: r.URL, Queue: []model.Song{{ID: "two"}}}
	uri = s.player.URI
	if e := s.exitPlayback(); e != nil || stopped != 1 {
		t.Fatal(e, stopped)
	}
}
func TestQueueViewPreservesBrowserSelection(t *testing.T) {
	s := newServer(t.TempDir(), 1)
	s.generation = 8
	s.songs = []model.Song{{ID: "browse", Name: "浏览歌曲"}}
	s.player = playbackState{Revision: 3, Queue: []model.Song{{ID: "play", Name: "播放歌曲"}}}
	s.publishQueue(99)
	b, _ := os.ReadFile(filepath.Join(s.dir, "response"))
	if !strings.Contains(string(b), "播放歌曲") || s.songs[0].ID != "browse" || s.generation != 8 {
		t.Fatal(string(b))
	}
	if e := s.queuePlay(2, 0); e == nil {
		t.Fatal("stale queue revision accepted")
	}
}
