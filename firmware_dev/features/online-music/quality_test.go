package main

import (
	"context"
	"errors"
	"github.com/guohuiyuan/music-lib/model"
	"net/http"
	"net/http/httptest"
	"reflect"
	"strings"
	"testing"
)

type qualityProvider struct {
	mockLibrary
	base        string
	seen        []string
	unavailable map[string]bool
}

func (p *qualityProvider) GetDownloadURL(song *model.Song) (string, error) {
	q := song.Extra["disc_quality"]
	p.seen = append(p.seen, q)
	if p.unavailable[q] {
		return "", errors.New("not available")
	}
	return p.base + "/" + q, nil
}
func qualityFixture(t *testing.T, mode string) (*server, *qualityProvider) {
	t.Helper()
	audio := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		q := strings.TrimPrefix(r.URL.Path, "/")
		if mode == "mp3" {
			if q == "hires" {
				http.Error(w, "no", 403)
				return
			}
			if q == "lossless" {
				w.Write([]byte("<html>not an audio source</html>"))
				return
			}
			w.Write([]byte("ID3" + strings.Repeat("a", 61)))
			return
		}
		b := make([]byte, 64)
		copy(b, "fLaC")
		b[7] = 34
		rate, bits := 44100, 16
		if mode == "hires" {
			rate = 96000
			bits = 24
		}
		packed := uint64(rate)<<44 | uint64(1)<<41 | uint64(bits-1)<<36
		for i := 0; i < 8; i++ {
			b[18+i] = byte(packed >> uint(56-i*8))
		}
		w.Write(b)
	}))
	t.Cleanup(audio.Close)
	s := newServer(t.TempDir(), 1)
	p := &qualityProvider{base: audio.URL}
	s.providers["mock"] = p
	return s, p
}
func TestAutoQualityStartsHighestAndVerifiesDowngradedHeader(t *testing.T) {
	s, p := qualityFixture(t, "lossless")
	song := model.Song{Extra: map[string]string{"disc_quality": "standard", "netease_level": "standard"}}
	_, ext, rate, bits, e := s.resolveAudio(context.Background(), "mock", &song, "auto")
	if e != nil || ext != "flac" || rate != 44100 || bits != 16 || song.Extra["disc_quality"] != "lossless" || !reflect.DeepEqual(p.seen, []string{"hires", "lossless"}) {
		t.Fatal(e, ext, rate, bits, song, p.seen)
	}
	s, p = qualityFixture(t, "hires")
	song = model.Song{}
	if _, _, _, _, e = s.resolveAudio(context.Background(), "mock", &song, "auto"); e != nil || len(p.seen) != 1 || song.Extra["disc_quality"] != "hires" {
		t.Fatal(e, p.seen, song)
	}
}
func TestQualityFallbackOnUnavailableOrInvalidAudio(t *testing.T) {
	s, p := qualityFixture(t, "mp3")
	song := model.Song{}
	_, ext, _, _, e := s.resolveAudio(context.Background(), "mock", &song, "hires")
	if e != nil || ext != "mp3" || song.Extra["disc_quality"] != "exhigh" || !reflect.DeepEqual(p.seen, []string{"hires", "lossless", "exhigh"}) {
		t.Fatal(e, ext, p.seen, song)
	}
	p.seen = nil
	p.unavailable = map[string]bool{"exhigh": true}
	if _, _, _, _, e = s.resolveAudio(context.Background(), "mock", &song, "exhigh"); e != nil || !reflect.DeepEqual(p.seen, []string{"exhigh", "standard"}) {
		t.Fatal(e, p.seen)
	}
	p.seen = nil
	p.unavailable["standard"] = true
	if _, _, _, _, e = s.resolveAudio(context.Background(), "mock", &song, "standard"); e == nil || len(p.seen) != 1 {
		t.Fatal("no source accepted", e, p.seen)
	}
}
