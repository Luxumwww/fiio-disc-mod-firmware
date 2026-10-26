package kugou

import (
	"errors"
	"github.com/guohuiyuan/music-lib/model"
	"strings"
)

func GetDownloadURL(s *model.Song) (string, error) { return defaultKugou.GetDownloadURL(s) }

func GetDownloadURLBySonginfo(s *model.Song) (string, error) {
	return defaultKugou.GetDownloadURLBySonginfo(s)
}

// GetDownloadURL 获取下载链接
func (k *Kugou) GetDownloadURL(s *model.Song) (string, error) {
	if s.Source != "kugou" {
		return "", errors.New("source mismatch")
	}
	q := s.Extra["disc_quality"]
	if q != "" && q != "auto" {
		key := "file_hash"
		if q == "exhigh" {
			key = "hq_hash"
		}
		if q == "lossless" {
			key = "sq_hash"
		}
		if q == "hires" {
			key = "res_hash"
		}
		hash := s.Extra[key]
		if hash == "" {
			if q == "standard" {
				hash = s.ID
			} else {
				return "", errors.New("该歌曲没有所选音质信息，请选择自动")
			}
		}
		info, e := k.fetchTrackerSongInfo(hash)
		if e != nil || info == nil || info.URL == "" {
			info, e = k.fetchSongInfo(hash)
		}
		if e != nil {
			return "", e
		}
		if info == nil || info.URL == "" {
			return "", errors.New("所选音质暂不可用")
		}
		s.Ext = info.Ext
		s.Bitrate = info.Bitrate
		return info.URL, nil
	}
	if s.URL != "" {
		return s.URL, nil
	}

	hash := s.ID
	if s.Extra != nil && s.Extra["hash"] != "" {
		hash = s.Extra["hash"]
	}

	privilege := getKugouPrivilege(s)

	if shouldTryKugouHighQualityDownload(k.cookie, privilege) {
		if info, err := k.fetchVIPSongInfo(s); err == nil && info != nil && info.URL != "" {
			s.Ext = info.Ext
			s.Bitrate = info.Bitrate
			return info.URL, nil
		}
	}

	isVip, vipErr := k.IsVipAccount()
	if vipErr == nil && isVip {
		if info, err := k.fetchTrackerSongInfo(hash); err == nil && info != nil && info.URL != "" {
			s.Ext = info.Ext
			s.Bitrate = info.Bitrate
			return info.URL, nil
		}
	}

	info, err := k.fetchSongInfo(hash)
	if err != nil {
		return "", err
	}
	s.Ext = info.Ext
	s.Bitrate = info.Bitrate
	return info.URL, nil
}

func (k *Kugou) GetDownloadURLBySonginfo(s *model.Song) (string, error) {
	var lastErr error
	for _, hash := range collectCandidateHashes(s) {
		info, err := k.fetchSonginfoV2(hash)
		if err != nil {
			lastErr = err
			continue
		}
		if info != nil && strings.TrimSpace(info.URL) != "" {
			s.Ext = info.Ext
			s.Bitrate = info.Bitrate
			return info.URL, nil
		}
		lastErr = errors.New("kugou songinfo v2 download url not found")
	}
	if lastErr != nil {
		return "", lastErr
	}
	return "", errors.New("kugou songinfo v2 download url not found")
}

func shouldTryKugouHighQualityDownload(cookie string, privilege int) bool {
	if privilege == 10 || privilege == 8 {
		return true
	}
	return kugouHasAppCookie(parseKugouCookie(cookie))
}
