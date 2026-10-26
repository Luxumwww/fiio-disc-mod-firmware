package main

import (
	"context"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"
)

type rendererProvider struct {
	mockLibrary
	url string
}

func (p *rendererProvider) GetDownloadURL(song *model.Song) (string, error) { return p.url, nil }
func TestPlaybackQueueSurvivesBrowsingAndNaturalEnd(t *testing.T) {
	audio := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b := make([]byte, 64)
		copy(b, "fLaC")
		b[7] = 34
		b[18] = 10
		b[19] = 196
		b[20] = 66
		b[21] = 240
		w.Write(b)
	}))
	defer audio.Close()
	var mu sync.Mutex
	state := "PLAYING"
	var actions []string
	renderer := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		mu.Lock()
		defer mu.Unlock()
		action := r.Header.Get("SOAPACTION")
		actions = append(actions, action)
		if strings.Contains(action, "#Pause") {
			state = "PAUSED_PLAYBACK"
		}
		if strings.Contains(action, "#Play\"") {
			state = "PLAYING"
		}
		fmt.Fprintf(w, "<Envelope><Body><CurrentTransportState>%s</CurrentTransportState><RelTime>00:00:02</RelTime><TrackDuration>00:00:30</TrackDuration></Body></Envelope>", state)
	}))
	defer renderer.Close()
	s := newServer(t.TempDir(), 49381)
	s.providers["mock"] = &rendererProvider{url: audio.URL}
	q := []model.Song{{ID: "first", Name: "A"}, {ID: "second", Name: "B"}, {ID: "third", Name: "C"}}
	if e := s.startTrack(q, "mock", playlistContext{}, 0, renderer.URL, 0); e != nil {
		t.Fatal(e)
	}
	// Browsing replaces the on-screen selection generation, never the playing queue.
	if _, e := s.search("mock", "other"); e != nil {
		t.Fatal(e)
	}
	if s.player.Queue[0].ID != "first" {
		t.Fatal("browsing destroyed queue")
	}
	if e := s.control("toggle", ""); e != nil || s.player.State != "PAUSED_PLAYBACK" {
		t.Fatal(e, s.player.State)
	}
	s.lastTransportPoll = time.Time{}
	s.pollPlayback()
	if s.player.Index != 0 {
		t.Fatal("pause advanced queue")
	}
	if e := s.control("toggle", ""); e != nil {
		t.Fatal(e)
	}
	s.lastTransportPoll = time.Time{}
	s.pollPlayback()
	mu.Lock()
	state = "STOPPED"
	mu.Unlock()
	s.lastTransportPoll = time.Time{}
	s.pollPlayback()
	if s.player.Index != 0 {
		t.Fatal("single stopped observation advanced")
	}
	s.lastTransportPoll = time.Time{}
	s.pollPlayback()
	if s.player.Index != 1 || s.player.Song.ID != "second" {
		t.Fatal("natural end did not advance", s.player.Index)
	}
	if s.player.Position != 0 {
		t.Fatal("old track position overwrote newly advanced track", s.player.Position)
	}
	if e := s.control("mode", "repeat"); e != nil {
		t.Fatal(e)
	}
	if e := s.advance(1, true); e != nil || s.player.Index != 1 {
		t.Fatal("repeat failed", e)
	}
	// Closing a screen only cancels a pending selection, never sends Stop.
	os.WriteFile(filepath.Join(s.dir, "request"), []byte("123\tcancel\n"), 0600)
	if e := s.advance(-1, false); e != nil || s.player.Index != 0 {
		t.Fatal(e)
	}
	for _, a := range actions {
		if strings.Contains(a, "#Stop") {
			t.Fatal("unexpected Stop", a)
		}
	}
}
func TestDownloadSelectionIsBoundedAndSnapshot(t *testing.T) {
	s := newServer(t.TempDir(), 1)
	s.generation = 5
	s.songs = []model.Song{{ID: "a"}, {ID: "b"}}
	s.source = "mock"
	if e := s.enqueueDownloads(4, "0"); e == nil {
		t.Fatal("stale generation")
	}
	if e := s.enqueueDownloads(5, "0,9"); e == nil || len(s.downloads) != 0 {
		t.Fatal("partial invalid selection accepted")
	}
	if e := s.enqueueDownloads(5, "0,0,1"); e != nil || len(s.downloads) != 2 {
		t.Fatal(e)
	}
	s.songs[0].ID = "replaced"
	if (<-s.downloads).Song.ID != "a" {
		t.Fatal("download selection not snapshotted")
	}
	for len(s.downloads) < cap(s.downloads) {
		s.downloads <- downloadTask{}
	}
	if e := s.enqueueDownloads(5, "0"); e == nil {
		t.Fatal("overflow accepted")
	}
}
func TestActualAudioHeaderAndFailedSelectionPreservesTrack(t *testing.T) {
	b := make([]byte, 64)
	copy(b, "fLaC")
	b[7] = 34
	b[18] = 46
	b[19] = 224
	b[20] = 1
	b[21] = 112
	ext, rate, bits := audioFormat(b)
	if ext != "flac" || rate != 192000 || bits != 24 {
		t.Fatal(ext, rate, bits)
	}
	if ext, _, _ := audioFormat([]byte("<html>login required</html>")); ext != "" {
		t.Fatal("login response called audio")
	}
	bad := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { http.Error(w, "no", 403) }))
	defer bad.Close()
	s := newServer(t.TempDir(), 1)
	s.providers["mock"] = &rendererProvider{url: bad.URL}
	s.player.Song = model.Song{ID: "keep"}
	if e := s.startTrack([]model.Song{{ID: "bad"}}, "mock", playlistContext{}, 0, bad.URL, 0); e == nil || s.player.Song.ID != "keep" {
		t.Fatal("failed song replaced playback")
	}
	if _, _, _, e := s.probeAudio(context.Background(), bad.URL); e == nil {
		t.Fatal("forbidden audio accepted")
	}
}

