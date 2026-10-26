package main

import (
	"bytes"
	"context"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync/atomic"
	"testing"
	"time"
)

func cacheServer(t *testing.T) *server {
	t.Helper()
	s := newServer(t.TempDir(), 49381)
	s.testCardRoot = t.TempDir()
	s.downloadLyric = func(model.Song, string) (string, error) { return "", nil }
	return s
}
func cacheFLAC() []byte {
	b := make([]byte, 128)
	copy(b, "fLaC")
	b[7] = 34
	b[18] = 10
	b[19] = 196
	b[20] = 66
	b[21] = 240
	return b
}
func TestTFPlatformPathsAndSymlinkIsolation(t *testing.T) {
	s := cacheServer(t)
	for _, source := range []string{"netease", "qq", "kugou"} {
		for _, kind := range []string{"歌曲", "封面", "列表"} {
			path, e := s.cacheDirectory(source, kind)
			if e != nil || path != filepath.Join(s.testCardRoot, "在线音频缓存", platformDirectory(source), kind) {
				t.Fatal(path, e)
			}
		}
		path, e := s.downloadDirectory(source)
		if e != nil || path != filepath.Join(s.testCardRoot, "流媒体下载", platformDirectory(source)) {
			t.Fatal(path, e)
		}
	}
	entries, _ := os.ReadDir(s.testCardRoot)
	if len(entries) != 2 {
		t.Fatal("extra TF root directories", entries)
	}
	if _, e := s.cacheDirectory("netease", "../escape"); e == nil {
		t.Fatal("traversal accepted")
	}
	root := t.TempDir()
	outside := t.TempDir()
	if e := os.Symlink(outside, filepath.Join(root, "在线音频缓存")); e != nil {
		t.Fatal(e)
	}
	s.testCardRoot = root
	if _, e := s.cacheDirectory("netease", "歌曲"); e == nil {
		t.Fatal("linked root accepted")
	}
	if entries, _ := os.ReadDir(outside); len(entries) != 0 {
		t.Fatal("wrote outside TF")
	}
	s.testCardRoot = filepath.Join(root, "missing")
	if _, e := s.downloadDirectory("netease"); e == nil {
		t.Fatal("unmounted root recreated")
	}
}
func TestLibraryDiskCacheAccountAndStableSegmentTokens(t *testing.T) {
	s := cacheServer(t)
	p := &mockLibrary{}
	s.providers["netease"] = p
	lists, e := s.library("netease", 1, false)
	if e != nil {
		t.Fatal(e)
	}
	first, e := s.libraryTracks(lists.Generation, 0, 1)
	if e != nil {
		t.Fatal(e)
	}
	second, e := s.libraryTracks(lists.Generation, 0, 2)
	if e != nil {
		t.Fatal(e)
	}
	_, e = s.libraryTracks(lists.Generation, 0, 1)
	if e != nil || p.trackCalls != 2 {
		t.Fatal("reentry fetched network", e, p.trackCalls)
	}
	if e = s.enqueueDownloadItems(fmt.Sprintf("%d:1,%d:2", first.Generation, second.Generation)); e != nil {
		t.Fatal(e)
	}
	a, b := <-s.downloads, <-s.downloads
	if a.Song.ID != "1" || b.Song.ID != "22" {
		t.Fatal("earlier segment selected wrong song", a.Song.ID, b.Song.ID)
	}
	os.WriteFile(filepath.Join(s.credentials, "netease.cookie"), []byte("different-test-account"), 0600)
	if _, _, e = s.playlistChunk("netease", "0", 1); e != nil || p.trackCalls != 3 {
		t.Fatal("account cache shared", e, p.trackCalls)
	}
	s.invalidateAccountPages("netease")
	if e = s.enqueueDownloadItems(fmt.Sprintf("%d:1", first.Generation)); e == nil {
		t.Fatal("account retained old tokens")
	}
	for i := uint64(1); i < 550; i++ {
		s.generation = i
		s.songs = []model.Song{{ID: fmt.Sprint(i)}}
		s.rememberSongsLocked()
	}
	if len(s.songPages) > 512 {
		t.Fatal("unbounded registry")
	}
}
func TestCompleteAudioCacheLocalRangeAndExplicitDownload(t *testing.T) {
	s := cacheServer(t)
	data := cacheFLAC()
	task := downloadTask{model.Song{ID: "123", Name: "测试曲", Artist: "歌手"}, "netease", "hires"}
	resp := &http.Response{StatusCode: 200, ContentLength: int64(len(data)), Body: io.NopCloser(bytes.NewReader(data)), Header: make(http.Header)}
	output := httptest.NewRecorder()
	s.streamAndCache(output, resp, task)
	if !bytes.Equal(output.Body.Bytes(), data) {
		t.Fatal("caching altered streaming")
	}
	path, info, ok := s.cachedAudio(task)
	if !ok || info.Format != "flac" || info.Size != int64(len(data)) {
		t.Fatal("cache not reusable", info)
	}
	song := task.Song
	u, _, _, _, e := s.resolvePlaybackAudio(context.Background(), task.Source, &song, task.Quality)
	if e != nil || u != "file://"+path {
		t.Fatal("cache did not bypass provider", u, e)
	}
	s.streamFiles = map[uint64]string{1: path}
	s.streamTasks = map[uint64]downloadTask{1: task}
	req := httptest.NewRequest("GET", "/audio/1", nil)
	req.Header.Set("Range", "bytes=4-7")
	result := httptest.NewRecorder()
	s.proxy(result, req)
	if result.Code != 206 || !bytes.Equal(result.Body.Bytes(), data[4:8]) {
		t.Fatal("cached Range", result.Code, result.Body.String())
	}
	if e = s.downloadSong(context.Background(), task); e != nil {
		t.Fatal(e)
	}
	dir := filepath.Join(s.testCardRoot, "流媒体下载", "网易云")
	files, _ := os.ReadDir(dir)
	if len(files) != 1 || filepath.Ext(files[0].Name()) != ".flac" {
		t.Fatal("actual download missing", files)
	}
	saved, _ := os.ReadFile(filepath.Join(dir, files[0].Name()))
	if !bytes.Equal(saved, data) {
		t.Fatal("download changed cached song")
	}
}
func TestIncompleteAndPartialStreamsNeverPublishCache(t *testing.T) {
	for _, mode := range []string{"truncated", "partial", "broken-output", "bad-format"} {
		t.Run(mode, func(t *testing.T) {
			s := cacheServer(t)
			data := cacheFLAC()
			task := downloadTask{model.Song{ID: mode}, "qq", "auto"}
			response := &http.Response{StatusCode: 200, ContentLength: int64(len(data)), Body: io.NopCloser(bytes.NewReader(data)), Header: make(http.Header)}
			var out http.ResponseWriter = httptest.NewRecorder()
			switch mode {
			case "truncated":
				response.ContentLength++
			case "partial":
				response.StatusCode = 206
				response.Header.Set("Content-Range", "bytes 10-137/200")
			case "broken-output":
				out = failingAudioOutput{}
			case "bad-format":
				data = bytes.Repeat([]byte("x"), 128)
				response.Body = io.NopCloser(bytes.NewReader(data))
			}
			s.streamAndCache(out, response, task)
			if _, _, ok := s.cachedAudio(task); ok {
				t.Fatal("bad cache published")
			}
			dir, _, _ := s.audioCacheLocation(task)
			files, _ := os.ReadDir(dir)
			if len(files) != 0 {
				t.Fatal("incomplete temp files retained", files)
			}
		})
	}
}

