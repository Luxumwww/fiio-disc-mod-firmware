package main

import (
	"context"
	"github.com/guohuiyuan/music-lib/provider"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"time"
)

func (s *server) cancelPrefetch() {
	s.mu.Lock()
	cancel := s.prefetchCancel
	s.prefetchCancel = nil
	s.mu.Unlock()
	if cancel != nil {
		cancel()
	}
}

// A single predictable next song, only after current complete-cache validation.
// Do not guess shuffle choices or download the repeated current track again.
func (s *server) maybePrefetch() {
	s.mu.Lock()
	p := s.player
	q := s.quality
	s.mu.Unlock()
	if p.Revision == 0 || p.Mode != "order" || len(p.Queue) == 0 {
		return
	}
	if _, _, ok := s.cachedAudio(downloadTask{p.Song, p.Source, q}); !ok {
		return
	}
	s.mu.Lock()
	if s.player.Revision != p.Revision || s.prefetchRevision == p.Revision || s.prefetchWorker {
		s.mu.Unlock()
		return
	}
	ctx, cancel := context.WithTimeout(context.Background(), 4*time.Minute)
	s.prefetchRevision = p.Revision
	s.prefetchCancel = cancel
	s.prefetchWorker = true
	s.mu.Unlock()
	defer func() { cancel(); s.mu.Lock(); s.prefetchWorker = false; s.mu.Unlock(); go s.maybePrefetch() }()
	// Providers have mutable VIP/cookie caches, so use an independent credential
	// snapshot, never the interactive provider gate or its instances.
	freshProvider := s.metadataProvider(p.Source)
	if s.prefetchFactory != nil {
		freshProvider = s.prefetchFactory(p.Source)
	}
	if freshProvider == nil {
		return
	}
	dir := filepath.Join(s.dir, "prefetch")
	os.MkdirAll(dir, 0700)
	isolated := &server{client: s.client, providers: map[string]provider.MusicProvider{p.Source: freshProvider}, dir: dir, credentials: s.credentials, testCardRoot: s.testCardRoot}
	account := s.accountCacheKey(p.Source, "prefetch")
	nextIndex := p.Index + 1
	queue := p.Queue
	if nextIndex >= len(queue) {
		if p.List.ID == "" || !p.List.More {
			return
		}
		var e error
		queue, _, e = isolated.playlistChunk(p.Source, p.List.ID, p.List.Page+1)
		if e != nil || ctx.Err() != nil || len(queue) == 0 {
			return
		}
		nextIndex = 0
	}
	next := cloneSong(queue[nextIndex])
	task := downloadTask{next, p.Source, q}
	if _, _, ok := s.cachedAudio(task); ok {
		return
	}
	u, _, _, _, e := isolated.resolveAudio(ctx, p.Source, &next, q)
	if e != nil || ctx.Err() != nil {
		return
	}
	req, e := http.NewRequestWithContext(ctx, "GET", u, nil)
	if e != nil {
		return
	}
	req.Header.Set("Accept-Encoding", "identity")
	response, e := (&http.Client{Transport: s.client.Transport}).Do(req)
	if e != nil {
		return
	}
	defer response.Body.Close()
	if ctx.Err() != nil || s.accountCacheKey(p.Source, "prefetch") != account {
		return
	}
	response.Body = &prefetchReader{ctx: ctx, body: response.Body, begin: time.Now()}
	s.streamAndCache(discardAudioResponse{}, response, task)
}

type discardAudioResponse struct{}

func (discardAudioResponse) Header() http.Header         { return http.Header{} }
func (discardAudioResponse) WriteHeader(int)             {}
func (discardAudioResponse) Write(b []byte) (int, error) { return len(b), nil }

type prefetchReader struct {
	ctx   context.Context
	body  io.ReadCloser
	begin time.Time
	bytes int64
}

func (r *prefetchReader) Read(b []byte) (int, error) {
	if e := r.ctx.Err(); e != nil {
		return 0, e
	}
	if len(b) > 32768 {
		b = b[:32768]
	}
	n, e := r.body.Read(b)
	r.bytes += int64(n)
	delay := time.Duration(r.bytes)*time.Second/(512<<10) - time.Since(r.begin)
	if delay > 0 {
		timer := time.NewTimer(delay)
		select {
		case <-r.ctx.Done():
			timer.Stop()
			return 0, r.ctx.Err()
		case <-timer.C:
		}
	}
	return n, e
}
func (r *prefetchReader) Close() error { return r.body.Close() }
