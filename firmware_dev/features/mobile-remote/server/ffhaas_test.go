package main

import (
	"bytes"
	"os"
	"path/filepath"
	"testing"
)

func TestFFmpegHaasSharedNativeFormat(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	e.audioPath = filepath.Join(e.dir, "audio")
	os.WriteFile(e.audioPath, []byte("shared_supported=1\nwork_mode=1\n"), 0600)
	if len(e.schema.Effects) != 11 {
		t.Fatal("legacy fixed matrix changed")
	}
	f, ok := e.module(14)
	if !ok || len(f.Params) != 12 {
		t.Fatal("FFmpeg controls missing")
	}
	run := func(q Request) {
		q.ID = 1
		q.Global = true
		q.Mode = 3
		c, _, _, _ := e.sharedProfile(3)
		q.Revision = c.Revision
		if r := e.handle(q); r["ok"] != true {
			t.Fatal(r)
		}
	}
	run(Request{Op: "chain_add", Effect: 14, Enabled: false})
	c, on, _, _ := e.sharedProfile(3)
	if on || c.Nodes[0].Enabled || c.Nodes[0].Params != [12]float64{1, 1, 1, 2, 2.05, -1, 1, 0, 2.12, 1, 1, 1} {
		t.Fatal("upstream defaults/off", c)
	}
	for _, key := range []string{"middle_source", "left_phase", "right_phase"} {
		before, _ := os.ReadFile(e.sharedPath())
		c, _, _, _ = e.sharedProfile(3)
		r := e.handle(Request{ID: 1, Mode: 3, Global: true, Revision: c.Revision, Op: "chain_param", Node: 1, Key: key, Value: .5})
		after, _ := os.ReadFile(e.sharedPath())
		if r["ok"] != false || !bytes.Equal(before, after) {
			t.Fatal("fractional enum changed config", key)
		}
	}
	run(Request{Op: "chain_duplicate", Node: 1})
	run(Request{Op: "chain_param", Node: 2, Key: "middle_source", Value: 7})
	run(Request{Op: "chain_param", Node: 2, Key: "left_delay", Value: 12.34})
	run(Request{Op: "chain_move", Node: 2, Value: 0})
	run(Request{Op: "preset_save", Name: "FFmpeg-Haas"})
	next := newEngine(e.dir)
	c, _, _, err := next.sharedProfile(0)
	if err != nil || c.Nodes[0].ID != 2 || c.Nodes[0].Params[4] != 12.34 || c.Nodes[1].Params[4] != 2.05 || c.Nodes[0].Params[3] != 7 {
		t.Fatal("independence/persistence", c, err)
	}
	if _, err := next.parseChain(chainText(c)); err != nil {
		t.Fatal(err)
	}
	if len(next.presets()) != 1 || next.presets()[0].Chain.Nodes[0].Type != 14 {
		t.Fatal("offline preset")
	}
}
