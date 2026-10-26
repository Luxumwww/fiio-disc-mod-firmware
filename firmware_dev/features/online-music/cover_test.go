package main

import (
	"bytes"
	"embed"
	"image"
	"image/color"
	"image/png"
	"net/http"
	"net/http/httptest"
	"net/url"
	"testing"
)

//go:embed testdata/*.webp
var coverFixtures embed.FS

func TestWebPCoverLossyAndLossless(t *testing.T) {
	for _, name := range []string{"cover-lossy.webp", "cover-lossless.webp"} {
		b, e := coverFixtures.ReadFile("testdata/" + name)
		if e != nil {
			t.Fatal(e)
		}
		r := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { w.Write(b) }))
		s := newServer(t.TempDir(), 1)
		out := s.coverPixels(r.URL, 48)
		r.Close()
		if len(out) != 48*48*4 {
			t.Fatal("WebP was not decoded", name, len(out))
		}
	}
}

func TestCoverThumbnailURLAndTransientRetry(t *testing.T) {
	u, e := url.Parse(sizedCoverURL("http://p1.music.126.net/art.jpg?key=keep&param=1000y1000", 48))
	if e != nil || u.Scheme != "https" || u.Query().Get("param") != "96y96" || u.Query().Get("key") != "keep" {
		t.Fatal(u, e)
	}
	raw := "http://other.example/art?key=keep"
	if sizedCoverURL(raw, 144) != raw {
		t.Fatal("changed unrelated provider")
	}
	var data bytes.Buffer
	im := image.NewRGBA(image.Rect(0, 0, 2, 2))
	im.Set(0, 0, color.RGBA{10, 20, 30, 255})
	png.Encode(&data, im)
	calls := 0
	r := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		calls++
		if calls == 1 {
			http.Error(w, "retry", 503)
			return
		}
		w.Write(data.Bytes())
	}))
	defer r.Close()
	s := newServer(t.TempDir(), 1)
	b := s.coverPixels(r.URL, 48)
	if len(b) != 48*48*4 || calls != 2 || b[0] != 30 || b[1] != 20 || b[2] != 10 || b[3] != 255 {
		t.Fatal(len(b), calls)
	}
}
