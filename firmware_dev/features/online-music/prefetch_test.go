package main

import (
	"bytes"
	"context"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/provider"
	"io"
	"net/http"
	"net/http/httptest"
	"testing"
	"time"
)

func TestPrefetchRateLimitAndCancellation(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	r := &prefetchReader{ctx: ctx, body: io.NopCloser(bytes.NewReader(make([]byte, 128<<10))), begin: time.Now()}
	n, e := io.Copy(io.Discard, r)
	if e != nil || n != 128<<10 || time.Since(r.begin) < 230*time.Millisecond {
		t.Fatal(n, e, time.Since(r.begin))
	}
	r = &prefetchReader{ctx: ctx, body: io.NopCloser(bytes.NewReader(make([]byte, 1<<20))), begin: time.Now()}
	done := make(chan error, 1)
	go func() { _, e := io.Copy(io.Discard, r); done <- e }()
	time.Sleep(10 * time.Millisecond)
	cancel()
	select {
	case e := <-done:
		if e != context.Canceled {
			t.Fatal(e)
		}
	case <-time.After(time.Second):
		t.Fatal("cancel did not interrupt limiter")
	}
}
func TestPrefetchCompleteCacheUsesIndependentProviderAndNextCache(t *testing.T) {
	s := cacheServer(t)
	audio := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b := cacheFLAC()
		w.Header().Set("Content-Length", fmt.Sprint(len(b)))
		w.Write(b)
	}))
	defer audio.Close()
	fresh := &mockProvider{url: audio.URL}
	interactive := &mockProvider{url: "invalid-interactive-provider-must-not-be-used"}
	s.providers["netease"] = interactive
	s.prefetchFactory = func(string) provider.MusicProvider { return fresh }
	current := model.Song{ID: "current", Name: "Current"}
	next := model.Song{ID: "next", Name: "Next"}
	s.player = playbackState{Revision: 1, Source: "netease", Song: current, Queue: []model.Song{current, next}, Mode: "shuffle"}
	task := downloadTask{current, "netease", "auto"}
	b := cacheFLAC()
	s.streamAndCache(discardAudioResponse{}, &http.Response{StatusCode: 200, Header: make(http.Header), ContentLength: int64(len(b)), Body: io.NopCloser(bytes.NewReader(b))}, task)
	s.mu.Lock()
	s.player.Mode = "order"
	s.mu.Unlock()
	s.maybePrefetch()
	if _, _, ok := s.cachedAudio(downloadTask{next, "netease", "auto"}); !ok {
		t.Fatal("next missing from complete private cache")
	}
	if interactive.called != 0 {
		t.Fatal("interactive provider touched")
	}
	u, _, _, _, e := s.resolvePlaybackAudio(context.Background(), "netease", &next, "auto")
	if e != nil || len(u) < 7 || u[:7] != "file://" {
		t.Fatal("prefetched track not used", u, e)
	}
	s.mu.Lock()
	rev := s.prefetchRevision
	s.mu.Unlock()
	s.maybePrefetch()
	s.mu.Lock()
	same := rev == s.prefetchRevision
	s.mu.Unlock()
	if !same {
		t.Fatal("duplicate revision")
	}
}
func TestPrefetchSkipsIncompleteShuffleRepeatAndDuplicateWorker(t *testing.T) {
	s := cacheServer(t)
	for _, mode := range []string{"order", "shuffle", "repeat"} {
		s.player = playbackState{Revision: 3, Mode: mode}
		s.maybePrefetch()
		if s.prefetchRevision != 0 || s.prefetchWorker {
			t.Fatal("prefetched missing current/queue")
		}
	}
	s.prefetchWorker = true
	s.maybePrefetch()
	if !s.prefetchWorker {
		t.Fatal("changed unrelated worker state")
	}
}
