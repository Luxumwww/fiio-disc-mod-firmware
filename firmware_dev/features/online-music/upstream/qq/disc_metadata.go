package qq

import (
	"errors"
	"github.com/guohuiyuan/music-lib/model"
	"strconv"
)

func positiveMetadataNumber(n int) string {
	if n <= 0 {
		return ""
	}
	return strconv.Itoa(n)
}
func (q *QQ) GetSongMetadata(song *model.Song) (*model.Song, error) {
	mid := song.Extra["songmid"]
	if mid == "" {
		mid = song.ID
	}
	s, e := q.fetchSongDetail(mid)
	if e != nil {
		return nil, e
	}
	if s.ID != mid {
		return nil, errors.New("metadata song mismatch")
	}
	return s, nil
}
