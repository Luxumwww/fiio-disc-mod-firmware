package main

import (
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"
)

func TestConcurrentPlaybackPublicationAndExit(t *testing.T) {
	s := newServer(t.TempDir(), 1)
	s.player = playbackState{Revision: 9, URI: "test", State: "PLAYING", Mode: "order", Format: "FLAC"}
	var writers sync.WaitGroup
	for n := 0; n < 8; n++ {
		writers.Add(1)
		go func() {
			defer writers.Done()
			for i := 0; i < 100; i++ {
				s.publishPlayback()
			}
		}()
	}
	writers.Wait()
	b, e := os.ReadFile(filepath.Join(s.dir, "nowplaying"))
	if e != nil || !strings.HasPrefix(string(b), "9\tPLAYING\torder\t") || len(strings.Split(strings.TrimSuffix(string(b), "\n"), "\n")) != 6 {
		t.Fatalf("incomplete publication: %q, %v", b, e)
	}
	// exitPlayback's locked state transition and file removal cannot be
	// undone by later timer/download publications from an idle player.
	s.mu.Lock()
	s.player = playbackState{Revision: 10, State: "STOPPED"}
	os.Remove(filepath.Join(s.dir, "nowplaying"))
	s.mu.Unlock()
	s.publishPlayback()
	if _, e := os.Stat(filepath.Join(s.dir, "nowplaying")); !os.IsNotExist(e) {
		t.Fatalf("idle publish recreated exited playback: %v", e)
	}
}
