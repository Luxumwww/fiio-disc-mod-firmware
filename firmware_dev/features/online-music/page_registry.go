package main

import (
	"errors"
	"github.com/guohuiyuan/music-lib/model"
	"strconv"
)

type songPage struct {
	Songs  []model.Song
	Source string
	Browse playlistContext
}
type listPage struct {
	Lists  []model.Playlist
	Source string
}

func oldestKey[T any](m map[uint64]T) uint64 {
	var oldest uint64
	for k := range m {
		if oldest == 0 || k < oldest {
			oldest = k
		}
	}
	return oldest
}
func (s *server) rememberSongsLocked() {
	if s.songPages == nil {
		s.songPages = make(map[uint64]songPage)
	}
	s.songPages[s.generation] = songPage{append([]model.Song(nil), s.songs...), s.source, s.browse}
	if len(s.songPages) > 512 {
		delete(s.songPages, oldestKey(s.songPages))
	}
}
func (s *server) rememberListsLocked() {
	if s.listPages == nil {
		s.listPages = make(map[uint64]listPage)
	}
	s.listPages[s.listGeneration] = listPage{append([]model.Playlist(nil), s.lists...), s.listSource}
	if len(s.listPages) > 128 {
		delete(s.listPages, oldestKey(s.listPages))
	}
}
func (s *server) selectedPageLocked(gen uint64) (songPage, bool) {
	if p, ok := s.songPages[gen]; ok {
		return p, true
	}
	if gen == s.generation {
		return songPage{s.songs, s.source, s.browse}, true
	}
	return songPage{}, false
}
func (s *server) playlistChunk(source, id string, page int) ([]model.Song, int, error) {
	key := "tracks:" + id + ":" + strconv.Itoa(page)
	if r, ok := s.readLibraryCache(source, key, false); ok {
		return r.Songs, r.Total, nil
	}
	p, ok := s.providers[source].(pagedSongs)
	if !ok {
		return nil, 0, errors.New("平台尚不支持分段歌曲")
	}
	songs, total, e := p.GetPlaylistSongsPage(id, page, trackPageSize)
	if e != nil {
		if r, ok := s.readLibraryCache(source, key, true); ok {
			return r.Songs, r.Total, nil
		}
		return nil, 0, e
	}
	if len(songs) > trackPageSize {
		return nil, 0, errors.New("平台未按段返回歌曲")
	}
	s.writeLibraryCache(source, key, libraryResult{Songs: songs, Total: total})
	return songs, total, nil
}

// Called while the provider gate is held when credentials change. Existing
// playback is independent, but old browse/download tokens must not cross accounts.
func (s *server) invalidateAccountPages(source string) {
	s.mu.Lock()
	defer s.mu.Unlock()
	for gen, p := range s.songPages {
		if p.Source == source {
			delete(s.songPages, gen)
		}
	}
	for gen, p := range s.listPages {
		if p.Source == source {
			delete(s.listPages, gen)
		}
	}
	if s.source == source {
		s.songs = nil
		s.browse = playlistContext{}
		s.generation++
	}
	if s.listSource == source {
		s.lists = nil
		s.listGeneration++
	}
}
