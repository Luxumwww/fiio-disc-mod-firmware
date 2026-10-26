package main

import (
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

func (f *fiio) local(tag, payload, expected string) (string, error) {
	seq := uint32(time.Now().UnixMilli()%2000000000) + 1
	frame := fmt.Sprintf("%s%04X%s", tag, len(payload)+8, payload)
	path := "/tmp/disc-runtime/remote-eq.request"
	if e := atomicFile(path, []byte(fmt.Sprintf("%d\n%s", seq, frame))); e != nil {
		return "", e
	}
	end := time.Now().Add(2 * time.Second)
	acked := false
	for time.Now().Before(end) {
		if !acked {
			b, _ := os.ReadFile("/tmp/disc-runtime/remote-eq.ack")
			parts := strings.SplitN(string(b), "\n", 3)
			if len(parts) >= 2 && parts[0] == strconv.FormatUint(uint64(seq), 10) {
				if parts[1] != "0" {
					return "", fmt.Errorf("native EQ queue unavailable")
				}
				acked = true
				if expected == "" {
					return "", nil
				}
			}
		}
		if acked {
			b, _ := os.ReadFile("/tmp/disc-runtime/remote-eq.reply")
			parts := strings.SplitN(string(b), "\n", 2)
			if len(parts) == 2 && parts[0] == strconv.FormatUint(uint64(seq), 10) {
				return parts[1], nil
			}
		}
		time.Sleep(20 * time.Millisecond)
	}
	return "", fmt.Errorf("native EQ reply timeout")
}
