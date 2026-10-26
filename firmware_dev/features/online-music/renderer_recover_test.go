package main

import (
	"context"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

func TestRendererDroppedPlayRecoveryDoesNotRestartOrSteal(t *testing.T) {
	for _, mode := range []string{"playing", "stopped", "foreign"} {
		t.Run(mode, func(t *testing.T) {
			plays := 0
			uri := "http://127.0.0.1/audio/1.flac"
			state := "STOPPED"
			if mode == "playing" {
				state = "PLAYING"
			}
			web := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
				action := r.Header.Get("SOAPACTION")
				if strings.Contains(action, "#Play") {
					plays++
					w.Write([]byte("ok"))
					return
				}
				if strings.Contains(action, "GetMediaInfo") {
					u := uri
					if mode == "foreign" {
						u = "other"
					}
					fmt.Fprintf(w, "<CurrentURI>%s</CurrentURI>", u)
				} else {
					fmt.Fprintf(w, "<CurrentTransportState>%s</CurrentTransportState>", state)
				}
			}))
			defer web.Close()
			s := newServer(t.TempDir(), 1)
			s.rendererControl = web.URL
			_, e := s.recoverRendererPlay(context.Background(), web.URL, uri)
			if mode == "foreign" {
				if e == nil || plays != 0 {
					t.Fatal("foreign playback overwritten")
				}
			} else if e != nil || plays != map[string]int{"playing": 0, "stopped": 1}[mode] {
				t.Fatal("retry/restart", plays, e)
			}
		})
	}
	if !rendererConnectionDropped(io.EOF) || rendererConnectionDropped(context.DeadlineExceeded) {
		t.Fatal("timeout retry policy")
	}
}
