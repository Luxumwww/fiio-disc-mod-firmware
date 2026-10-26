package kugou

import (
	"github.com/guohuiyuan/music-lib/model"
)

func (k *Kugou) GetSongMetadata(song *model.Song) (*model.Song, error) {
	return k.fetchSongInfo(song.ID)
}
