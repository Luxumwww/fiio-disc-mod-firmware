package netease

import (
	"encoding/json"
	"errors"
	"github.com/guohuiyuan/music-lib/model"
	"strconv"
)

func positiveMetaInt(n int64) string {
	if n <= 0 {
		return ""
	}
	return strconv.FormatInt(n, 10)
}
func positiveMetaRaw(b json.RawMessage) string {
	var n int64
	if json.Unmarshal(b, &n) == nil {
		return positiveMetaInt(n)
	}
	var s string
	if json.Unmarshal(b, &s) == nil {
		n, _ = strconv.ParseInt(s, 10, 64)
		return positiveMetaInt(n)
	}
	return ""
}

// Metadata only: never resolves/replaces a download URL or changes its quality.
func (n *Netease) GetSongMetadata(song *model.Song) (*model.Song, error) {
	items, e := n.fetchSongsBatch([]string{song.ID})
	if e != nil {
		return nil, e
	}
	if len(items) != 1 || items[0].ID != song.ID {
		return nil, errors.New("metadata song mismatch")
	}
	return &items[0], nil
}
