package main

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"time"
)

func waitRenderer(ctx context.Context, probe func(context.Context) (string, error)) (string, error) {
	for {
		control, e := probe(ctx)
		if e == nil {
			return control, nil
		}
		if ctx.Err() != nil {
			return "", ctx.Err()
		}
		timer := time.NewTimer(150 * time.Millisecond)
		select {
		case <-ctx.Done():
			timer.Stop()
			return "", ctx.Err()
		case <-timer.C:
		}
	}
}
func discoverControl(ctx context.Context) (string, error) {
	limited, cancel := context.WithTimeout(ctx, 6*time.Second)
	defer cancel()
	return waitRenderer(limited, discoverControlOnce)
}
func (s *server) readyControl(ctx context.Context) (string, error) {
	s.mu.Lock()
	cached := s.rendererControl
	s.mu.Unlock()
	if cached != "" {
		quick, cancel := context.WithTimeout(ctx, 350*time.Millisecond)
		_, e := soapFields(quick, cached, "GetTransportInfo")
		cancel()
		if e == nil {
			return cached, nil
		}
	}
	control, e := discoverControl(ctx)
	if e == nil {
		s.mu.Lock()
		s.rendererControl = control
		s.mu.Unlock()
	}
	return control, e
}

func (s *server) ensureControl(ctx context.Context, control string) (string, error) {
	quick, cancel := context.WithTimeout(ctx, 800*time.Millisecond)
	_, e := soapFields(quick, control, "GetTransportInfo")
	cancel()
	if e == nil {
		return control, nil
	}
	return s.readyControl(ctx)
}

// RAM-only bounded recent stages; no URLs, cookies or periodic flash writes.
func (s *server) recordTiming(stage string, started time.Time, e error) {
	s.timingMu.Lock()
	defer s.timingMu.Unlock()
	path := filepath.Join(s.dir, "timings")
	b, _ := os.ReadFile(path)
	if len(b) > 4096 {
		b = b[len(b)-2048:]
	}
	result := "ok"
	if e != nil {
		result = "error"
	}
	b = append(b, []byte(fmt.Sprintf("%s %dms %s\n", stage, time.Since(started).Milliseconds(), result))...)
	atomicWrite(path, b)
}
