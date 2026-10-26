package main

import (
	"encoding/binary"
	"errors"
	"github.com/guohuiyuan/music-lib/model"
	"strconv"
	"strings"
	"time"
	"unicode/utf16"
)

type songTag struct{ key, value string }

func mergeSongMetadata(base, detail model.Song) model.Song {
	result := cloneSong(base)
	if result.Name == "" {
		result.Name = detail.Name
	}
	if result.Artist == "" {
		result.Artist = detail.Artist
	}
	if result.Album == "" {
		result.Album = detail.Album
	}
	if result.AlbumID == "" || result.AlbumID == "0" {
		result.AlbumID = detail.AlbumID
	}
	if result.Cover == "" {
		result.Cover = detail.Cover
	}
	if result.Duration <= 0 {
		result.Duration = detail.Duration
	}
	if result.Link == "" {
		result.Link = detail.Link
	}
	for key, value := range detail.Extra {
		if result.Extra[key] == "" && value != "" {
			result.Extra[key] = value
		}
	}
	return result
}
func tagText(v string) string {
	v = strings.ToValidUTF8(strings.ReplaceAll(v, "\x00", ""), "")
	if len(v) > 512<<10 {
		return ""
	}
	return strings.TrimSpace(v)
}
func knownSongTags(song model.Song) []songTag {
	var tags []songTag
	add := func(key, value string) {
		if value = tagText(value); value != "" {
			tags = append(tags, songTag{key, value})
		}
	}
	extra := func(keys ...string) string {
		for _, key := range keys {
			if v := tagText(song.Extra[key]); v != "" {
				return v
			}
		}
		return ""
	}
	add("TITLE", song.Name)
	add("ARTIST", song.Artist)
	add("ALBUM", song.Album)
	add("ALBUMARTIST", extra("album_artist", "albumartist"))
	add("COMPOSER", extra("composer"))
	add("LYRICIST", extra("lyricist"))
	add("CONDUCTOR", extra("conductor"))
	add("GENRE", extra("genre"))
	add("COPYRIGHT", extra("copyright"))
	add("LABEL", extra("label", "company"))
	add("ISRC", extra("isrc"))
	add("LANGUAGE", extra("language"))
	add("COMMENT", extra("comment", "description"))
	add("SUBTITLE", extra("subtitle"))
	add("DISC_SUBTITLE", extra("disc_subtitle"))
	for _, entry := range []struct {
		tag  string
		keys []string
	}{{"TRACKNUMBER", []string{"track_number", "tracknumber"}}, {"DISCNUMBER", []string{"disc_number", "discnumber"}}, {"TRACKTOTAL", []string{"track_total", "total_tracks"}}, {"DISCTOTAL", []string{"disc_total", "total_discs"}}, {"BPM", []string{"bpm"}}} {
		if v := extra(entry.keys...); v != "" {
			if n, e := strconv.Atoi(v); e == nil && n > 0 && n < 100000 {
				add(entry.tag, strconv.Itoa(n))
			}
		}
	}
	if date := metadataDate(extra("date", "release_date", "publish_time", "year"), song.Source); date != "" {
		add("DATE", date)
	}
	if song.Duration > 0 && song.Duration < 86400 {
		add("LENGTH", strconv.Itoa(song.Duration))
		add("DURATION_MS", strconv.Itoa(song.Duration*1000))
	}
	add("SOURCE_PLATFORM", song.Source)
	add("SOURCE_ID", song.ID)
	if song.AlbumID != "0" {
		add("SOURCE_ALBUM_ID", song.AlbumID)
	}
	add("SOURCE_URL", publicSongLink(song.Link))
	add("LYRICS", extra("lyrics"))
	return tags
}
func publicSongLink(raw string) string {
	// Public canonical song pages only; never persist temporary audio links or
	// arbitrary provider Extra (credentials, signed CDN URLs, hashes for tiers).
	for _, prefix := range []string{"https://music.163.com/#/song?id=", "https://y.qq.com/n/ryqq/songDetail/", "https://www.kugou.com/song/#hash="} {
		if strings.HasPrefix(raw, prefix) && !strings.ContainsAny(strings.TrimPrefix(raw, prefix), "&?\r\n") {
			return raw
		}
	}
	return ""
}
func metadataDate(v, source string) string {
	if v == "" {
		return ""
	}
	for _, format := range []string{"2006-01-02", "2006/01/02", "2006-01-02 15:04:05", time.RFC3339} {
		if t, e := time.Parse(format, v); e == nil {
			return t.Format("2006-01-02")
		}
	}
	if len(v) == 4 {
		if n, e := strconv.Atoi(v); e == nil && n >= 1000 && n <= 9999 {
			return v
		}
	}
	if len(v) == 13 {
		if n, e := strconv.ParseInt(v, 10, 64); e == nil && n > 0 {
			zone := time.UTC
			if source == "netease" {
				zone = time.FixedZone("CST", 8*3600)
			}
			return time.UnixMilli(n).In(zone).Format("2006-01-02")
		}
	}
	return ""
}
func mergeVorbisComment(existing []byte, song model.Song) ([]byte, error) {
	vendor := "DISC"
	var fields []string
	if existing != nil {
		pos := 0
		readString := func() (string, error) {
			if len(existing)-pos < 4 {
				return "", errors.New("short Vorbis length")
			}
			n := uint64(binary.LittleEndian.Uint32(existing[pos : pos+4]))
			pos += 4
			if n > uint64(len(existing)-pos) {
				return "", errors.New("invalid Vorbis length")
			}
			v := string(existing[pos : pos+int(n)])
			pos += int(n)
			return v, nil
		}
		var e error
		vendor, e = readString()
		if e != nil {
			return nil, e
		}
		if len(existing)-pos < 4 {
			return nil, errors.New("short Vorbis count")
		}
		count := binary.LittleEndian.Uint32(existing[pos : pos+4])
		pos += 4
		if count > 65536 {
			return nil, errors.New("Vorbis count limit")
		}
		for i := uint32(0); i < count; i++ {
			v, e := readString()
			if e != nil {
				return nil, e
			}
			fields = append(fields, v)
		}
		if pos != len(existing) {
			return nil, errors.New("trailing Vorbis bytes")
		}
	}
	for _, tag := range knownSongTags(song) {
		found := false
		for _, field := range fields {
			key, value, ok := strings.Cut(field, "=")
			if ok && strings.EqualFold(key, tag.key) && tagText(value) != "" {
				found = true
				break
			}
		}
		kept := fields[:0]
		for _, field := range fields {
			key, value, ok := strings.Cut(field, "=")
			if !ok || !strings.EqualFold(key, tag.key) || tagText(value) != "" {
				kept = append(kept, field)
			}
		}
		fields = kept
		if !found {
			fields = append(fields, tag.key+"="+tag.value)
		}
	}
	var b []byte
	put := func(v uint32) { var tmp [4]byte; binary.LittleEndian.PutUint32(tmp[:], v); b = append(b, tmp[:]...) }
	put(uint32(len(vendor)))
	b = append(b, vendor...)
	put(uint32(len(fields)))
	for _, f := range fields {
		put(uint32(len(f)))
		b = append(b, f...)
	}
	if len(b) > 8<<20 {
		return nil, errors.New("Vorbis tags too large")
	}
	return b, nil
}
func id3Text(version byte, v string) []byte {
	if version == 4 {
		return append([]byte{3}, []byte(v)...)
	}
	b := []byte{1, 255, 254}
	for _, u := range utf16.Encode([]rune(v)) {
		b = append(b, byte(u), byte(u>>8))
	}
	return b
}
func decodeID3Text(b []byte) string {
	if len(b) < 1 {
		return ""
	}
	encoding := b[0]
	b = b[1:]
	if encoding == 0 {
		var runes []rune
		for _, v := range b {
			runes = append(runes, rune(v))
		}
		return tagText(strings.Trim(string(runes), "\x00"))
	}
	if encoding == 3 {
		return tagText(strings.Trim(string(b), "\x00"))
	}
	if encoding != 1 && encoding != 2 {
		return ""
	}
	little := false
	if len(b) >= 2 && b[0] == 255 && b[1] == 254 {
		little = true
		b = b[2:]
	} else if len(b) >= 2 && b[0] == 254 && b[1] == 255 {
		b = b[2:]
	}
	if len(b)%2 != 0 {
		return ""
	}
	var u []uint16
	for len(b) >= 2 {
		if little {
			u = append(u, binary.LittleEndian.Uint16(b[:2]))
		} else {
			u = append(u, binary.BigEndian.Uint16(b[:2]))
		}
		b = b[2:]
	}
	return tagText(strings.Trim(string(utf16.Decode(u)), "\x00"))
}
func id3TagFrame(version byte, t songTag) (string, []byte) {
	ids := map[string]string{"TITLE": "TIT2", "ARTIST": "TPE1", "ALBUM": "TALB", "ALBUMARTIST": "TPE2", "COMPOSER": "TCOM", "LYRICIST": "TEXT", "CONDUCTOR": "TPE3", "GENRE": "TCON", "COPYRIGHT": "TCOP", "LABEL": "TPUB", "ISRC": "TSRC", "LANGUAGE": "TLAN", "SUBTITLE": "TIT3", "BPM": "TBPM", "DURATION_MS": "TLEN", "TRACKNUMBER": "TRCK", "DISCNUMBER": "TPOS"}
	if id := ids[t.key]; id != "" {
		return id, id3Text(version, t.value)
	}
	if t.key == "DATE" && version == 4 {
		return "TDRC", id3Text(version, t.value)
	}
	if t.key == "DATE" && version == 3 {
		return "TYER", id3Text(version, t.value[:4])
	}
	if t.key == "LYRICS" {
		text := id3Text(version, t.value)
		data := []byte{text[0], 'u', 'n', 'd'}
		if text[0] == 1 {
			data = append(data, 255, 254, 0, 0)
		} else {
			data = append(data, 0)
		}
		return "USLT", append(data, text[1:]...)
	}
	if t.key == "COMMENT" {
		text := id3Text(version, t.value)
		data := []byte{text[0], 'u', 'n', 'd'}
		if text[0] == 1 {
			data = append(data, 255, 254, 0, 0)
		} else {
			data = append(data, 0)
		}
		return "COMM", append(data, text[1:]...)
	}
	if t.key == "LENGTH" {
		return "", nil
	}
	data := id3Text(version, t.key)
	if data[0] == 1 {
		data = append(data, 0, 0)
		value := id3Text(version, t.value)
		data = append(data, value[1:]...)
	} else {
		data = append(data, 0)
		data = append(data, []byte(t.value)...)
	}
	return "TXXX", data
}
func splitID3Description(data []byte) (string, []byte) {
	if len(data) < 1 {
		return "", nil
	}
	enc := data[0]
	width := 1
	if enc == 1 || enc == 2 {
		width = 2
	}
	for pos := 1; pos+width <= len(data); pos += width {
		if data[pos] == 0 && (width == 1 || data[pos+1] == 0) {
			desc := decodeID3Text(data[:pos])
			rest := append([]byte{enc}, data[pos+width:]...)
			return desc, rest
		}
	}
	return "", nil
}
