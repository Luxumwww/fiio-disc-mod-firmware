package main

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestSharedAutomaticApplyPersistenceAndOldBankPreservation(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	e.audioPath = filepath.Join(e.dir, "audio-state")
	os.WriteFile(e.audioPath, []byte("shared_supported=1\nwork_mode=1\n"), 0600)
	e.saveMasks([4]uint32{288, 16, 16, 0})
	legacy := e.parameters(3)
	run := func(q Request) {
		q.ID = 1
		q.Mode = 3
		q.Global = true
		c, _, _, err := e.sharedProfile(3)
		if err != nil {
			t.Fatal(err)
		}
		q.Revision = c.Revision
		r := e.handle(q)
		if r["ok"] != true {
			t.Fatal(r)
		}
	}
	run(Request{Op: "chain_add", Effect: 8, Enabled: false})
	c, on, _, _ := e.sharedProfile(0)
	if len(c.Nodes) != 1 || on {
		t.Fatal("add disabled or shared scope")
	}
	run(Request{Op: "chain_toggle", Node: 1, Enabled: true})
	c, on, _, _ = e.sharedProfile(1)
	if !on || !c.Nodes[0].Enabled {
		t.Fatal("automatic activate")
	}
	run(Request{Op: "chain_param", Node: 1, Key: "width", Value: .5})
	for mode := 0; mode < 4; mode++ {
		c, _, _, _ := e.sharedProfile(mode)
		if c.Nodes[0].Params[0] != .5 {
			t.Fatal("mode did not share profile")
		}
	}
	run(Request{Op: "chain_master", Enabled: false})
	run(Request{Op: "chain_param", Node: 1, Key: "width", Value: 1.2})
	_, on, _, _ = e.sharedProfile(3)
	if on {
		t.Fatal("editing resumed paused effects")
	}
	run(Request{Op: "chain_duplicate", Node: 1})
	run(Request{Op: "chain_param", Node: 2, Key: "width", Value: .8})
	c, _, _, _ = e.sharedProfile(3)
	if c.Nodes[0].Params[0] != 1.2 || c.Nodes[1].Params[0] != .8 {
		t.Fatal("duplicate alias")
	}
	run(Request{Op: "chain_move", Node: 2, Value: 0})
	c, _, _, _ = e.sharedProfile(3)
	if c.Nodes[0].ID != 2 {
		t.Fatal("order")
	}
	original, _ := os.ReadFile(e.sharedPath())
	bad := Request{ID: 2, Global: true, Mode: 3, Op: "chain_remove", Node: 1, Revision: c.Revision - 1}
	if e.handle(bad)["ok"] != false {
		t.Fatal("stale accepted")
	}
	after, _ := os.ReadFile(e.sharedPath())
	if !bytes.Equal(original, after) {
		t.Fatal("failed write changed active profile")
	}
	run(Request{Op: "preset_save", Name: "统一测试"})
	items := e.presets()
	if len(items) != 1 || !items[0].Global || items[0].Chain == nil {
		t.Fatal("global named preset")
	}
	view, _ := os.ReadFile(filepath.Join(e.dir, "preset-"+items[0].ID+".profile"))
	if !strings.HasPrefix(string(view), "DSP-PRESET-3\n") {
		t.Fatal("offline native projection")
	}
	run(Request{Op: "preset_save", Name: "统一测试"})
	if len(e.presets()) != 1 {
		t.Fatal("same name duplicate")
	}
	next := newEngine(e.dir)
	c, _, _, err := next.sharedProfile(0)
	if err != nil || c.Nodes[0].ID != 2 || c.Nodes[0].Params[0] != .8 {
		t.Fatal("restart persistence", err)
	}
	if e.masks() != [4]uint32{288, 16, 16, 0} || e.parameters(3) != legacy {
		t.Fatal("legacy banks damaged")
	}
}
func TestSharedRequiresUpdatedNativeAndRejectsMalformedProfile(t *testing.T) {
	e := newEngine(t.TempDir())
	e.audioPath = filepath.Join(e.dir, "audio")
	if e.handle(Request{ID: 1, Mode: 3, Global: true, Op: "chain_add", Effect: 8})["ok"] != false {
		t.Fatal("old native accepted")
	}
	if _, err := os.Stat(e.sharedPath()); !os.IsNotExist(err) {
		t.Fatal("configuration changed before support")
	}
	os.WriteFile(e.sharedPath(), []byte("DSP-SHARED-1 2\n1 0\n"), 0600)
	if _, _, _, err := e.sharedProfile(3); err == nil {
		t.Fatal("invalid master accepted")
	}
	os.WriteFile(e.sharedPath(), []byte("DSP-SHARED-1 1\n1 0 extra\n"), 0600)
	if _, _, _, err := e.sharedProfile(3); err == nil {
		t.Fatal("trailing garbage accepted")
	}
}
