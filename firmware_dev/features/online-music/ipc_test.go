package main

import (
	"context"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"
)

func TestRealFileCommandsPrettySOAPAndAutomaticNext(t *testing.T) {
	audio := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { io.WriteString(w, "ID3"+strings.Repeat("a", 61)) }))
	defer audio.Close()
	var mu sync.Mutex
	state, uri := "PLAYING", ""
	renderer := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		mu.Lock()
		defer mu.Unlock()
		a := r.Header.Get("SOAPACTION")
		if strings.Contains(a, "#SetAVTransportURI") {
			b, _ := io.ReadAll(r.Body)
			v := string(b)
			start := strings.Index(v, "<CurrentURI>") + 12
			end := strings.Index(v, "</CurrentURI>")
			if start >= 12 && end > start {
				uri = v[start:end]
			}
		}
		if strings.Contains(a, "#Pause") {
			state = "PAUSED_PLAYBACK"
		}
		if strings.Contains(a, "#Play\"") {
			state = "PLAYING"
		}
		fmt.Fprintf(w, "<Envelope>\n<Body>\n<CurrentTransportState>%s</CurrentTransportState>\n<TrackURI>%s</TrackURI>\n<RelTime>00:00:07</RelTime>\n<TrackDuration>00:00:30</TrackDuration>\n</Body>\n</Envelope>\n", state, uri)
	}))
	defer renderer.Close()
	s := newServer(t.TempDir(), 1)
	s.quality = "standard"
	s.providers["mock"] = &rendererProvider{url: audio.URL}
	if e := s.startTrack([]model.Song{{ID: "one"}, {ID: "two"}, {ID: "three"}}, "mock", playlistContext{}, 0, renderer.URL, 0); e != nil {
		t.Fatal(e)
	}
	s.mu.Lock()
	s.player.StartedAt = time.Now().Add(-time.Minute)
	s.mu.Unlock()
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	done := make(chan struct{})
	go func() { s.fileLoop(ctx); close(done) }()
	playDone := make(chan struct{})
	go func() { s.playbackLoop(ctx); close(playDone) }()
	defer func() { cancel(); <-done }()
	defer func() { cancel(); <-playDone }()
	wait := func(condition func() bool) {
		t.Helper()
		until := time.Now().Add(4 * time.Second)
		for time.Now().Before(until) {
			if condition() {
				return
			}
			time.Sleep(20 * time.Millisecond)
		}
		t.Fatal("file command/state did not update")
	}
	command := func(seq int, a string) {
		t.Helper()
		if e := atomicWrite(filepath.Join(s.dir, "request"), []byte(fmt.Sprintf("%d\tcontrol\t%s\t\n", seq, a))); e != nil {
			t.Fatal(e)
		}
	}
	command(1, "next")
	wait(func() bool { s.mu.Lock(); defer s.mu.Unlock(); return s.player.Index == 1 })
	command(2, "previous")
	wait(func() bool { s.mu.Lock(); defer s.mu.Unlock(); return s.player.Index == 0 })
	command(3, "toggle")
	wait(func() bool { s.mu.Lock(); defer s.mu.Unlock(); return s.player.State == "PAUSED_PLAYBACK" })
	command(4, "toggle")
	wait(func() bool { s.mu.Lock(); defer s.mu.Unlock(); return s.player.State == "PLAYING" })
	wait(func() bool { s.mu.Lock(); defer s.mu.Unlock(); return s.player.SeenPlaying && s.player.Position == 7 })
	if _, e := os.Stat(filepath.Join(s.dir, "active")); e != nil {
		t.Fatal("pretty SOAP lost playback ownership", e)
	}
	b, _ := os.ReadFile(filepath.Join(s.dir, "nowplaying"))
	if len(strings.Split(strings.SplitN(string(b), "\n", 2)[0], "\t")) != 10 {
		t.Fatal("multiline IPC header", string(b))
	}
	mu.Lock()
	state = "STOPPED"
	mu.Unlock()
	wait(func() bool { s.mu.Lock(); defer s.mu.Unlock(); return s.player.Index == 1 })
}
