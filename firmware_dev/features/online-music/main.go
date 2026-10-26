// DISC online music: upstream providers run outside the native audio/UI processes.
package main

import (
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"net/http"
	"net/url"
	"os"
	"os/signal"
	"path/filepath"
	"runtime"
	"runtime/debug"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/guohuiyuan/music-lib/kugou"
	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/netease"
	"github.com/guohuiyuan/music-lib/provider"
	"github.com/guohuiyuan/music-lib/qq"
	"github.com/guohuiyuan/music-lib/utils"
)

const version = "0.7.3"
const maxSongs = 30

type result struct {
	Generation uint64       `json:"generation"`
	Songs      []model.Song `json:"songs"`
	Error      string       `json:"error,omitempty"`
}
type server struct {
	mu                sync.Mutex
	generation        uint64
	songs             []model.Song
	source            string
	streamURL         string
	streamID          uint64
	streams           map[uint64]string
	providers         map[string]provider.MusicProvider
	client            *http.Client
	dir               string
	port              int
	gate              chan struct{}
	credentials       string
	qr                *model.QRLoginSession
	listGeneration    uint64
	lists             []model.Playlist
	listSource        string
	browse            playlistContext
	player            playbackState
	quality           string
	coverGate         chan struct{}
	coverMu           sync.Mutex
	coverCache        []coverCacheEntry
	artCancel         context.CancelFunc
	artPages          []string
	lyricGate         chan struct{}
	artKey            string
	artActiveKeys     map[string]bool
	downloads         chan downloadTask
	downloadMu        sync.Mutex
	downloadStatus    string
	lastTransportPoll time.Time
	pcmRead           func() pcmObservation
	rendererControl   string
	timingMu          sync.Mutex
	testCardRoot      string
	songPages         map[uint64]songPage
	listPages         map[uint64]listPage
	streamFiles       map[uint64]string
	streamTasks       map[uint64]downloadTask
	cacheDiskMu       sync.Mutex
	downloadLyric     func(model.Song, string) (string, error)
	prefetchCancel    context.CancelFunc
	prefetchRevision  uint64
	prefetchWorker    bool
	prefetchFactory   func(string) provider.MusicProvider
}

