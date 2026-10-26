package main

import (
	"context"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"
)

func cacheDigestKeyInvalid(key string) bool { _, e := hex.DecodeString(key); return e != nil }

type audioCacheInfo struct {
	Source, ID, Format string
	Rate, Bits         int
	Size               int64
}

func (s *server) audioCacheLocation(task downloadTask) (string, string, error) {
	dir, e := s.cacheDirectory(task.Source, "歌曲")
	if e != nil {
		return "", "", e
	}
	key := s.accountCacheKey(task.Source, "audio:"+task.Song.ID+":"+task.Quality)
	return dir, key, nil
}
func (s *server) cachedAudio(task downloadTask) (string, audioCacheInfo, bool) {
	var info audioCacheInfo
	dir, key, e := s.audioCacheLocation(task)
	if e != nil {
		return "", info, false
	}
	b, e := os.ReadFile(filepath.Join(dir, key+".json"))
	if e != nil || len(b) > 4096 || json.Unmarshal(b, &info) != nil {
		return "", info, false
	}
	if info.Source != task.Source || info.ID != task.Song.ID || info.Size <= 0 || info.Size > 512<<20 {
		return "", info, false
	}
	switch info.Format {
	case "flac", "mp3", "wav", "m4a", "ogg", "ape":
	default:
		return "", info, false
	}
	path := filepath.Join(dir, key+".discaudio")
	f, e := os.Open(path)
	if e != nil {
		return "", info, false
	}
	defer f.Close()
	st, e := f.Stat()
	if e != nil || !st.Mode().IsRegular() || st.Size() != info.Size {
		return "", info, false
	}
	head := make([]byte, 64)
	n, _ := f.Read(head)
	ext, _, _ := audioFormat(head[:n])
	if ext != info.Format {
		return "", info, false
	}
	// Touch only on actual play/explicit download, never in the audio callback.
	os.Chtimes(path, time.Now(), time.Now())
	return path, info, true
}
func (s *server) resolvePlaybackAudio(ctx context.Context, source string, song *model.Song, quality string) (string, string, int, int, error) {
	if path, info, ok := s.cachedAudio(downloadTask{*song, source, quality}); ok {
		return "file://" + path, info.Format, info.Rate, info.Bits, nil
	}
	return s.resolveAudio(ctx, source, song, quality)
}

// Cache failures never terminate audible streaming. Only complete, validated
// files are published; Range fragments and canceled connections stay temporary.
type audioCacheWriter struct {
	output io.Writer
	file   *os.File
	failed bool
	count  int64
}

func (w *audioCacheWriter) Write(b []byte) (int, error) {
	n, e := w.output.Write(b)
	if w.file != nil && !w.failed {
		if w.count+int64(n) > 512<<20 {
			w.failed = true
		}
	}
	if w.file != nil && !w.failed {
		k, err := w.file.Write(b[:n])
		if err != nil || k != n {
			w.failed = true
		}
	}
	w.count += int64(n)
	return n, e
}
func fullAudioResponse(resp *http.Response) bool {
	if resp.StatusCode == http.StatusOK {
		return true
	}
	if resp.StatusCode != http.StatusPartialContent {
		return false
	}
	var first, last, total int64
	_, e := fmt.Sscanf(resp.Header.Get("Content-Range"), "bytes %d-%d/%d", &first, &last, &total)
	return e == nil && first == 0 && last+1 == total && total > 0 && total == resp.ContentLength
}
func (s *server) streamAndCache(w http.ResponseWriter, resp *http.Response, task downloadTask) {
	dir, key, e := s.audioCacheLocation(task)
	if e != nil || !fullAudioResponse(resp) || resp.ContentLength > 512<<20 {
		io.CopyBuffer(w, resp.Body, make([]byte, 32768))
		return
	}
	f, e := os.CreateTemp(dir, ".disc-audio-part-")
	if e != nil {
		io.CopyBuffer(w, resp.Body, make([]byte, 32768))
		return
	}
	tmp := f.Name()
	defer os.Remove(tmp)
	writer := &audioCacheWriter{output: w, file: f}
	_, copyErr := io.CopyBuffer(writer, resp.Body, make([]byte, 32768))
	closeErr := f.Close()
	if writer.failed || copyErr != nil || closeErr != nil || writer.count > 512<<20 || writer.count <= 0 || resp.ContentLength >= 0 && writer.count != resp.ContentLength {
		return
	}
	saved, e := os.Open(tmp)
	if e != nil {
		return
	}
	head := make([]byte, 64)
	n, _ := saved.Read(head)
	saved.Close()
	ext, rate, bits := audioFormat(head[:n])
	if ext == "" {
		return
	}
	s.cacheDiskMu.Lock()
	defer s.cacheDiskMu.Unlock()
	final := filepath.Join(dir, key+".discaudio")
	if os.Rename(tmp, final) != nil {
		return
	}
	data, _ := json.Marshal(audioCacheInfo{task.Source, task.Song.ID, ext, rate, bits, writer.count})
	if diskAtomicWrite(filepath.Join(dir, key+".json"), data) != nil {
		os.Remove(final)
		return
	}
	trimManagedCache(dir, 512<<20)
	go s.maybePrefetch()
}

// Rename only validated, app-owned legacy entries. Content/JSON stays intact;
// private suffix prevents the native music scanner from treating cache as music.
func (s *server) migrateAudioCache() {
	for _, source := range []string{"netease", "qq", "kugou"} {
		dir, e := s.cacheDirectory(source, "歌曲")
		if e != nil {
			continue
		}
		entries, _ := os.ReadDir(dir)
		for _, entry := range entries {
			stJSON, err := entry.Info()
			if err != nil || !stJSON.Mode().IsRegular() || stJSON.Size() > 4096 {
				continue
			}
			key := strings.TrimSuffix(entry.Name(), ".json")
			if len(key) != 64 || key == entry.Name() || cacheDigestKeyInvalid(key) {
				continue
			}
			b, e := os.ReadFile(filepath.Join(dir, entry.Name()))
			var info audioCacheInfo
			if e != nil || len(b) > 4096 || json.Unmarshal(b, &info) != nil || info.Source != source || info.Size <= 0 || info.Size > 512<<20 {
				continue
			}
			switch info.Format {
			case "flac", "mp3", "wav", "m4a", "ogg", "ape":
			default:
				continue
			}
			old := filepath.Join(dir, key+"."+info.Format)
			st, e := os.Lstat(old)
			if e != nil || !st.Mode().IsRegular() || st.Size() != info.Size {
				continue
			}
			f, e := os.Open(old)
			if e != nil {
				continue
			}
			head := make([]byte, 64)
			n, _ := f.Read(head)
			f.Close()
			format, _, _ := audioFormat(head[:n])
			if format != info.Format {
				continue
			}
			path := filepath.Join(dir, key+".discaudio")
			if _, e = os.Lstat(path); os.IsNotExist(e) {
				os.Rename(old, path)
			}
		}
	}
}
func (s *server) rememberStreamLocked(id uint64, u string, song model.Song, source, quality string) {
	if s.streamFiles == nil {
		s.streamFiles = map[uint64]string{}
	}
	if s.streamTasks == nil {
		s.streamTasks = map[uint64]downloadTask{}
	}
	if strings.HasPrefix(u, "file://") {
		s.streamFiles[id] = strings.TrimPrefix(u, "file://")
	} else {
		s.streams[id] = u
		s.streamTasks[id] = downloadTask{cloneSong(song), source, quality}
	}
	if id > 16 {
		delete(s.streams, id-16)
		delete(s.streamFiles, id-16)
		delete(s.streamTasks, id-16)
	}
}
