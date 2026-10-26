package netease

import "github.com/guohuiyuan/music-lib/model"

func (n *Netease) GetPlaylistSongsPage(id string, page, limit int) ([]model.Song, int, error) {
	p, s, e := n.fetchPlaylistPage(id, page, limit)
	if e != nil {
		return nil, 0, e
	}
	return s, p.TrackCount, nil
}
