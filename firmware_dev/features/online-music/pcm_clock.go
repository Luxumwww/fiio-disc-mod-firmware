package main

import (
	"os"
	"strconv"
	"strings"
)

type pcmObservation struct {
	Frames         uint64
	Rate           int
	State, Trigger string
	Valid, Closed  bool
}

func parsePCMObservation(status, params string) pcmObservation {
	p := pcmObservation{Closed: strings.TrimSpace(status) == "closed"}
	fields := func(line string) []string {
		// Actual X2000 ALSA prints "hw_ptr      : 7132832", not two fields.
		return strings.Fields(strings.ReplaceAll(line, ":", " "))
	}
	found := false
	for _, line := range strings.Split(status, "\n") {
		f := fields(line)
		if len(f) < 2 {
			continue
		}
		switch f[0] {
		case "hw_ptr":
			var e error
			p.Frames, e = strconv.ParseUint(f[1], 10, 64)
			found = e == nil
		case "state":
			p.State = f[1]
		case "trigger_time":
			p.Trigger = f[1]
		}
	}
	for _, line := range strings.Split(params, "\n") {
		f := fields(line)
		if len(f) > 1 && f[0] == "rate" {
			p.Rate, _ = strconv.Atoi(f[1])
		}
	}
	p.Valid = found && p.Rate >= 8000 && p.Rate <= 768000
	return p
}
func nativePCMObservation() pcmObservation {
	status, e := os.ReadFile("/proc/asound/card0/pcm3p/sub0/status")
	if e != nil {
		return pcmObservation{}
	}
	params, _ := os.ReadFile("/proc/asound/card0/pcm3p/sub0/hw_params")
	return parsePCMObservation(string(status), string(params))
}
func (s *server) observePCM() pcmObservation {
	if s.pcmRead != nil {
		return s.pcmRead()
	}
	return nativePCMObservation()
}
func updatePCMClock(p *playbackState, observation pcmObservation, state string) {
	if !observation.Valid {
		return
	}
	if observation.State == "RUNNING" {
		p.SeenPCM = true
	}
	if observation.Trigger != "" && observation.Trigger != p.PCMTrigger {
		p.PCMTrigger = observation.Trigger
		p.PCMSeconds = float64(observation.Frames) / float64(observation.Rate)
	} else if observation.Frames >= p.PCMFrames && state == "PLAYING" {
		p.PCMSeconds += float64(observation.Frames-p.PCMFrames) / float64(observation.Rate)
	}
	p.PCMFrames = observation.Frames
}
