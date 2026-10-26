package main

import (
	"bytes"
	"context"
	"encoding/binary"
	"encoding/json"
	"github.com/guohuiyuan/music-lib/model"
	"image"
	"image/jpeg"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func realFLAC() ([]byte, []byte) {
	frame := []byte{255, 248, 11, 22, 33, 44, 55}
	b := make([]byte, 42)
	copy(b, "fLaC")
	b[4] = 128
	b[7] = 34
	copy(b[18:22], cacheFLAC()[18:22])
	return append(b, frame...), frame
}
func flacAudio(b []byte) ([]byte, []flacBlock) {
	if len(b) < 4 {
		return nil, nil
	}
	pos := 4
	var blocks []flacBlock
	for pos+4 <= len(b) {
		h := b[pos : pos+4]
		n := int(h[1])<<16 | int(h[2])<<8 | int(h[3])
		pos += 4
		if n > len(b)-pos {
			return nil, nil
		}
		blocks = append(blocks, flacBlock{h[0] & 127, b[pos : pos+n]})
		pos += n
		if h[0]&128 != 0 {
			return b[pos:], blocks
		}
	}
	return nil, nil
}
func TestPrivateCacheMigrationAndProxyFormat(t *testing.T) {
	s := cacheServer(t)
	task := downloadTask{model.Song{ID: "legacy"}, "netease", "auto"}
	dir, key, _ := s.audioCacheLocation(task)
	b := cacheFLAC()
	old := filepath.Join(dir, key+".flac")
	os.WriteFile(old, b, 0644)
	meta, _ := json.Marshal(audioCacheInfo{"netease", "legacy", "flac", 44100, 16, int64(len(b))})
	os.WriteFile(filepath.Join(dir, key+".json"), meta, 0644)
	personal := filepath.Join(dir, "personal.flac")
	os.WriteFile(personal, b, 0644)
	s.migrateAudioCache()
	path, _, ok := s.cachedAudio(task)
	if !ok || filepath.Ext(path) != ".discaudio" {
		t.Fatal("not migrated", path, ok)
	}
	if _, e := os.Stat(old); !os.IsNotExist(e) {
		t.Fatal("old scan-visible file remains")
	}
	if _, e := os.Stat(personal); e != nil {
		t.Fatal("personal file touched")
	}
	s.streamFiles = map[uint64]string{1: path}
	w := httptest.NewRecorder()
	s.proxy(w, httptest.NewRequest("GET", "/audio/1", nil))
	if !bytes.Equal(w.Body.Bytes(), b) || w.Header().Get("Content-Type") != "audio/flac" {
		t.Fatal("private cache playback", w.Header())
	}
	s.migrateAudioCache()
	got, _ := os.ReadFile(path)
	if !bytes.Equal(got, b) {
		t.Fatal("migration altered bytes")
	}
}
func TestEmbeddedFLACAndMP3KeepAudioAndExistingFrames(t *testing.T) {
	song := model.Song{Name: "中文歌曲", Artist: "艺术家", Album: "专辑"}
	jpg := []byte{255, 216, 255, 217}
	b, frame := realFLAC()
	var out bytes.Buffer
	if e := writeFLACMetadata(&out, bytes.NewReader(b), song, jpg); e != nil {
		t.Fatal(e)
	}
	audio, blocks := flacAudio(out.Bytes())
	if !bytes.Equal(audio, frame) || len(blocks) != 3 || blocks[1].kind != 4 || blocks[2].kind != 6 {
		t.Fatal("FLAC frames/metadata")
	}
	var again bytes.Buffer
	if e := writeFLACMetadata(&again, bytes.NewReader(out.Bytes()), song, jpg); e != nil || !bytes.Equal(again.Bytes(), out.Bytes()) {
		t.Fatal("duplicate FLAC tags", e)
	}
	for _, v := range []byte{3, 4} {
		original := id3Frame(v, "TXXX", []byte{0, 'o', 'l', 'd', 0, 'v'})
		header := append([]byte{'I', 'D', '3', v, 0, 0}, syncSize(len(original)+16)...)
		input := append(header, original...)
		input = append(input, make([]byte, 16)...)
		input = append(input, frame...)
		var mp3 bytes.Buffer
		if e := writeMP3Metadata(&mp3, bytes.NewReader(input), song, jpg); e != nil {
			t.Fatal(e)
		}
		size, _ := syncSafe(mp3.Bytes()[6:10])
		if !bytes.Equal(mp3.Bytes()[10:10+len(original)], original) || !bytes.Equal(mp3.Bytes()[10+size:], frame) || !bytes.Contains(mp3.Bytes(), []byte("APIC")) {
			t.Fatal("ID3/audio changed")
		}
	}
	dir := t.TempDir()
	bad := filepath.Join(dir, "bad.flac")
	os.WriteFile(bad, []byte("fLaCbroken"), 0644)
	if embedDownloadMetadata(bad, song, jpg) == nil {
		t.Fatal("bad FLAC accepted")
	}
	got, _ := os.ReadFile(bad)
	if string(got) != "fLaCbroken" {
		t.Fatal("bad file overwritten")
	}
}
func TestDownloadCoverLyricsAndOfflineCacheSeparation(t *testing.T) {
	var jpg bytes.Buffer
	jpeg.Encode(&jpg, image.NewRGBA(image.Rect(0, 0, 8, 8)), nil)
	web := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { w.Write(jpg.Bytes()) }))
	defer web.Close()
	s := cacheServer(t)
	s.downloadLyric = func(model.Song, string) (string, error) { return "[00:01.250]一句歌词\n[00:02.00]第二句", nil }
	song := model.Song{ID: "321", Name: "测试", Artist: "歌手", Cover: web.URL}
	task := downloadTask{song, "netease", "auto"}
	dir, _ := s.downloadDirectory("netease")
	b, frame := realFLAC()
	if e := s.saveDownloadReader(context.Background(), task, song, bytes.NewReader(b), int64(len(b)), dir, false); e != nil {
		t.Fatal(e)
	}
	files, _ := os.ReadDir(dir)
	if len(files) != 3 {
		t.Fatal("missing cover/lyrics", files)
	}
	for _, f := range files {
		data, _ := os.ReadFile(filepath.Join(dir, f.Name()))
		switch filepath.Ext(f.Name()) {
		case ".flac":
			audio, blocks := flacAudio(data)
			if !bytes.Equal(audio, frame) || len(blocks) != 3 {
				t.Fatal("changed audio")
			}
		case ".jpg":
			if _, e := jpeg.Decode(bytes.NewReader(data)); e != nil {
				t.Fatal(e)
			}
		case ".lrc":
			if !strings.Contains(string(data), "[00:01.25]一句歌词") {
				t.Fatal("native lyrics", string(data))
			}
		}
	}
	resp := &http.Response{StatusCode: 200, ContentLength: int64(len(b)), Body: io.NopCloser(bytes.NewReader(b)), Header: http.Header{}}
	s.streamAndCache(httptest.NewRecorder(), resp, task)
	path, _, ok := s.cachedAudio(task)
	if !ok {
		t.Fatal("cache")
	}
	cached, _ := os.ReadFile(path)
	if !bytes.Equal(cached, b) {
		t.Fatal("metadata changed online cache")
	}
	if publicOnlineError("Post \"http://192.168.1.5:123/control\": timeout") != "连接暂时失败，请稍后重试" {
		t.Fatal("raw endpoint shown")
	}
	// FLAC PICTURE length field points to complete JPEG.
	_, blocks := flacAudio(mustRead(t, filepath.Join(dir, "歌手 - 测试 [321].flac")))
	pic := blocks[2].data
	mime := int(binary.BigEndian.Uint32(pic[4:8]))
	offset := 8 + mime
	desc := int(binary.BigEndian.Uint32(pic[offset : offset+4]))
	offset += 4 + desc + 16
	n := int(binary.BigEndian.Uint32(pic[offset : offset+4]))
	if n != len(pic)-offset-4 {
		t.Fatal("PICTURE size")
	}
}
func mustRead(t *testing.T, p string) []byte {
	t.Helper()
	b, e := os.ReadFile(p)
	if e != nil {
		t.Fatal(e)
	}
	return b
}
