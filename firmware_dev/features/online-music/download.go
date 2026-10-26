package main

import (
	"bufio"
	"context"
	"errors"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/utils"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

type downloadTask struct {
	Song    model.Song
	Source  string
	Quality string
}

func (s *server) enqueueDownloads(gen uint64, indices string) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	selected, ok := s.selectedPageLocked(gen)
	if !ok {
		return errors.New("歌曲列表已更新，请重新选择")
	}
	seen := map[int]bool{}
	var tasks []downloadTask
	for _, v := range strings.Split(indices, ",") {
		i, e := strconv.Atoi(v)
		if e != nil || i < 0 || i >= len(selected.Songs) {
			return errors.New("无效下载选择")
		}
		if !seen[i] {
			seen[i] = true
			tasks = append(tasks, downloadTask{cloneSong(selected.Songs[i]), selected.Source, s.quality})
		}
	}
	if len(tasks) > cap(s.downloads)-len(s.downloads) {
		return errors.New("下载队列已满，请等待当前任务完成")
	}
	for _, t := range tasks {
		s.downloads <- t
	}
	return nil
}
func (s *server) setDownload(v string) {
	s.downloadMu.Lock()
	s.downloadStatus = v
	s.downloadMu.Unlock()
	atomicWrite(filepath.Join(s.dir, "download-status"), []byte(clean(v)))
	s.publishPlayback()
}
func (s *server) downloadLoop(ctx context.Context) {
	for {
		select {
		case <-ctx.Done():
			return
		case task := <-s.downloads:
			s.setDownload("下载中：" + task.Song.Name)
			if e := s.downloadSong(ctx, task); e != nil {
				s.setDownload("下载失败：" + e.Error())
			} else {
				s.setDownload("已下载：" + task.Song.Name)
			}
		}
	}
}
func cardDirectory() (string, error) { return mountedCardRoot() }
func (s *server) downloadSong(parent context.Context, task downloadTask) error {
	dir, e := s.downloadDirectory(task.Source)
	if e != nil {
		return e
	}
	return s.downloadToDirectory(parent, task, dir)
}

type downloadProgressWriter struct {
	server         *server
	file           io.Writer
	song           string
	total, written int64
	at             time.Time
	ctx            context.Context
}

func (w *downloadProgressWriter) Write(b []byte) (int, error) {
	if e := w.ctx.Err(); e != nil {
		return 0, e
	}
	n, e := w.file.Write(b)
	w.written += int64(n)
	if time.Since(w.at) >= 500*time.Millisecond {
		w.at = time.Now()
		if w.total > 0 {
			w.server.setDownload(fmt.Sprintf("下载中 %d%%：%s", w.written*100/w.total, w.song))
		} else {
			w.server.setDownload(fmt.Sprintf("下载中 %d KiB：%s", w.written/1024, w.song))
		}
	}
	return n, e
}
func (s *server) downloadToDirectory(parent context.Context, task downloadTask, dir string) error {
	if path, info, ok := s.cachedAudio(task); ok {
		file, e := os.Open(path)
		if e != nil {
			return e
		}
		defer file.Close()
		return s.saveDownloadReader(parent, task, task.Song, file, info.Size, dir, false)
	}
	select {
	case s.gate <- struct{}{}:
	case <-parent.Done():
		return parent.Err()
	}
	song := task.Song
	ctx, cancel := context.WithTimeout(parent, 20*time.Second)
	u, _, _, _, e := s.resolveAudio(ctx, task.Source, &song, task.Quality)
	cancel()
	<-s.gate
	if e != nil {
		return e
	}
	ctx, cancel = context.WithTimeout(parent, 4*time.Minute)
	defer cancel()
	req, e := http.NewRequestWithContext(ctx, "GET", u, nil)
	if e != nil {
		return e
	}
	req.Header.Set("Accept-Encoding", "identity")
	resp, e := (&http.Client{Transport: s.client.Transport}).Do(req)
	if e != nil {
		return e
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		return fmt.Errorf("音频返回%d", resp.StatusCode)
	}
	return s.saveDownloadReader(ctx, task, song, resp.Body, resp.ContentLength, dir, true)
}
func (s *server) saveDownloadReader(ctx context.Context, task downloadTask, song model.Song, body io.Reader, length int64, dir string, verifyQuality bool) error {
	if length > 512<<20 {
		return errors.New("音频文件超过512MiB")
	}
	reader := bufio.NewReaderSize(body, 32768)
	head, e := reader.Peek(32)
	if e != nil && len(head) < 12 {
		return errors.New("音频数据不完整")
	}
	ext, rate, bits := audioFormat(head)
	if ext == "" {
		return errors.New("返回的不是普通音频文件")
	}
	if verifyQuality {
		if e := checkQuality(song.Extra["disc_quality"], ext, rate, bits); e != nil {
			return e
		}
	}
	name := utils.SanitizeFilename(clean(song.Artist) + " - " + clean(song.Name) + " [" + clean(song.ID) + "]")
	final := filepath.Join(dir, name+"."+ext)
	if st, e := os.Stat(final); e == nil && st.Mode().IsRegular() {
		s.saveDownloadMetadata(ctx, task, song, final)
		return nil
	}
	file, e := os.CreateTemp(dir, ".disc-download-")
	if e != nil {
		return e
	}
	tmp := file.Name()
	defer os.Remove(tmp)
	defer file.Close()
	progress := &downloadProgressWriter{server: s, file: file, song: song.Name, total: length, ctx: ctx}
	n, e := io.CopyBuffer(progress, io.LimitReader(reader, (512<<20)+1), make([]byte, 32768))
	if e != nil {
		return e
	}
	if n > 512<<20 {
		return errors.New("音频文件超过限制")
	}
	if length >= 0 && n != length {
		return errors.New("音频下载不完整")
	}
	if e = file.Sync(); e != nil {
		return e
	}
	if e = file.Close(); e != nil {
		return e
	}
	if _, e = os.Stat(final); e == nil {
		return nil
	}
	if e = os.Rename(tmp, final); e != nil {
		return e
	}
	s.saveDownloadMetadata(ctx, task, song, final)
	return nil
}
func (s *server) enqueueDownloadItems(items string) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	var tasks []downloadTask
	seen := map[string]bool{}
	for _, item := range strings.Split(items, ",") {
		parts := strings.Split(item, ":")
		if len(parts) != 2 {
			return errors.New("无效下载选择")
		}
		gen, e := strconv.ParseUint(parts[0], 10, 64)
		if e != nil {
			return errors.New("无效下载选择")
		}
		index, e := strconv.Atoi(parts[1])
		page, ok := s.selectedPageLocked(gen)
		if e != nil || !ok || index < 0 || index >= len(page.Songs) {
			return errors.New("歌曲列表已更新")
		}
		if !seen[item] {
			seen[item] = true
			tasks = append(tasks, downloadTask{cloneSong(page.Songs[index]), page.Source, s.quality})
		}
	}
	if len(tasks) == 0 || len(tasks) > cap(s.downloads)-len(s.downloads) {
		return errors.New("下载任务过多，请等待当前任务完成")
	}
	for _, t := range tasks {
		s.downloads <- t
	}
	return nil
}
