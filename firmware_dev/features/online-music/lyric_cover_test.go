package main

import (
	"bytes"
	"github.com/guohuiyuan/music-lib/model"
	"image"
	"image/png"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync/atomic"
	"testing"
	"time"
)

func TestMillisecondIPCUsesConsumedPCMAndPreservesPrefix(t *testing.T) {
	s := newServer(t.TempDir(), 1)
	s.player = playbackState{Revision: 3, URI: "audio", State: "PLAYING", Position: 85, PCMSeconds: 85.375, Song: model.Song{Name: "123 numeric title"}}
	s.publishPlayback()
	b, e := os.ReadFile(filepath.Join(s.dir, "nowplaying"))
	if e != nil {
		t.Fatal(e)
	}
	header := strings.SplitN(string(b), "\n", 2)[0]
	if !strings.HasSuffix(header, "\t85375") || len(strings.Split(header, "\t")) != 10 {
		t.Fatal(header)
	}
	s.player.Position = 100
	if playbackMillis(s.player) != 100000 {
		t.Fatal("stale PCM overrode authoritative position")
	}
}

func TestQueueArtworkSlowFirstDoesNotBlockLaterAndCacheRetained(t *testing.T) {
	var encoded bytes.Buffer
	png.Encode(&encoded, image.NewRGBA(image.Rect(0, 0, 2, 2)))
	slow := make(chan struct{})
	started := make(chan struct{})
	var fast atomic.Int32
	r := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.URL.Path == "/slow" {
			close(started)
			select {
			case <-slow:
			case <-r.Context().Done():
				return
			}
		} else {
			fast.Add(1)
		}
		w.Write(encoded.Bytes())
	}))
	defer r.Close()
	s := newServer(t.TempDir(), 1)
	s.player = playbackState{Revision: 8, Queue: []model.Song{{Name: "slow", Cover: r.URL + "/slow"}, {Name: "fast", Cover: r.URL + "/fast"}}}
	s.publishQueue(22)
	select {
	case <-started:
	case <-time.After(time.Second):
		t.Fatal("first cover not started")
	}
	wait := func(name string) {
		t.Helper()
		deadline := time.Now().Add(2 * time.Second)
		for time.Now().Before(deadline) {
			if b, e := os.ReadFile(filepath.Join(s.dir, name)); e == nil && len(b) == 48*48*4 {
				return
			}
			time.Sleep(10 * time.Millisecond)
		}
		t.Fatal("missing", name)
	}
	wait("thumb-queue-8-1.bgra") // succeeds while the first request is held open
	close(slow)
	wait("thumb-queue-8-0.bgra")
	s.startBrowseArt("lists", 9, []string{r.URL + "/fast"})
	wait("thumb-lists-9-0.bgra")
	if fast.Load() != 1 {
		t.Fatal("decoded cache did not prevent repeat network fetch", fast.Load())
	}
	if _, e := os.Stat(filepath.Join(s.dir, "thumb-queue-8-1.bgra")); e != nil {
		t.Fatal("new browser destroyed queue cache", e)
	}
	s.mu.Lock()
	s.artCancel()
	s.mu.Unlock()
}

func TestCoverPageChangeCancelsOldNetworkRequest(t *testing.T) {
	started := make(chan struct{})
	canceled := make(chan struct{})
	r := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { close(started); <-r.Context().Done(); close(canceled) }))
	defer r.Close()
	s := newServer(t.TempDir(), 1)
	s.startBrowseArt("lists", 1, []string{r.URL})
	select {
	case <-started:
	case <-time.After(time.Second):
		t.Fatal("not started")
	}
	s.startBrowseArt("queue", 2, nil)
	select {
	case <-canceled:
	case <-time.After(time.Second):
		t.Fatal("stale network fetch not canceled")
	}
	if _, e := os.Stat(filepath.Join(s.dir, "thumb-lists-1-0.bgra")); !os.IsNotExist(e) {
		t.Fatal("stale artwork published")
	}
}

func TestDecodedCoverCacheStaysWithinOneMiB(t *testing.T) {
	var encoded bytes.Buffer
	png.Encode(&encoded, image.NewRGBA(image.Rect(0, 0, 2, 2)))
	r := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { w.Write(encoded.Bytes()) }))
	defer r.Close()
	s := newServer(t.TempDir(), 1)
	for i := 0; i < 18; i++ {
		if len(s.coverPixels(r.URL+"/"+strconv.Itoa(i), 144)) != 144*144*4 {
			t.Fatal("missing decoded cover")
		}
	}
	s.coverMu.Lock()
	defer s.coverMu.Unlock()
	total := 0
	for _, c := range s.coverCache {
		total += len(c.data)
	}
	if total > 1<<20 || len(s.coverCache) != 12 {
		t.Fatal("unbounded decoded cache", total, len(s.coverCache))
	}
}
