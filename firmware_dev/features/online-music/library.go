package main

import (
	"errors"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/provider"
	"path/filepath"
	"strings"
)

const libraryPageSize = 14 // QQ merges up to two categories plus its favorite list.
const trackPageSize = 20

type pagedSongs interface {
	GetPlaylistSongsPage(string, int, int) ([]model.Song, int, error)
}
type libraryResult struct {
	Kind           string
	Generation     uint64
	Page           int
	More           bool
	Title          string
	Lists          []model.Playlist
	Songs          []model.Song
	ListGeneration uint64
	ListIndex      int
	Total          int
}

func validPage(page int) bool { return page >= 1 && page <= 1000 }
func favoriteList(p model.Playlist) bool {
	return p.ID == "profile:favorites" || p.Extra["favorite"] == "true" || strings.Contains(p.Name, "喜欢") || strings.Contains(p.Name, "最爱")
}
func (s *server) library(source string, page int, favorite bool) (libraryResult, error) {
	if !validPage(page) {
		return libraryResult{}, errors.New("无效页码")
	}
	p, ok := s.providers[source].(provider.UserPlaylistProvider)
	if !ok {
		return libraryResult{}, errors.New("平台尚不支持个人歌单")
	}
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return libraryResult{}, errors.New("正在处理，请稍后")
	}
	key := fmt.Sprintf("lists:%d:%t", page, favorite)
	cached, hit := s.readLibraryCache(source, key, false)
	lists := cached.Lists
	var e error
	if !hit {
		lists, e = p.GetUserPlaylists(page, libraryPageSize)
		if e != nil {
			if old, ok := s.readLibraryCache(source, key, true); ok {
				lists = old.Lists
				e = nil
			}
		}
	}
	if e != nil {
		return libraryResult{}, fmt.Errorf("歌单加载失败：%w", e)
	}
	more := len(lists) >= libraryPageSize
	// Preserve all categories returned by a provider rather than silently dropping
	// collected lists. Providers receive a small network page size; cap protects IPC.
	if len(lists) > maxSongs {
		return libraryResult{}, errors.New("平台返回歌单过多，请稍后重试")
	}
	if favorite {
		found := []model.Playlist{}
		for _, l := range lists {
			if favoriteList(l) {
				found = append(found, l)
			}
		}
		if len(found) == 0 {
			return libraryResult{}, errors.New("未找到喜欢的音乐，请从我的歌单选择")
		}
		lists = found
		more = false
	}
	s.mu.Lock()
	s.listGeneration++
	s.lists = lists
	s.listSource = source
	s.rememberListsLocked()
	gen := s.listGeneration
	s.mu.Unlock()
	title := "我的歌单"
	if favorite {
		title = "我的收藏"
	}
	r := libraryResult{Kind: "lists", Generation: gen, Page: page, More: more, Title: title, Lists: lists}
	if !hit {
		s.writeLibraryCache(source, key, r)
	}
	return r, nil
}
func (s *server) libraryTracks(gen uint64, index, page int) (libraryResult, error) {
	if !validPage(page) {
		return libraryResult{}, errors.New("无效页码")
	}
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return libraryResult{}, errors.New("正在处理，请稍后")
	}
	s.mu.Lock()
	saved, exists := s.listPages[gen]
	if !exists && gen == s.listGeneration {
		saved = listPage{s.lists, s.listSource}
		exists = true
	}
	if !exists || index < 0 || index >= len(saved.Lists) {
		s.mu.Unlock()
		return libraryResult{}, errors.New("歌单列表已更新，请重新选择")
	}
	list := saved.Lists[index]
	source := saved.Source
	s.mu.Unlock()
	songs, total, e := s.playlistChunk(source, list.ID, page)
	if e != nil {
		return libraryResult{}, fmt.Errorf("歌曲加载失败：%w", e)
	}
	if len(songs) > trackPageSize {
		return libraryResult{}, errors.New("平台未按页返回歌曲")
	}
	if total <= 0 && list.TrackCount > 0 {
		total = list.TrackCount
	}
	more := len(songs) >= trackPageSize
	if total > 0 {
		more = page*trackPageSize < total
	}
	s.mu.Lock()
	s.generation++
	s.songs = songs
	s.source = source
	s.browse = playlistContext{ID: list.ID, Title: list.Name, Page: page, More: more, Total: total}
	s.rememberSongsLocked()
	songGen := s.generation
	s.mu.Unlock()
	return libraryResult{Kind: "tracks", Generation: songGen, Page: page, More: more, Title: list.Name, Songs: songs, ListGeneration: gen, ListIndex: index, Total: total}, nil
}
func (s *server) favoriteTracks(source string, page int) (libraryResult, error) {
	lists, e := s.library(source, 1, true)
	if e != nil {
		return libraryResult{}, e
	}
	return s.libraryTracks(lists.Generation, 0, page)
}
func (s *server) publishLibrary(seq uint64, r libraryResult) {
	var b strings.Builder
	fmt.Fprintf(&b, "%d\t%s\t%d\t%s\n@\t%d\t%t\t%d\t%d\t%d\n", seq, r.Kind, r.Generation, clean(r.Title), r.Page, r.More, r.ListGeneration, r.ListIndex, r.Total)
	for i, l := range r.Lists {
		fmt.Fprintf(&b, "%d\t%s\t%d 首\n", i, clean(l.Name), l.TrackCount)
	}
	for i, t := range r.Songs {
		fmt.Fprintf(&b, "%d\t%s\t%s\n", i, clean(t.Name), clean(t.Artist))
	}
	atomicWrite(filepath.Join(s.dir, "response"), []byte(b.String()))
	urls := make([]string, 0, len(r.Lists)+len(r.Songs))
	for _, l := range r.Lists {
		urls = append(urls, l.Cover)
	}
	for _, t := range r.Songs {
		urls = append(urls, t.Cover)
	}
	s.startBrowseArt(r.Kind, r.Generation, urls)
}

func (s *server) recommended(source string) (libraryResult, error) {
	p, ok := s.providers[source].(provider.RecommendedPlaylistProvider)
	if !ok {
		return libraryResult{}, errors.New("平台暂不支持推荐")
	}
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return libraryResult{}, errors.New("正在处理，请稍后")
	}
	cached, hit := s.readLibraryCache(source, "recommend", false)
	lists := cached.Lists
	var e error
	if !hit {
		lists, e = p.GetRecommendedPlaylists()
		if e != nil {
			if old, ok := s.readLibraryCache(source, "recommend", true); ok {
				lists = old.Lists
				e = nil
			}
		}
	}
	if e != nil {
		return libraryResult{}, e
	}
	if len(lists) > 14 {
		lists = lists[:14]
	}
	s.mu.Lock()
	s.listGeneration++
	s.lists = lists
	s.listSource = source
	s.rememberListsLocked()
	gen := s.listGeneration
	s.mu.Unlock()
	r := libraryResult{Kind: "lists", Generation: gen, Page: 1, Title: "歌单推荐", Lists: lists}
	if !hit {
		s.writeLibraryCache(source, "recommend", r)
	}
	return r, nil
}