func TestDownloadWritesActualFormatAndRejectsIncomplete(t *testing.T) {
	body := make([]byte, 100)
	copy(body, "fLaC")
	body[7] = 34
	body[18] = 46
	body[19] = 224
	body[20] = 1
	body[21] = 112
	bad := false
	audio := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if bad {
			w.Header().Set("Content-Length", "200")
		}
		w.Write(body)
	}))
	defer audio.Close()
	s := newServer(t.TempDir(), 1)
	s.providers["mock"] = &rendererProvider{url: audio.URL}
	dir := t.TempDir()
	task := downloadTask{Song: model.Song{ID: "test", Name: "test", Artist: "artist", Ext: "mp3"}, Source: "mock", Quality: "lossless"}
	if e := s.downloadToDirectory(context.Background(), task, dir); e != nil {
		t.Fatal(e)
	}
	files, _ := os.ReadDir(dir)
	if len(files) != 1 || !strings.HasSuffix(files[0].Name(), ".flac") {
		t.Fatal("saved using advertised rather than actual format", files)
	}
	b, _ := os.ReadFile(filepath.Join(dir, files[0].Name()))
	if string(b) != string(body) {
		t.Fatal("download altered audio data")
	}
	bad = true
	task.Song.ID = "incomplete"
	if e := s.downloadToDirectory(context.Background(), task, dir); e == nil {
		t.Fatal("incomplete download accepted")
	}
	files, _ = os.ReadDir(dir)
	if len(files) != 1 {
		t.Fatal("incomplete file published or temporary file leaked", files)
	}
}

func TestRendererURILagAndExternalTakeover(t *testing.T) {
	s := newServer(t.TempDir(), 1)
	now := time.Now()
	s.player = playbackState{Revision: 2, URI: "http://127.0.0.1/audio/2.flac", StartedAt: now}
	active := filepath.Join(s.dir, "active")
	os.WriteFile(active, []byte("1"), 0600)
	old := "http://127.0.0.1/audio/1.flac"
	for i := 0; i < 4; i++ {
		if s.observeRendererURI(2, old, now.Add(time.Duration(i)*time.Second)) {
			t.Fatal("stale URI treated as current")
		}
	}
	if _, e := os.Stat(active); e != nil {
		t.Fatal("initial URI lag lost playback ownership")
	}
	if !s.observeRendererURI(2, s.player.URI, now.Add(6*time.Second)) {
		t.Fatal("current URI rejected")
	}
	external := "http://phone/music.flac"
	s.observeRendererURI(2, external, now.Add(7*time.Second))
	s.observeRendererURI(2, s.player.URI, now.Add(8*time.Second))
	s.observeRendererURI(2, external, now.Add(9*time.Second))
	s.observeRendererURI(2, external, now.Add(10*time.Second))
	if _, e := os.Stat(active); e != nil {
		t.Fatal("transient external URI lost ownership")
	}
	s.observeRendererURI(2, external, now.Add(11*time.Second))
	if _, e := os.Stat(active); !os.IsNotExist(e) {
		t.Fatal("persistent external takeover not released")
	}
	os.WriteFile(active, []byte("1"), 0600)
	if s.observeRendererURI(1, external, now.Add(12*time.Second)) {
		t.Fatal("obsolete poll accepted")
	}
	if _, e := os.Stat(active); e != nil {
		t.Fatal("obsolete poll affected new selection")
	}
}
