package main

import (
	"bytes"
	"context"
	"fmt"
	_ "golang.org/x/image/webp"
	"image"
	_ "image/gif"
	_ "image/jpeg"
	_ "image/png"
	"io"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"
)

type coverCacheEntry struct {
	key  string
	data []byte
}

type browseArtPage struct {
	Kind       string
	Generation uint64
	URLs       []string
	Source     string
}

func (s *server) refreshPageArt(kind string, gen uint64) {
	s.refreshPagesArt(kind, strconv.FormatUint(gen, 10))
}
func (s *server) refreshPagesArt(kind, generations string) {
	if kind != "lists" && kind != "tracks" {
		return
	}
	var pages []browseArtPage
	seen := map[uint64]bool{}
	s.mu.Lock()
	for _, token := range strings.Split(generations, ",") {
		if len(pages) >= 2 {
			break
		}
		gen, e := strconv.ParseUint(token, 10, 64)
		if e != nil || seen[gen] {
			continue
		}
		seen[gen] = true
		page := browseArtPage{Kind: kind, Generation: gen}
		if kind == "lists" {
			if v, ok := s.listPages[gen]; ok {
				page.Source = v.Source
				for _, item := range v.Lists {
					page.URLs = append(page.URLs, item.Cover)
				}
			}
		} else {
			if v, ok := s.selectedPageLocked(gen); ok {
				page.Source = v.Source
				for _, item := range v.Songs {
					page.URLs = append(page.URLs, item.Cover)
				}
			}
		}
		if page.Source != "" {
			pages = append(pages, page)
		}
	}
	s.mu.Unlock()
	if len(pages) > 0 {
		s.startBrowseArtPages(pages)
	}
}

