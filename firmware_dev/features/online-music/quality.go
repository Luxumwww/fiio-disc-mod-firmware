package main

import (
	"context"
	"errors"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"time"
)

// Preference is a ceiling; auto starts at the highest ordinary stereo quality.
// Providers may downgrade a request themselves, so verify the actual audio too.
func qualityCandidates(q string) []string {
	levels := []string{"hires", "lossless", "exhigh", "standard"}
	if q == "auto" {
		return levels
	}
	for i, level := range levels {
		if level == q {
			return levels[i:]
		}
	}
	return []string{"standard"}
}
func (s *server) resolveAudio(ctx context.Context, source string, song *model.Song, preference string) (string, string, int, int, error) {
	var last error
	original := cloneSong(*song)
	for _, q := range qualityCandidates(preference) {
		if e := ctx.Err(); e != nil {
			return "", "", 0, 0, e
		}
		candidate := cloneSong(original)
		started := time.Now()
		u, e := s.songURLQuality(source, &candidate, q)
		s.recordTiming("source_"+q, started, e)
		if e != nil {
			last = e
			continue
		}
		started = time.Now()
		ext, rate, bits, e := s.probeAudio(ctx, u)
		s.recordTiming("header_"+q, started, e)
		if e == nil {
			e = checkQuality(q, ext, rate, bits)
		}
		if e != nil {
			last = e
			continue
		}
		candidate.Ext = ext
		*song = candidate
		return u, ext, rate, bits, nil
	}
	if last == nil {
		last = errors.New("没有可播放音源")
	}
	return "", "", 0, 0, fmt.Errorf("各档音质均不可用：%w", last)
}
