package main

import (
	"bytes"
	"encoding/binary"
	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/netease"
	"github.com/guohuiyuan/music-lib/qq"
	"github.com/guohuiyuan/music-lib/utils"
	"io"
	"net/http"
	"strings"
	"testing"
)

func vorbisFixture(fields ...string) []byte {
	var b bytes.Buffer
	binary.Write(&b, binary.LittleEndian, uint32(4))
	b.WriteString("TEST")
	binary.Write(&b, binary.LittleEndian, uint32(len(fields)))
	for _, s := range fields {
		binary.Write(&b, binary.LittleEndian, uint32(len(s)))
		b.WriteString(s)
	}
	return b.Bytes()
}
func TestExistingEncoderOnlyFLACReceivesAllKnownTags(t *testing.T) {
	song := model.Song{Name: "歌名", Artist: "艺术家甲、艺术家乙", Album: "专辑", Source: "netease", ID: "3338698698", AlbumID: "999", Duration: 123, Link: "https://music.163.com/#/song?id=3338698698", Extra: map[string]string{"composer": "作曲者", "lyricist": "作词者", "album_artist": "专辑艺术家", "publish_time": "2026-10-13", "track_number": "2", "disc_number": "1", "genre": "电子", "company": "唱片公司", "copyright": "版权", "lyrics": "[00:01.00]歌词", "cookie": "SECRET", "audio_url": "https://cdn/SECRET"}}
	original := vorbisFixture("ENCODER=original", "ARTIST=", "CUSTOM=keep")
	merged, e := mergeVorbisComment(original, song)
	if e != nil {
		t.Fatal(e)
	}
	for _, v := range []string{"TITLE=歌名", "ARTIST=艺术家甲、艺术家乙", "ALBUM=专辑", "ALBUMARTIST=专辑艺术家", "COMPOSER=作曲者", "LYRICIST=作词者", "DATE=2026-10-13", "TRACKNUMBER=2", "DISCNUMBER=1", "GENRE=电子", "LABEL=唱片公司", "LENGTH=123", "SOURCE_ID=3338698698", "LYRICS=[00:01.00]歌词", "ENCODER=original", "CUSTOM=keep"} {
		if !bytes.Contains(merged, []byte(v)) {
			t.Fatal("missing", v)
		}
	}
	if bytes.Contains(merged, []byte("SECRET")) {
		t.Fatal("private Extra leaked")
	}
	again, e := mergeVorbisComment(merged, song)
	if e != nil || !bytes.Equal(again, merged) {
		t.Fatal("tags not idempotent", e)
	}
	preserved, e := mergeVorbisComment(vorbisFixture("ARTIST=original artist", "TITLE=original title"), song)
	if e != nil || bytes.Contains(preserved, []byte("ARTIST=艺术家")) {
		t.Fatal("overwrote valid tag")
	}
	flac, frame := realFLAC()
	flac[4] = 0
	flac = append(flac[:42], append([]byte{132, byte(len(original) >> 16), byte(len(original) >> 8), byte(len(original))}, original...)...)
	flac = append(flac, frame...)
	var out bytes.Buffer
	if e = writeFLACMetadata(&out, bytes.NewReader(flac), song, nil); e != nil {
		t.Fatal(e)
	}
	audio, blocks := flacAudio(out.Bytes())
	if !bytes.Equal(audio, frame) || len(blocks) != 2 || !bytes.Equal(blocks[1].data, merged) {
		t.Fatal("encoder-only file regression")
	}
	for _, bad := range [][]byte{{1}, append(original, 0), {255, 255, 255, 127}} {
		if _, e = mergeVorbisComment(bad, song); e == nil {
			t.Fatal("invalid tags accepted")
		}
	}
}
func TestEmptyID3ArtistFilledWithoutLosingTagsOrAudio(t *testing.T) {
	if decodeID3Text([]byte{0, 233}) != "é" {
		t.Fatal("existing Latin-1 text must remain valid")
	}
	for _, v := range []byte{3, 4} {
		old := append(id3Frame(v, "TPE1", id3Text(v, "")), id3Frame(v, "TIT2", id3Text(v, "original title"))...)
		old = append(old, id3Frame(v, "TXXX", id3Text(v, "CUSTOM\x00keep"))...)
		file := append(append([]byte{'I', 'D', '3', v, 0, 0}, syncSize(len(old))...), old...)
		audio := []byte{255, 251, 77, 22, 11}
		file = append(file, audio...)
		song := model.Song{Name: "new title", Artist: "中文歌手", Album: "专辑", Duration: 12, Source: "qq", ID: "abc", Extra: map[string]string{"date": "2026-10-13", "track_number": "2", "track_total": "10", "lyrics": "歌词正文", "comment": "说明"}}
		var out bytes.Buffer
		if e := writeMP3Metadata(&out, bytes.NewReader(file), song, nil); e != nil {
			t.Fatal(e)
		}
		size, _ := syncSafe(out.Bytes()[6:10])
		if !bytes.Equal(out.Bytes()[10+size:], audio) {
			t.Fatal("changed compressed MP3")
		}
		tag := out.Bytes()[10 : 10+size]
		var artist string
		count := 0
		for pos := 0; pos < len(tag); {
			id := string(tag[pos : pos+4])
			n := int(binary.BigEndian.Uint32(tag[pos+4 : pos+8]))
			if v == 4 {
				n, _ = syncSafe(tag[pos+4 : pos+8])
			}
			data := tag[pos+10 : pos+10+n]
			if id == "TPE1" {
				count++
				artist = decodeID3Text(data)
			}
			if id == "TRCK" && decodeID3Text(data) != "2/10" {
				t.Fatal("track total missing")
			}
			if id == "TIT2" && decodeID3Text(data) != "original title" {
				t.Fatal("title overwritten")
			}
			pos += 10 + n
		}
		if count != 1 || artist != "中文歌手" {
			t.Fatal("empty artist not replaced", count, artist)
		}
		var again bytes.Buffer
		if e := writeMP3Metadata(&again, bytes.NewReader(out.Bytes()), song, nil); e != nil || !bytes.Equal(again.Bytes(), out.Bytes()) {
			t.Fatal("ID3 not idempotent", v, e)
		}
	}
}

