package main

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"syscall"
	"time"
)

func platformDirectory(source string) string {
	switch source {
	case "netease":
		return "网易云"
	case "qq":
		return "QQ音乐"
	case "kugou":
		return "酷狗音乐"
	}
	return ""
}
func mountedCardRoot() (string, error) {
	b, e := os.ReadFile("/proc/mounts")
	if e != nil {
		return "", e
	}
	for _, line := range strings.Split(string(b), "\n") {
		f := strings.Fields(line)
		if len(f) >= 4 && f[1] == "/tmp/sdcard" && strings.HasPrefix(f[0], "/dev/mmcblk") && strings.Contains(","+f[3]+",", ",rw,") {
			var st syscall.Statfs_t
			if e := syscall.Statfs(f[1], &st); e != nil {
				return "", e
			}
			if uint64(st.Bavail)*uint64(st.Bsize) < 64<<20 {
				return "", errors.New("TF卡空间不足")
			}
			return f[1], nil
		}
	}
	return "", errors.New("TF卡未挂载或只读")
}
func (s *server) cardRoot() (string, error) {
	if s.testCardRoot != "" {
		return s.testCardRoot, nil
	}
	return mountedCardRoot()
}
func safeCardDirectory(root string, parts ...string) (string, error) {
	root, e := filepath.EvalSymlinks(root)
	if e != nil {
		return "", e
	}
	dir := root
	for _, part := range parts {
		if part == "" || part == "." || part == ".." || strings.ContainsAny(part, "/\\") {
			return "", errors.New("无效TF目录")
		}
		dir = filepath.Join(dir, part)
		if e := os.Mkdir(dir, 0755); e != nil && !os.IsExist(e) {
			return "", e
		}
		actual, e := filepath.EvalSymlinks(dir)
		if e != nil {
			return "", e
		}
		if actual != dir {
			return "", errors.New("TF目录不能是链接")
		}
	}
	return dir, nil
}
func (s *server) cacheDirectory(source, kind string) (string, error) {
	name := platformDirectory(source)
	if name == "" {
		return "", errors.New("未知平台")
	}
	root, e := s.cardRoot()
	if e != nil {
		return "", e
	}
	return safeCardDirectory(root, "在线音频缓存", name, kind)
}
func (s *server) downloadDirectory(source string) (string, error) {
	name := platformDirectory(source)
	if name == "" {
		return "", errors.New("未知平台")
	}
	root, e := s.cardRoot()
	if e != nil {
		return "", e
	}
	return safeCardDirectory(root, "流媒体下载", name)
}
func cacheDigest(key string) string { h := sha256.Sum256([]byte(key)); return hex.EncodeToString(h[:]) }
func (s *server) accountCacheKey(source, key string) string {
	b, _ := os.ReadFile(filepath.Join(s.credentials, source+".cookie"))
	return cacheDigest("v1:" + source + ":" + cacheDigest(string(b)) + ":" + key)
}
func diskAtomicWrite(path string, b []byte) error {
	f, e := os.CreateTemp(filepath.Dir(path), ".disc-part-")
	if e != nil {
		return e
	}
	tmp := f.Name()
	defer os.Remove(tmp)
	if _, e = f.Write(b); e != nil {
		f.Close()
		return e
	}
	if e = f.Close(); e != nil {
		return e
	}
	return os.Rename(tmp, path)
}

type cachedLibrary struct {
	Saved  time.Time
	Result libraryResult
}

func (s *server) readLibraryCache(source, key string, stale bool) (libraryResult, bool) {
	dir, e := s.cacheDirectory(source, "列表")
	if e != nil {
		return libraryResult{}, false
	}
	f, e := os.Open(filepath.Join(dir, s.accountCacheKey(source, key)+".json"))
	if e != nil {
		return libraryResult{}, false
	}
	defer f.Close()
	var c cachedLibrary
	if json.NewDecoder(io.LimitReader(f, 2<<20)).Decode(&c) != nil || len(c.Result.Songs) > 30 || len(c.Result.Lists) > 30 {
		return libraryResult{}, false
	}
	ttl := 15 * time.Minute
	if stale {
		ttl = 7 * 24 * time.Hour
	}
	if c.Saved.IsZero() || time.Since(c.Saved) < 0 || time.Since(c.Saved) > ttl {
		return libraryResult{}, false
	}
	return c.Result, true
}
func (s *server) writeLibraryCache(source, key string, r libraryResult) {
	dir, e := s.cacheDirectory(source, "列表")
	if e != nil {
		return
	}
	r.Generation = 0
	r.ListGeneration = 0
	b, e := json.Marshal(cachedLibrary{time.Now(), r})
	if e == nil && len(b) <= 2<<20 {
		diskAtomicWrite(filepath.Join(dir, s.accountCacheKey(source, key)+".json"), b)
		trimManagedCache(dir, 32<<20)
	}
}

// Only app-owned hash filenames are eligible for LRU eviction. No traversal,
// symlinks, user music or explicit downloaded files are removed.
func trimManagedCache(dir string, budget int64) {
	files, _ := os.ReadDir(dir)
	var entries []os.FileInfo
	var total int64
	for _, f := range files {
		name := f.Name()
		stem := strings.SplitN(name, ".", 2)[0]
		if len(stem) != 64 || f.Type()&os.ModeSymlink != 0 || f.IsDir() {
			continue
		}
		if _, e := hex.DecodeString(stem); e != nil {
			continue
		}
		info, e := f.Info()
		if e != nil {
			continue
		}
		entries = append(entries, info)
		total += info.Size()
	}
	sort.Slice(entries, func(i, j int) bool { return entries[i].ModTime().Before(entries[j].ModTime()) })
	for _, f := range entries {
		if total <= budget {
			break
		}
		if os.Remove(filepath.Join(dir, f.Name())) == nil {
			total -= f.Size()
		}
	}
}
