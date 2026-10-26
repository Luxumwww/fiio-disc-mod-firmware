package kugou

import "github.com/guohuiyuan/music-lib/model"

func (k *Kugou) GetPlaylistSongsPage(id string, page, limit int) ([]model.Song, int, error) {
	var p *model.Playlist
	var s []model.Song
	var e error
	if listID, ok := parseKugouCloudlistID(id); ok {
		p, s, e = k.fetchCloudlistPage(listID, page, limit)
	} else {
		p, s, e = k.fetchPlaylistPage(id, page, limit)
	}
	if e != nil {
		return nil, 0, e
	}
	total := p.TrackCount
	if total == 0 {
		total = -1
	}
	return s, total, nil
}
