package qq

import (
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"strings"
)

func (q *QQ) GetPlaylistSongsPage(id string, page, limit int) ([]model.Song, int, error) {
	if id == qqFavoriteSongsPlaylistID {
		uin := normalizeQQUIN(q.cookie)
		if uin == "" {
			return nil, 0, fmt.Errorf("请先登录")
		}
		total, s, e := q.fetchProfileOrderSongs(uin, page, limit)
		return s, total, e
	}
	if strings.HasPrefix(id, qqProfileDirPlaylistPrefix) {
		s, e := q.fetchProfileDirSongsPage(strings.TrimPrefix(id, qqProfileDirPlaylistPrefix), page, limit)
		return s, -1, e
	}
	p, s, e := q.fetchPlaylistPage(id, page, limit)
	if e != nil {
		return nil, 0, e
	}
	return s, p.TrackCount, nil
}
