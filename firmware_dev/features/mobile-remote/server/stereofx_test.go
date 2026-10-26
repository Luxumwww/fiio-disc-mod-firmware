package main

import (
	"bytes"
	"os"
	"path/filepath"
	"testing"
)

func TestStereoFXIndependentOfHaas(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	e.audioPath = filepath.Join(e.dir, "audio")
	os.WriteFile(e.audioPath, []byte("shared_supported=1\nwork_mode=1\n"), 0600)
	if f, ok := e.module(14); !ok || f.Name != "Haas 立体声 · FFmpeg" {
		t.Fatal("Haas rename missing")
	}
	if f, ok := e.module(15); !ok || len(f.Params) != 3 {
		t.Fatal("StereoFX controls missing")
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
	run(Request{Op: "chain_add", Effect: 15, Enabled: false})
	c, on, _, _ := e.sharedProfile(3)
	if on || c.Nodes[1].Params != [12]float64{.5, 0, 0} {
		t.Fatal(c, on)
	}
	run(Request{Op: "chain_param", Node: 2, Key: "width", Value: .73})
	run(Request{Op: "chain_duplicate", Node: 2})
	run(Request{Op: "chain_param", Node: 3, Key: "mono_bass", Value: .5})
	run(Request{Op: "preset_save", Name: "StereoFX"})
	before, _ := os.ReadFile(e.sharedPath())
	c, _, _, _ = e.sharedProfile(3)
	r := e.handle(Request{ID: 1, Mode: 3, Global: true, Revision: c.Revision, Op: "chain_param", Node: 2, Key: "mono_bass", Value: .71})
	after, _ := os.ReadFile(e.sharedPath())
	if r["ok"] != false || !bytes.Equal(before, after) {
		t.Fatal("invalid IIR endpoint changed settings")
	}
	next := newEngine(e.dir)
	c, _, _, err := next.sharedProfile(0)
	if err != nil || c.Nodes[0].Type != 14 || c.Nodes[0].Enabled || c.Nodes[1].Params[0] != .73 || c.Nodes[1].Params[1] != 0 || c.Nodes[2].Params[1] != .5 {
		t.Fatal(c, err)
	}
	if len(next.presets()) != 1 || next.presets()[0].Chain.Nodes[1].Type != 15 {
		t.Fatal("offline preset")
	}
}