type metadataRoundTrip func(*http.Request) (*http.Response, error)

func (f metadataRoundTrip) RoundTrip(r *http.Request) (*http.Response, error) { return f(r) }
func TestProviderMetadataPreservesAlbumDateAndTrackWithoutDownloadResolve(t *testing.T) {
	client := &http.Client{Transport: metadataRoundTrip(func(r *http.Request) (*http.Response, error) {
		var body string
		if strings.Contains(r.URL.Path, "song/detail") {
			body = `{"songs":[{"id":3338698698,"name":"曲名","ar":[{"name":"歌手"}],"al":{"id":777,"name":"专辑","picUrl":"cover"},"dt":123000,"publishTime":1767196800000,"no":2,"cd":"1"}]}`
		} else if strings.Contains(r.URL.Path, "fcg_play_single_song") {
			body = `{"data":[{"id":123,"mid":"abc","name":"曲名","album":{"name":"专辑","mid":"album"},"singer":[{"name":"歌手"}],"interval":123,"time_public":"2026-10-13","index_album":2,"index_cd":1}]}`
		} else {
			t.Fatal("metadata unexpectedly resolves audio", r.URL.Path)
		}
		return &http.Response{StatusCode: 200, Header: http.Header{}, Body: io.NopCloser(strings.NewReader(body))}, nil
	})}
	utils.ConfigureEmbeddedHTTP(client, 2<<20)
	defer utils.ConfigureEmbeddedHTTP(&http.Client{}, 2<<20)
	a, e := netease.New("").GetSongMetadata(&model.Song{ID: "3338698698"})
	if e != nil || a.AlbumID != "777" || a.Extra["track_number"] != "2" || a.Extra["disc_number"] != "1" {
		t.Fatal(a, e)
	}
	b, e := qq.New("").GetSongMetadata(&model.Song{ID: "abc"})
	if e != nil || b.AlbumID != "album" || b.Extra["publish_time"] != "2026-10-13" {
		t.Fatal(b, e)
	}
	if metadataDate("1767196800000", "netease") != "2026-01-01" {
		t.Fatal("date timezone")
	}
}