func clean(s string) string {
	s = strings.NewReplacer("\t", " ", "\n", " ", "\r", " ", "\x00", "").Replace(s)
	r := []rune(s)
	if len(r) > 100 {
		r = r[:100]
	}
	return string(r)
}
func atomicWrite(path string, b []byte) error {
	p := path + ".next"
	if err := os.WriteFile(p, b, 0600); err != nil {
		return err
	}
	return os.Rename(p, path)
}
func dnsDial(ctx context.Context, network, address string) (net.Conn, error) {
	// Native DHCP sometimes leaves resolv.conf empty. Prefer its configured DNS;
	// otherwise use a configurable public resolver, without rewriting system networking.
	raw, _ := os.ReadFile("/etc/resolv.conf")
	if strings.Contains(string(raw), "nameserver") {
		return (&net.Dialer{Timeout: 3 * time.Second}).DialContext(ctx, network, address)
	}
	dns := os.Getenv("DISC_ONLINE_DNS")
	if dns == "" {
		dns = "223.5.5.5"
	}
	return (&net.Dialer{Timeout: 3 * time.Second}).DialContext(ctx, network, net.JoinHostPort(dns, "53"))
}
func newServer(dir string, port int) *server {
	dialer := &net.Dialer{Timeout: 5 * time.Second, KeepAlive: 20 * time.Second, Resolver: &net.Resolver{PreferGo: true, Dial: dnsDial}}
	tr := &http.Transport{DialContext: dialer.DialContext, TLSHandshakeTimeout: 6 * time.Second, ResponseHeaderTimeout: 8 * time.Second, MaxIdleConns: 4, MaxIdleConnsPerHost: 2, IdleConnTimeout: 30 * time.Second, DisableCompression: false}
	client := &http.Client{Transport: tr, Timeout: 15 * time.Second}
	// This is an adapter-only copy of upstream. Bound its API bodies and timeouts.
	utils.ConfigureEmbeddedHTTP(client, 2<<20)
	http.DefaultTransport = tr
	http.DefaultClient = client
	readCookie := func(name string) string {
		b, _ := os.ReadFile(filepath.Join(dir, name+".cookie"))
		if len(b) > 16384 {
			return ""
		}
		return strings.TrimSpace(string(b))
	}
	return &server{dir: dir, credentials: dir, port: port, client: client, gate: make(chan struct{}, 1), streams: make(map[uint64]string), quality: "auto", coverGate: make(chan struct{}, 1), lyricGate: make(chan struct{}, 1), downloads: make(chan downloadTask, 30), providers: map[string]provider.MusicProvider{"netease": netease.New(readCookie("netease")), "qq": qq.New(readCookie("qq")), "kugou": kugou.New(readCookie("kugou"))}}
}
func (s *server) search(source, query string) (result, error) {
	p := s.providers[source]
	if p == nil {
		return result{}, errors.New("不支持的平台")
	}
	query = strings.TrimSpace(query)
	if len(query) == 0 || len(query) > 180 {
		return result{}, errors.New("请输入歌曲或歌手，最多 180 字节")
	}
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return result{}, errors.New("正在处理，请稍后")
	}
	cached, hit := s.readLibraryCache(source, "search:"+query, false)
	songs := cached.Songs
	var e error
	if !hit {
		songs, e = p.Search(query)
		if e != nil {
			if old, ok := s.readLibraryCache(source, "search:"+query, true); ok {
				songs = old.Songs
				e = nil
			}
		}
	}
	if e != nil {
		return result{}, e
	}
	if len(songs) > maxSongs {
		songs = songs[:maxSongs]
	}
	if !hit {
		s.writeLibraryCache(source, "search:"+query, libraryResult{Songs: songs})
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	s.generation++
	s.songs = songs
	s.source = source
	s.browse = playlistContext{}
	s.rememberSongsLocked()
	return result{Generation: s.generation, Songs: append([]model.Song(nil), songs...)}, nil
}
func (s *server) resolve(gen uint64, index int) (model.Song, string, uint64, error) {
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return model.Song{}, "", 0, errors.New("正在处理，请稍后")
	}
	s.mu.Lock()
	if gen != s.generation || index < 0 || index >= len(s.songs) {
		s.mu.Unlock()
		return model.Song{}, "", 0, errors.New("搜索结果已更新，请重选")
	}
	song := s.songs[index]
	source := s.source
	s.mu.Unlock()
	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	s.mu.Lock()
	q := s.quality
	s.mu.Unlock()
	u, _, _, _, e := s.resolveAudio(ctx, source, &song, q)
	if e != nil {
		return song, "", 0, e
	}
	parsed, e := url.Parse(u)
	if e != nil || parsed.Host == "" || (parsed.Scheme != "http" && parsed.Scheme != "https") {
		return song, "", 0, errors.New("该歌曲没有可播放地址；可能需要登录或会员")
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	s.streamID++
	s.streamURL = u
	id := s.streamID
	s.streams[id] = u
	if id > 16 {
		delete(s.streams, id-16)
	}
	return song, fmt.Sprintf("http://127.0.0.1:%d/audio/%d.%s", s.port, id, song.Ext), id, nil
}
func (s *server) proxy(w http.ResponseWriter, r *http.Request) {
	if r.Method != "GET" && r.Method != "HEAD" {
		http.Error(w, "method", 405)
		return
	}
	var id uint64
	fmt.Sscanf(strings.TrimPrefix(r.URL.Path, "/audio/"), "%d", &id)
	s.mu.Lock()
	u := s.streams[id]
	file := s.streamFiles[id]
	task := s.streamTasks[id]
	valid := id != 0 && (u != "" || file != "")
	s.mu.Unlock()
	if !valid {
		http.Error(w, "expired", 410)
		return
	}
	if file != "" {
		f, e := os.Open(file)
		if e != nil {
			http.Error(w, "cache unavailable", 410)
			return
		}
		defer f.Close()
		st, e := f.Stat()
		if e != nil {
			http.Error(w, "cache unavailable", 410)
			return
		}
		head := make([]byte, 64)
		n, _ := f.ReadAt(head, 0)
		format, _, _ := audioFormat(head[:n])
		// The private on-disk suffix must not leak into decoder format detection.
		http.ServeContent(w, r, "audio."+format, st.ModTime(), f)
		return
	}
	req, e := http.NewRequestWithContext(r.Context(), r.Method, u, nil)
	if e != nil {
		http.Error(w, "upstream", 502)
		return
	}
	req.Header.Set("User-Agent", "Mozilla/5.0")
	req.Header.Set("Accept-Encoding", "identity")
	if v := r.Header.Get("Range"); v != "" {
		req.Header.Set("Range", v)
	}
	// Timeout applies to API calls; streaming uses per-connect/header bounds and
	// native decoder cancellation, never loads an entire track into RAM.
	streamClient := &http.Client{Transport: s.client.Transport}
	resp, e := streamClient.Do(req)
	if e != nil {
		http.Error(w, "audio connection failed", 502)
		return
	}
	defer resp.Body.Close()
	for _, key := range []string{"Content-Type", "Content-Length", "Content-Range", "Accept-Ranges"} {
		if v := resp.Header.Get(key); v != "" {
			w.Header().Set(key, v)
		}
	}
	w.WriteHeader(resp.StatusCode)
	if r.Method != "HEAD" {
		s.streamAndCache(w, resp, task)
	}
}
func soap(ctx context.Context, control, action, args string) error {
	body := `<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:` + action + ` xmlns:u="urn:schemas-upnp-org:service:AVTransport:1"><InstanceID>0</InstanceID>` + args + `</u:` + action + `></s:Body></s:Envelope>`
	req, _ := http.NewRequestWithContext(ctx, "POST", control, strings.NewReader(body))
	req.Header.Set("Content-Type", `text/xml; charset="utf-8"`)
	req.Header.Set("SOAPACTION", `"urn:schemas-upnp-org:service:AVTransport:1#`+action+`"`)
	// Native SetURI/Play can wait for decoder teardown and network startup.
	// A two-second timeout left a delivered command running after a false error.
	c := http.Client{Timeout: 6 * time.Second}
	resp, e := c.Do(req)
	if e != nil {
		return e
	}
	defer resp.Body.Close()
	io.Copy(io.Discard, io.LimitReader(resp.Body, 32768))
	if resp.StatusCode != 200 {
		return fmt.Errorf("播放器返回 %d", resp.StatusCode)
	}
	return nil
}
func xmlText(s string) string {
	return strings.NewReplacer("&", "&amp;", "<", "&lt;", ">", "&gt;", "\"", "&quot;", "'", "&apos;").Replace(s)
}
func discoverControlOnce(ctx context.Context) (string, error) {
	// Stock renderer binds the Wi-Fi address and rotates ports on re-entry.
	addrs, e := net.InterfaceAddrs()
	if e != nil {
		return "", e
	}
	var ip string
	if wifi, e := net.InterfaceByName("wlan0"); e == nil {
		if wifiAddrs, e := wifi.Addrs(); e == nil {
			addrs = wifiAddrs
		}
	}
	for _, a := range addrs {
		if n, ok := a.(*net.IPNet); ok && n.IP.To4() != nil && !n.IP.IsLoopback() {
			ip = n.IP.String()
			break
		}
	}
	if ip == "" {
		return "", errors.New("请先连接 Wi-Fi")
	}
	conn, e := net.ListenPacket("udp4", "0.0.0.0:0")
	if e != nil {
		return "", e
	}
	defer conn.Close()
	deadline := time.Now().Add(1500 * time.Millisecond)
	if until, ok := ctx.Deadline(); ok && until.Before(deadline) {
		deadline = until
	}
	conn.SetDeadline(deadline)
	dst, _ := net.ResolveUDPAddr("udp4", ip+":1900")
	conn.WriteTo([]byte("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 1\r\nST: urn:schemas-upnp-org:device:MediaRenderer:1\r\n\r\n"), dst)
	buf := make([]byte, 4096)
	for {
		n, peer, e := conn.ReadFrom(buf)
		if e != nil {
			return "", errors.New("接收服务尚未就绪，请稍后再选歌曲")
		}
		host, _, _ := net.SplitHostPort(peer.String())
		if host != ip {
			continue
		}
		for _, line := range strings.Split(string(buf[:n]), "\n") {
			if !strings.HasPrefix(strings.ToLower(line), "location:") {
				continue
			}
			u, e := url.Parse(strings.TrimSpace(strings.SplitN(line, ":", 2)[1]))
			if e == nil && u.Hostname() == ip && u.Scheme == "http" {
				return "http://" + u.Host + "/upnp/control/rendertransport1", nil
			}
		}
		select {
		case <-ctx.Done():
			return "", ctx.Err()
		default:
		}
	}
}
func (s *server) currentRequest(seq uint64) bool {
	b, e := os.ReadFile(filepath.Join(s.dir, "request"))
	return e == nil && strings.HasPrefix(string(b), strconv.FormatUint(seq, 10)+"\tplay\t")
}
func (s *server) play(gen uint64, index int, seq uint64) error {
	s.cancelPrefetch()
	s.mu.Lock()
	selected, ok := s.selectedPageLocked(gen)
	if !ok || index < 0 || index >= len(selected.Songs) {
		s.mu.Unlock()
		return errors.New("歌曲列表已更新，请重选")
	}
	queue := append([]model.Song(nil), selected.Songs...)
	source := selected.Source
	browse := selected.Browse
	s.mu.Unlock()
	ctx, cancel := context.WithTimeout(context.Background(), 8*time.Second)
	defer cancel()
	started := time.Now()
	control, e := s.readyControl(ctx)
	s.recordTiming("renderer_ready", started, e)
	if e != nil {
		return e
	}
	if !s.currentRequest(seq) {
		return errors.New("已取消选歌")
	}
	return s.startTrack(queue, source, browse, index, control, seq)
}
func (s *server) publish(seq uint64, kind, msg string, res result) {
	if kind == "error" {
		msg = publicOnlineError(msg)
	}
	var b strings.Builder
	fmt.Fprintf(&b, "%d\t%s\t%d\t%s\n", seq, kind, res.Generation, clean(msg))
	for i, song := range res.Songs {
		fmt.Fprintf(&b, "%d\t%s\t%s\n", i, clean(song.Name), clean(song.Artist))
	}
	atomicWrite(filepath.Join(s.dir, "response"), []byte(b.String()))
	if kind == "results" {
		urls := make([]string, len(res.Songs))
		for i, t := range res.Songs {
			urls[i] = t.Cover
		}
		s.startBrowseArt("results", res.Generation, urls)
	}
}
func (s *server) fileLoop(ctx context.Context) {
	tick := time.NewTicker(150 * time.Millisecond)
	defer tick.Stop()
	var last uint64
	for {
		select {
		case <-ctx.Done():
			return
		case <-tick.C:
			b, e := os.ReadFile(filepath.Join(s.dir, "request"))
			if e != nil || len(b) > 512 {
				continue
			}
			f := parseRequest(b)
			if len(f) < 2 {
				continue
			}
			seq, e := strconv.ParseUint(f[0], 10, 64)
			if e != nil || seq == last {
				continue
			}
			last = seq
			switch f[1] {
			case "exit":
				if e := s.exitPlayback(); e != nil {
					s.publish(seq, "error", e.Error(), result{})
				} else {
					s.publish(seq, "exited", "已退出在线播放", result{})
				}
			case "queue":
				s.publishQueue(seq)
			case "queue-play":
				if len(f) == 4 {
					rev, _ := strconv.ParseUint(f[2], 10, 64)
					index, _ := strconv.Atoi(f[3])
					if e := s.queuePlay(rev, index); e != nil {
						s.publish(seq, "error", e.Error(), result{})
					} else {
						s.publish(seq, "playing", "已开始播放", result{})
					}
				}
			case "control":
				if len(f) == 4 {
					e := s.control(f[2], f[3])
					if e != nil {
						s.publish(seq, "error", e.Error(), result{})
					} else {
						s.publish(seq, "controlled", "已更新", result{})
					}
					s.publishPlayback()
				}
			case "download-items":
				if len(f) == 4 {
					if e := s.enqueueDownloadItems(f[2]); e != nil {
						s.publish(seq, "error", e.Error(), result{})
					} else {
						s.publish(seq, "download-started", "正在下载到TF卡", result{})
					}
				}
			case "art-pages":
				if len(f) == 4 {
					s.refreshPagesArt(f[2], f[3])
					s.publish(seq, "art-ready", "", result{})
				}
			case "art-page":
				if len(f) == 4 {
					gen, _ := strconv.ParseUint(f[3], 10, 64)
					s.refreshPageArt(f[2], gen)
					s.publish(seq, "art-ready", "", result{})
				}
			case "download":
				if len(f) == 4 {
					gen, _ := strconv.ParseUint(f[2], 10, 64)
					e := s.enqueueDownloads(gen, f[3])
					if e != nil {
						s.publish(seq, "error", e.Error(), result{})
					} else {
						s.publish(seq, "download-started", "正在下载到TF卡", result{})
					}
				}
			case "download-current":
				s.mu.Lock()
				song, source, available, quality := s.player.Song, s.player.Source, len(s.player.Queue) > 0, s.quality
				s.mu.Unlock()
				if !available {
					s.publish(seq, "error", "没有正在播放的歌曲", result{})
				} else {
					select {
					case s.downloads <- downloadTask{song, source, quality}:
						s.publish(seq, "download-started", "正在下载到TF卡", result{})
					default:
						s.publish(seq, "error", "下载队列已满", result{})
					}
				}
			case "recommend":
				if len(f) >= 3 {
					r, e := s.recommended(f[2])
					if e != nil {
						s.publish(seq, "error", e.Error(), result{})
					} else {
						s.publishLibrary(seq, r)
					}
				}
			case "lists", "favorites":
				if len(f) != 4 {
					continue
				}
				page, e := strconv.Atoi(f[3])
				if e != nil {
					continue
				}
				s.publish(seq, "busy", "正在加载歌单…", result{})
				var r libraryResult
				if f[1] == "favorites" {
					r, e = s.favoriteTracks(f[2], page)
				} else {
					r, e = s.library(f[2], page, false)
				}
				if e != nil {
					s.publish(seq, "error", e.Error(), result{})
				} else {
					s.publishLibrary(seq, r)
				}
			case "tracks":
				if len(f) != 4 {
					continue
				}
				gen, _ := strconv.ParseUint(f[2], 10, 64)
				parts := strings.Split(f[3], ",")
				if len(parts) != 2 {
					continue
				}
				idx, _ := strconv.Atoi(parts[0])
				page, _ := strconv.Atoi(parts[1])
				s.publish(seq, "busy", "正在加载歌曲…", result{})
				r, e := s.libraryTracks(gen, idx, page)
				if e != nil {
					s.publish(seq, "error", e.Error(), result{})
				} else {
					s.publishLibrary(seq, r)
				}
			case "account":
				if len(f) >= 3 && s.providers[f[2]] != nil {
					s.publish(seq, "account", s.accountMessage(f[2]), result{})
				}
			case "logout":
				if len(f) < 3 {
					continue
				}
				if e := s.logout(f[2]); e != nil {
					s.publish(seq, "error", e.Error(), result{})
				} else {
					s.publish(seq, "logged-out", "已退出登录", result{})
				}
			case "login":
				if len(f) < 3 {
					continue
				}
				s.publish(seq, "busy", "正在生成二维码…", result{})
				e := s.startLogin(f[2])
				if e != nil {
					s.publish(seq, "error", e.Error(), result{})
				} else {
					s.publish(seq, "qr", "用对应音乐应用扫码登录", result{})
				}
			case "login-check":
				if len(f) < 3 {
					continue
				}
				kind, msg, e := s.checkLogin(f[2])
				if e != nil {
					s.publish(seq, "error", e.Error(), result{})
				} else {
					s.publish(seq, kind, msg, result{})
				}
			case "search":
				if len(f) != 4 {
					continue
				}
				s.publish(seq, "busy", "正在搜索…", result{})
				r, e := s.search(f[2], f[3])
				if e != nil {
					s.publish(seq, "error", e.Error(), result{})
				} else {
					s.publish(seq, "results", fmt.Sprintf("找到 %d 首", len(r.Songs)), r)
				}
			case "play":
				if len(f) != 4 {
					continue
				}
				gen, _ := strconv.ParseUint(f[2], 10, 64)
				idx, _ := strconv.Atoi(f[3])
				s.publish(seq, "busy", "正在连接歌曲…", result{})
				e := s.play(gen, idx, seq)
				if e != nil {
					s.publish(seq, "error", e.Error(), result{})
				} else {
					s.publish(seq, "playing", "已开始播放", result{})
				}
			}
		}
	}
}
func main() {
	dir := flag.String("state", "/tmp/disc-online-music", "state/IPC directory")
	port := flag.Int("port", 49381, "loopback proxy port")
	probe := flag.String("probe", "", "search keyword, output JSON and exit")
	source := flag.String("source", "netease", "probe source")
	resolveProbe := flag.Int("resolve-index", -1, "probe one returned track's audio headers")
	credentials := flag.String("credentials", "/usr/data/disc-runtime/module-data/disc-online-music", "private persistent credentials directory")
	qrProbe := flag.Bool("qr-probe", false, "create one QR bitmap and exit; no login authorization")
	libraryProbe := flag.Bool("library-probe", false, "probe saved-account playlists and paged songs; output counts only")
	repairDownloads := flag.Bool("repair-downloads", false, "backfill generated downloads from saved catalog IDs")
	flag.Parse()
	runtime.GOMAXPROCS(1)
	debug.SetMemoryLimit(24 << 20)
	debug.SetGCPercent(50)
	os.MkdirAll(*dir, 0700)
	syscall.Setpriority(syscall.PRIO_PROCESS, 0, 10)
	s := newServer(*dir, *port)
	s.loadCredentials(*credentials)
	if *repairDownloads {
		s.migrateAudioCache()
		a, b := s.repairDownloads(context.Background())
		fmt.Printf("matched=%d cover_saved=%d\n", a, b)
		return
	}
	if b, e := os.ReadFile(filepath.Join(*credentials, "quality")); e == nil {
		q := strings.TrimSpace(string(b))
		if q == "auto" || q == "standard" || q == "exhigh" || q == "lossless" || q == "hires" {
			s.quality = q
		}
	}
	if *libraryProbe {
		r, e := s.library(*source, 1, false)
		if e != nil {
			fmt.Fprintln(os.Stderr, e)
			os.Exit(1)
		}
		fmt.Printf("personal playlists=%d more=%t\n", len(r.Lists), r.More)
		if len(r.Lists) > 0 {
			tracks, e := s.libraryTracks(r.Generation, 0, 1)
			if e != nil {
				fmt.Fprintln(os.Stderr, e)
				os.Exit(1)
			}
			fmt.Printf("first playlist page1 songs=%d more=%t\n", len(tracks.Songs), tracks.More)
			if tracks.More {
				next, e := s.libraryTracks(r.Generation, 0, 2)
				if e != nil {
					fmt.Fprintln(os.Stderr, e)
					os.Exit(1)
				}
				same := len(next.Songs) > 0 && len(tracks.Songs) > 0 && next.Songs[0].ID == tracks.Songs[0].ID
				fmt.Printf("first playlist page2 songs=%d repeated_first=%t\n", len(next.Songs), same)
				if same {
					os.Exit(2)
				}
			}
		}
		return
	}
	if *qrProbe {
		if e := s.startLogin(*source); e != nil {
			fmt.Fprintln(os.Stderr, e)
			os.Exit(1)
		}
		fmt.Printf("QR bitmap generated for %s: %d bytes\n", *source, qrSize*qrSize*4)
		return
	}
	if *probe != "" {
		r, e := s.search(*source, *probe)
		if e != nil {
			r.Error = e.Error()
		}
		json.NewEncoder(os.Stdout).Encode(r)
		if e == nil && *resolveProbe >= 0 {
			song, _, id, err := s.resolve(r.Generation, *resolveProbe)
			if err != nil {
				fmt.Fprintln(os.Stderr, err)
				os.Exit(1)
			}
			req, _ := http.NewRequest("GET", s.streams[id], nil)
			req.Header.Set("Range", "bytes=0-63")
			resp, err := s.client.Do(req)
			if err != nil {
				fmt.Fprintln(os.Stderr, err)
				os.Exit(1)
			}
			b, _ := io.ReadAll(io.LimitReader(resp.Body, 64))
			resp.Body.Close()
			json.NewEncoder(os.Stdout).Encode(map[string]interface{}{"song": song.Name, "ext": song.Ext, "audio_status": resp.StatusCode, "prefix_bytes": len(b), "content_type": resp.Header.Get("Content-Type")})
		}
		if e != nil {
			os.Exit(1)
		}
		return
	}
	for _, name := range []string{"request", "response", "active", "nowplaying", "cover.revision", "lyrics", "session"} {
		os.Remove(filepath.Join(*dir, name))
	}
	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGTERM, syscall.SIGINT)
	defer stop()
	mux := http.NewServeMux()
	mux.HandleFunc("/audio/", s.proxy)
	mux.HandleFunc("/health", func(w http.ResponseWriter, r *http.Request) { fmt.Fprintf(w, "DISC online music %s\n", version) })
	listener, e := net.Listen("tcp4", fmt.Sprintf("127.0.0.1:%d", *port))
	if e != nil {
		log.Fatal(e)
	}
	httpServer := &http.Server{Handler: mux, ReadHeaderTimeout: 3 * time.Second, IdleTimeout: 20 * time.Second, MaxHeaderBytes: 8192}
	// Record endian/target in state so native UI can reject an unavailable helper.
	atomicWrite(filepath.Join(*dir, "ready"), []byte(fmt.Sprintf("%s %d\n", version, os.Getpid())))
	s.migrateAudioCache()
	go s.fileLoop(ctx)
	go s.playbackLoop(ctx)
	go s.downloadLoop(ctx)
	go httpServer.Serve(listener)
	<-ctx.Done()
	httpServer.Close()
	os.Remove(filepath.Join(*dir, "ready"))
}
