package main

import (
	"bytes"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestSavedAndActiveLimitsAreSeparate(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	e.audioPath = filepath.Join(e.dir, "audio")
	os.WriteFile(e.audioPath, []byte("shared_supported=1\nwork_mode=1\n"), 0600)
	run := func(q Request) map[string]any {
		q.ID = 1
		q.Global = true
		q.Mode = 3
		c, _, _, _ := e.sharedProfile(3)
		q.Revision = c.Revision
		return e.handle(q)
	}
	for _, typ := range []int{4, 2, 5, 14, 3, 6, 0, 15} {
		if r := run(Request{Op: "chain_add", Effect: typ, Enabled: false}); r["ok"] != true {
			t.Fatal(r)
		}
	}
	c, _, _, _ := e.sharedProfile(3)
	old := append([]ChainNode{}, c.Nodes...)
	if r := run(Request{Op: "chain_add", Effect: 7, Enabled: true}); r["ok"] != true {
		t.Fatal(r)
	}
	c, _, _, _ = e.sharedProfile(0)
	if len(c.Nodes) != 9 || c.Nodes[8].Type != 7 || !c.Nodes[8].Enabled {
		t.Fatal(c)
	}
	for i, n := range old {
		if c.Nodes[i] != n {
			t.Fatal("previous node changed", i)
		}
	}
	for len(c.Nodes) < 16 {
		if r := run(Request{Op: "chain_add", Effect: 5, Enabled: false}); r["ok"] != true {
			t.Fatal(r)
		}
		c, _, _, _ = e.sharedProfile(3)
	}
	before, _ := os.ReadFile(e.sharedPath())
	r := run(Request{Op: "chain_add", Effect: 7, Enabled: false})
	after, _ := os.ReadFile(e.sharedPath())
	if r["ok"] != false || !strings.Contains(r["error"].(string), "16") || !bytes.Equal(before, after) {
		t.Fatal(r)
	}
	for i := 1; i <= 7; i++ {
		if r := run(Request{Op: "chain_toggle", Node: uint32(i), Enabled: true}); r["ok"] != true {
			t.Fatal(r)
		}
	}
	before, _ = os.ReadFile(e.sharedPath())
	r = run(Request{Op: "chain_toggle", Node: 8, Enabled: true})
	after, _ = os.ReadFile(e.sharedPath())
	if r["ok"] != false || !strings.Contains(r["error"].(string), "8") || !bytes.Equal(before, after) {
		t.Fatal(r)
	}
	if r := run(Request{Op: "preset_save", Name: "16 saved"}); r["ok"] != true {
		t.Fatal(r)
	}
	next := newEngine(e.dir)
	c, _, _, err := next.sharedProfile(0)
	if err != nil || len(c.Nodes) != 16 || chainActiveCount(c) != 8 || len(next.presets()) != 1 {
		t.Fatal(c, err)
	}
	b, _ := json.Marshal(map[string]any{"draft": c, "active": c, "eq": Equalizer{Bands: make([]Band, 10)}, "audio": audioState()})
	if len(b) > 12000 {
		t.Fatal("BLE response too large", len(b))
	}
	if _, err = next.parseChain(chainText(c)); err != nil {
		t.Fatal(err)
	}
	c.Nodes[15].Enabled = true
	if next.validChain(c) {
		t.Fatal("ninth enabled accepted")
	}
}
