package main

import (
	"context"
	"errors"
	"fmt"
	"github.com/guohuiyuan/music-lib/model"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

func TestActualX2000PCMClockFormatAndPause(t *testing.T) {
	status := "state: RUNNING\nowner_pid   : 1682\ntrigger_time: 1791744176.909162695\nhw_ptr      : 7132832\nappl_ptr    : 7136672\n"
	pcm := parsePCMObservation(status, "rate: 48000 (48000/1)\nformat: S32_LE\n")
	if !pcm.Valid || pcm.Frames != 7132832 || pcm.Rate != 48000 || pcm.Trigger != "1791744176.909162695" {
		t.Fatal(pcm)
	}
	p := playbackState{}
	updatePCMClock(&p, pcm, "PLAYING")
	if int(p.PCMSeconds) != 148 || !p.SeenPCM {
		t.Fatal(p)
	}
	updatePCMClock(&p, pcm, "PAUSED_PLAYBACK")
	if int(p.PCMSeconds) != 148 {
		t.Fatal("pause advanced clock")
	}
	pcm.Frames += 96000
	updatePCMClock(&p, pcm, "PLAYING")
	if int(p.PCMSeconds) != 150 {
		t.Fatal("resume did not follow consumed frames", p.PCMSeconds)
	}
	if !parsePCMObservation("closed\n", "closed\n").Closed {
		t.Fatal("closed stream missed")
	}
	if parsePCMObservation("state: RUNNING\nhw_ptr : bad\n", "rate: 44100").Valid {
		t.Fatal("invalid counter accepted")
	}
}

func TestZeroNativePositionUsesPCMAndClosedStreamAdvances(t *testing.T) {
	audio := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { w.Write([]byte("ID3" + strings.Repeat("a", 61))) }))
	defer audio.Close()
	state := "PLAYING"
	renderer := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "<Envelope><CurrentTransportState>%s</CurrentTransportState><TrackURI></TrackURI><RelTime>0:00:00</RelTime><TrackDuration>0:00:00</TrackDuration></Envelope>", state)
	}))
	defer renderer.Close()
	s := newServer(t.TempDir(), 1)
	s.quality = "standard"
	s.providers["mock"] = &rendererProvider{url: audio.URL}
	pcm := parsePCMObservation("state: RUNNING\ntrigger_time: 100\nhw_ptr : 441000\n", "rate: 44100")
	s.pcmRead = func() pcmObservation { return pcm }
	if e := s.startTrack([]model.Song{{ID: "a", Duration: 30}, {ID: "b", Duration: 30}}, "mock", playlistContext{}, 0, renderer.URL, 0); e != nil {
		t.Fatal(e)
	}
	poll := func() { s.lastTransportPoll = time.Time{}; s.pollPlayback() }
	poll()
	if s.player.Position != 10 {
		t.Fatal("native zero suppressed PCM clock", s.player.Position)
	}
	pcm = pcmObservation{Closed: true}
	state = "PAUSED_PLAYBACK"
	poll()
	poll()
	if s.player.Index != 0 {
		t.Fatal("paused stream advanced")
	}
	state = "PLAYING"
	poll()
	if s.player.Index != 0 {
		t.Fatal("one closed observation advanced")
	}
	poll()
	if s.player.Index != 1 || s.player.Position != 0 {
		t.Fatal("closed PCM did not advance despite stale PLAYING", s.player)
	}
	b, _ := os.ReadFile(filepath.Join(s.dir, "nowplaying"))
	if !strings.Contains(string(b), "\tPLAYING\torder\t0\t30\t") {
		t.Fatal(string(b))
	}
}

func TestReceiverReadinessRetriesAndCancellation(t *testing.T) {
	attempts := 0
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	control, e := waitRenderer(ctx, func(context.Context) (string, error) {
		attempts++
		if attempts < 3 {
			return "", errors.New("not yet")
		}
		return "ready", nil
	})
	if e != nil || control != "ready" || attempts != 3 {
		t.Fatal(control, e, attempts)
	}
	stopped, stop := context.WithCancel(context.Background())
	stop()
	if _, e := waitRenderer(stopped, func(context.Context) (string, error) { return "", errors.New("not yet") }); !errors.Is(e, context.Canceled) {
		t.Fatal(e)
	}
}

func TestStaleRendererEndpointRefresh(t *testing.T) {
	old := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { w.Write([]byte("<Envelope/>")) }))
	stale := old.URL
	old.Close()
	ready := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("<Envelope><CurrentTransportState>PLAYING</CurrentTransportState></Envelope>"))
	}))
	defer ready.Close()
	s := newServer(t.TempDir(), 1)
	s.rendererControl = ready.URL
	control, e := s.ensureControl(context.Background(), stale)
	if e != nil || control != ready.URL {
		t.Fatal("stale retained endpoint was not replaced", control, e)
	}
}
