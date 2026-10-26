package main

import (
	"io"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/netease"
	"github.com/guohuiyuan/music-lib/utils"
)

type responseTransport struct{}

func (responseTransport) RoundTrip(r *http.Request) (*http.Response, error) {
	return &http.Response{StatusCode: 200, Header: make(http.Header), Body: io.NopCloser(strings.NewReader(`{"result":{"songs":[{"id":3362629877,"name":"32bit regression","dt":10000,"privilege":{"fl":128000},"l":{"size":160000}}]}}`))}, nil
}
func TestNeteaseLargeSongID(t *testing.T) {
	utils.ConfigureEmbeddedHTTP(&http.Client{Transport: responseTransport{}}, 2<<20)
	songs, e := netease.New("").Search("test")
	if e != nil || len(songs) != 1 || songs[0].ID != "3362629877" {
		t.Fatalf("32bit ID parse failed: %v %v", songs, e)
	}
}
func TestQRBitmap(t *testing.T) {
	b, e := qrPixels(&model.QRLoginSession{URL: "https://music.163.com/login?codekey=example"}, &http.Client{})
	if e != nil || len(b) != qrSize*qrSize*4 {
		t.Fatal(e, len(b))
	}
	for i := 3; i < len(b); i += 4 {
		if b[i] != 255 {
			t.Fatal("nonopaque QR")
		}
	}
}

type mockProvider struct {
	called int
	url    string
}

func (p *mockProvider) Search(q string) ([]model.Song, error) {
	p.called++
	return []model.Song{{ID: q, Name: "A < B & C", Artist: "人声", Ext: "mp3"}}, nil
}
func (p *mockProvider) Parse(q string) (*model.Song, error) { return &model.Song{}, nil }
func (p *mockProvider) GetDownloadURL(s *model.Song) (string, error) {
	return p.url, nil
}
func (p *mockProvider) GetLyrics(s *model.Song) (string, error) { return "", nil }
func TestStaleSelectionAndPreviousResource(t *testing.T) {
	s := newServer(t.TempDir(), 49381)
	audio := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { io.WriteString(w, "ID3"+strings.Repeat("a", 61)) }))
	defer audio.Close()
	p := &mockProvider{url: audio.URL}
	s.providers["test"] = p
	r, e := s.search("test", "one")
	if e != nil {
		t.Fatal(e)
	}
	_, _, first, e := s.resolve(r.Generation, 0)
	if e != nil {
		t.Fatal(e)
	}
	second, e := s.search("test", "two")
	if e != nil {
		t.Fatal(e)
	}
	if _, _, _, e = s.resolve(r.Generation, 0); e == nil {
		t.Fatal("stale search selection accepted")
	}
	if s.streams[first] == "" {
		t.Fatal("search invalidated playing/history URI")
	}
	if _, _, _, e = s.resolve(second.Generation, -1); e == nil {
		t.Fatal("negative index accepted")
	}
	if _, _, _, e = s.resolve(second.Generation, 99); e == nil {
		t.Fatal("out of bounds accepted")
	}
}
func TestStreamProxyRangesAndHistory(t *testing.T) {
	upstream := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.Header.Get("Range") != "bytes=10-19" {
			t.Error("Range lost")
		}
		if r.Header.Get("Accept-Encoding") != "identity" {
			t.Error("audio compressed")
		}
		w.Header().Set("Content-Range", "bytes 10-19/100")
		w.Header().Set("Content-Type", "audio/mpeg")
		w.WriteHeader(206)
		io.WriteString(w, "0123456789")
	}))
	defer upstream.Close()
	s := newServer(t.TempDir(), 49381)
	s.streams[1] = upstream.URL
	s.streamID = 2
	s.streams[2] = upstream.URL
	req := httptest.NewRequest("GET", "/audio/1.mp3", nil)
	req.Header.Set("Range", "bytes=10-19")
	w := httptest.NewRecorder()
	s.proxy(w, req)
	if w.Code != 206 || w.Body.String() != "0123456789" || w.Header().Get("Content-Range") != "bytes 10-19/100" {
		t.Fatal(w)
	}
	w = httptest.NewRecorder()
	s.proxy(w, httptest.NewRequest("GET", "/audio/999.mp3", nil))
	if w.Code != 410 {
		t.Fatal("unknown resource accepted")
	}
}
func TestBoundedAPIAndSafeMetadata(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { io.WriteString(w, strings.Repeat("x", 65)) }))
	defer srv.Close()
	utils.ConfigureEmbeddedHTTP(srv.Client(), 64)
	if _, e := utils.Get(srv.URL); e == nil {
		t.Fatal("oversize provider body accepted")
	}
	if xmlText("<&\"") != "&lt;&amp;&quot;" {
		t.Fatal("invalid metadata escape")
	}
}