type failingAudioOutput struct{}

func (failingAudioOutput) Header() http.Header       { return make(http.Header) }
func (failingAudioOutput) WriteHeader(int)           {}
func (failingAudioOutput) Write([]byte) (int, error) { return 0, io.ErrClosedPipe }
func TestDownloadProgressCancellationAndCacheEvictionScope(t *testing.T) {
	s := cacheServer(t)
	ctx, cancel := context.WithCancel(context.Background())
	var body bytes.Buffer
	writer := &downloadProgressWriter{server: s, file: &body, song: "song", total: 100, ctx: ctx}
	if _, e := writer.Write(make([]byte, 25)); e != nil || !strings.Contains(s.downloadStatus, "25%") {
		t.Fatal("no real progress", e, s.downloadStatus)
	}
	cancel()
	if _, e := writer.Write([]byte("x")); e == nil {
		t.Fatal("canceled download continued")
	}
	dir, _ := s.cacheDirectory("qq", "歌曲")
	owned := filepath.Join(dir, cacheDigest("owned")+".flac")
	personal := filepath.Join(dir, "personal.flac")
	os.WriteFile(owned, make([]byte, 100), 0644)
	os.WriteFile(personal, []byte("keep"), 0644)
	trimManagedCache(dir, 1)
	if _, e := os.Stat(owned); !os.IsNotExist(e) {
		t.Fatal("budget did not evict app cache")
	}
	if b, _ := os.ReadFile(personal); string(b) != "keep" {
		t.Fatal("user file deleted")
	}
}
func TestCoverTFCacheSurvivesBackendRestartAndAdjacentSegments(t *testing.T) {
	var calls atomic.Int32
	pngBytes, _ := coverFixtures.ReadFile("testdata/cover-lossless.webp")
	web := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { calls.Add(1); w.Write(pngBytes) }))
	defer web.Close()
	s := cacheServer(t)
	got := s.coverPixelsFor(context.Background(), "kugou", web.URL, 48)
	if len(got) != 48*48*4 {
		t.Fatal("cover decode")
	}
	next := newServer(t.TempDir(), 49381)
	next.testCardRoot = s.testCardRoot
	again := next.coverPixelsFor(context.Background(), "kugou", web.URL, 48)
	if !bytes.Equal(got, again) || calls.Load() != 1 {
		t.Fatal("restart lost TF artwork", calls.Load())
	}
	next.songPages = map[uint64]songPage{1: {Songs: []model.Song{{Cover: web.URL}}, Source: "kugou"}, 2: {Songs: []model.Song{{Cover: web.URL}}, Source: "kugou"}}
	next.refreshPagesArt("tracks", "1,2")
	defer next.artCancel()
	deadline := time.Now().Add(time.Second)
	for time.Now().Before(deadline) {
		a, _ := os.ReadFile(filepath.Join(next.dir, "thumb-tracks-1-0.bgra"))
		b, _ := os.ReadFile(filepath.Join(next.dir, "thumb-tracks-2-0.bgra"))
		if bytes.Equal(a, got) && bytes.Equal(b, got) {
			return
		}
		time.Sleep(10 * time.Millisecond)
	}
	t.Fatal("one adjacent segment canceled the other")
}