func (s *server) fetchBrowseArt(ctx context.Context, kind string, generation uint64, urls []string, source string) {
	// Two network requests overlap; image decoding remains serialized. A slow
	// first cover cannot hold up all following visible rows or the player cover.
	jobs := make(chan int)
	var workers sync.WaitGroup
	for n := 0; n < 2; n++ {
		workers.Add(1)
		go func() {
			defer workers.Done()
			for i := range jobs {
				if ctx.Err() != nil {
					return
				}
				if urls[i] == "" {
					continue
				}
				data := s.coverPixelsFor(ctx, source, urls[i], 48)
				if data == nil {
					continue
				}
				s.mu.Lock()
				if ctx.Err() == nil && (s.artActiveKeys[fmt.Sprintf("%s-%d", kind, generation)] || s.artKey == fmt.Sprintf("%s-%d", kind, generation)) {
					atomicWrite(filepath.Join(s.dir, fmt.Sprintf("thumb-%s-%d-%d.bgra", kind, generation, i)), data)
				}
				s.mu.Unlock()
			}
		}()
	}
	defer func() { close(jobs); workers.Wait() }()
	for i := range urls {
		select {
		case jobs <- i:
		case <-ctx.Done():
			return
		}
	}
}
func (s *server) startBrowseArt(kind string, generation uint64, urls []string) {
	s.mu.Lock()
	source := s.source
	if kind == "lists" {
		source = s.listSource
	}
	if kind == "queue" {
		source = s.player.Source
	}
	s.mu.Unlock()
	s.startBrowseArtSource(kind, generation, urls, source)
}
func (s *server) startBrowseArtSource(kind string, generation uint64, urls []string, source string) {
	s.startBrowseArtPages([]browseArtPage{{kind, generation, append([]string(nil), urls...), source}})
}
func (s *server) startBrowseArtPages(pages []browseArtPage) {
	ctx, cancel := context.WithCancel(context.Background())
	s.mu.Lock()
	if s.artCancel != nil {
		s.artCancel()
	}
	s.artCancel = cancel
	s.artActiveKeys = map[string]bool{}
	s.artKey = ""
	for _, p := range pages {
		key := fmt.Sprintf("%s-%d", p.Kind, p.Generation)
		s.artActiveKeys[key] = true
		s.artKey = key
		for i, k := range s.artPages {
			if k == key {
				s.artPages = append(s.artPages[:i], s.artPages[i+1:]...)
				break
			}
		}
		s.artPages = append(s.artPages, key)
	}
	for len(s.artPages) > 4 {
		old := s.artPages[0]
		s.artPages = s.artPages[1:]
		files, _ := filepath.Glob(filepath.Join(s.dir, "thumb-"+old+"-*.bgra"))
		for _, f := range files {
			os.Remove(f)
		}
	}
	s.mu.Unlock()
	go func() {
		for _, p := range pages {
			if ctx.Err() != nil {
				return
			}
			s.fetchBrowseArt(ctx, p.Kind, p.Generation, p.URLs, p.Source)
		}
	}()
}
func (s *server) fetchCover(u string, revision uint64) {
	if u == "" {
		return
	}
	s.mu.Lock()
	source := s.player.Source
	current := s.player.Revision == revision
	s.mu.Unlock()
	if !current {
		return
	}
	out := s.coverPixelsFor(context.Background(), source, u, 144)
	if out == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.player.Revision != revision {
		return
	}
	atomicWrite(filepath.Join(s.dir, "cover.bgra"), out)
	atomicWrite(filepath.Join(s.dir, "cover.revision"), []byte(strconv.FormatUint(revision, 10)))
}
func (s *server) coverPixels(u string, size int) []byte {
	return s.coverPixelsContext(context.Background(), u, size)
}
func (s *server) coverPixelsContext(ctx context.Context, u string, size int) []byte {
	return s.coverPixelsFor(ctx, "", u, size)
}
func (s *server) coverPixelsFor(ctx context.Context, source, u string, size int) []byte {
	u = sizedCoverURL(u, size)
	key := fmt.Sprintf("%d:%s", size, u)
	s.coverMu.Lock()
	for i, c := range s.coverCache {
		if c.key == key {
			copy(s.coverCache[i:], s.coverCache[i+1:])
			s.coverCache[len(s.coverCache)-1] = c
			s.coverMu.Unlock()
			return c.data
		}
	}
	s.coverMu.Unlock()
	diskDir, diskErr := s.cacheDirectory(source, "封面")
	diskPath := ""
	if diskErr == nil {
		diskPath = filepath.Join(diskDir, cacheDigest(key)+".bgra")
		if b, e := os.ReadFile(diskPath); e == nil && len(b) == size*size*4 {
			return b
		}
	}
	for i := 0; i < 2 && ctx.Err() == nil; i++ {
		if data := s.coverAttemptContext(ctx, u, size); data != nil {
			s.coverMu.Lock()
			for _, c := range s.coverCache {
				if c.key == key {
					s.coverMu.Unlock()
					return c.data
				}
			}
			// Immutable decoded pixels: <=1MiB RAM LRU plus platform-separated TF cache.
			total := len(data)
			for _, c := range s.coverCache {
				total += len(c.data)
			}
			for total > 1<<20 && len(s.coverCache) > 0 {
				total -= len(s.coverCache[0].data)
				copy(s.coverCache, s.coverCache[1:])
				s.coverCache[len(s.coverCache)-1] = coverCacheEntry{}
				s.coverCache = s.coverCache[:len(s.coverCache)-1]
			}
			s.coverCache = append(s.coverCache, coverCacheEntry{key, data})
			s.coverMu.Unlock()
			if diskPath != "" {
				s.cacheDiskMu.Lock()
				diskAtomicWrite(diskPath, data)
				trimManagedCache(diskDir, 64<<20)
				s.cacheDiskMu.Unlock()
			}
			return data
		}
	}
	return nil
}
func sizedCoverURL(raw string, size int) string {
	u, e := url.Parse(raw)
	if e != nil {
		return raw
	}
	h := strings.ToLower(u.Hostname())
	if strings.HasSuffix(h, ".music.126.net") || strings.HasSuffix(h, ".music.163.com") {
		q := u.Query()
		q.Set("param", fmt.Sprintf("%dy%d", size*2, size*2))
		u.RawQuery = q.Encode()
		u.Scheme = "https"
	}
	return u.String()
}
func (s *server) coverAttempt(u string, size int) []byte {
	return s.coverAttemptContext(context.Background(), u, size)
}
func (s *server) coverAttemptContext(parent context.Context, u string, size int) []byte {
	ctx, cancel := context.WithTimeout(parent, 8*time.Second)
	defer cancel()
	req, e := http.NewRequestWithContext(ctx, "GET", u, nil)
	if e != nil {
		return nil
	}
	req.Header.Set("Accept", "image/jpeg,image/png,image/webp,image/gif;q=0.8")
	resp, e := s.client.Do(req)
	if e != nil {
		return nil
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		return nil
	}
	b, e := io.ReadAll(io.LimitReader(resp.Body, (1<<20)+1))
	if e != nil || len(b) > 1<<20 {
		return nil
	}
	config, _, e := image.DecodeConfig(bytes.NewReader(b))
	if e != nil || config.Width <= 0 || config.Height <= 0 || config.Width > 4096 || config.Height > 4096 || config.Width*config.Height > 1600*1600 {
		return nil
	}
	select {
	case s.coverGate <- struct{}{}:
		defer func() { <-s.coverGate }()
	case <-ctx.Done():
		return nil
	}
	im, _, e := image.Decode(bytes.NewReader(b))
	if e != nil {
		return nil
	}
	bounds := im.Bounds()
	out := make([]byte, size*size*4)
	for y := 0; y < size; y++ {
		for x := 0; x < size; x++ {
			r, g, b, _ := im.At(bounds.Min.X+x*bounds.Dx()/size, bounds.Min.Y+y*bounds.Dy()/size).RGBA()
			i := (y*size + x) * 4
			out[i] = byte(b >> 8)
			out[i+1] = byte(g >> 8)
			out[i+2] = byte(r >> 8)
			out[i+3] = 255
		}
	}
	return out
}
