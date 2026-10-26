package main

import (
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"os"
	"path/filepath"
	"testing"
)

type mockLibrary struct {
	mockProvider
	trackCalls          int
	lastPage, lastLimit int
}

func (p *mockLibrary) GetUserPlaylists(page, limit int) ([]model.Playlist, error) {
	p.lastPage = page
	p.lastLimit = limit
	var a []model.Playlist
	for i := 0; i < limit; i++ {
		a = append(a, model.Playlist{ID: fmt.Sprint(i), Name: "测试歌单", TrackCount: 45})
	}
	a[0].Name = "我喜欢的音乐"
	return a, nil
}
func (p *mockLibrary) GetPlaylistSongsPage(id string, page, limit int) ([]model.Song, int, error) {
	p.trackCalls++
	p.lastPage = page
	p.lastLimit = limit
	var a []model.Song
	for i := (page - 1) * limit; i < page*limit && i < 45; i++ {
		a = append(a, model.Song{ID: fmt.Sprint(i)})
	}
	return a, 45, nil
}
func TestLibraryPagination(t *testing.T) {
	s := newServer(t.TempDir(), 49381)
	p := &mockLibrary{}
	s.providers["mock"] = p
	r, e := s.library("mock", 1, false)
	if e != nil || !r.More || p.lastLimit != 14 {
		t.Fatal(r, e)
	}
	if p.trackCalls != 0 {
		t.Fatal("eager songs fetch")
	}
	first, e := s.libraryTracks(r.Generation, 0, 1)
	if e != nil || len(first.Songs) != 20 || !first.More {
		t.Fatal(first, e)
	}
	second, e := s.libraryTracks(r.Generation, 0, 2)
	if e != nil || second.Songs[0].ID == first.Songs[0].ID {
		t.Fatal("page repeated", e)
	}
	third, e := s.libraryTracks(r.Generation, 0, 3)
	if e != nil || len(third.Songs) != 5 || third.More {
		t.Fatal(third, e)
	}
	s.library("mock", 2, false)
	if _, e = s.libraryTracks(r.Generation, 0, 1); e != nil {
		t.Fatal("earlier visible segment lost its valid token", e)
	}
	fav, e := s.library("mock", 1, true)
	if e != nil || len(fav.Lists) != 1 || fav.More {
		t.Fatal(fav, e)
	}
	if _, e = s.library("mock", 0, false); e == nil {
		t.Fatal("invalid page accepted")
	}
	direct, e := s.favoriteTracks("mock", 1)
	if e != nil || direct.Kind != "tracks" || len(direct.Songs) != 20 || direct.ListGeneration == 0 || !direct.More {
		t.Fatal("favorites did not enter tracks directly", direct, e)
	}
}
func TestLoginPersistenceAndExplicitLogout(t *testing.T) {
	dir := t.TempDir()
	os.WriteFile(filepath.Join(dir, "netease.cookie"), []byte("test-cookie=example"), 0600)
	s := newServer(t.TempDir(), 49381)
	s.loadCredentials(dir)
	if s.accountMessage("netease") != "已保存登录" {
		t.Fatal("saved account not restored")
	}
	if e := s.logout("netease"); e != nil {
		t.Fatal(e)
	}
	if _, e := os.Stat(filepath.Join(dir, "netease.cookie")); !os.IsNotExist(e) {
		t.Fatal("explicit logout retained cookie")
	}
}
