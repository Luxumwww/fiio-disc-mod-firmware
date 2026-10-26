package main

import (
	"bytes"
	"math"
	"os"
	"path/filepath"
	"testing"
)

func TestHaasSharedControlPersistenceAndValidation(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	e.audioPath = filepath.Join(e.dir, "audio")
	os.WriteFile(e.audioPath, []byte("shared_supported=1\nwork_mode=1\n"), 0600)
	if len(e.schema.Effects) != 11 {
		t.Fatal("legacy fixed matrix changed")
	}
	for _, legacy := range e.schema.Effects {
		if legacy.Index >= 12 || legacy.Index == 1 {
			t.Fatal("chain-only module in legacy matrix")
		}
	}
	f, ok := e.module(13)
	if !ok || len(f.Params) != 5 {
		t.Fatal("Haas schema missing")
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
	run(Request{Op: "chain_add", Effect: 13, Enabled: false})
	c, on, _, _ := e.sharedProfile(3)
	if on || len(c.Nodes) != 1 || c.Nodes[0].Enabled {
		t.Fatal("add must stay off")
	}
	if c.Nodes[0].Params != [12]float64{3, .35, 2, 150, 0} {
		t.Fatal("defaults", c.Nodes)
	}
	run(Request{Op: "chain_param", Node: 1, Key: "delay", Value: 7.3})
	run(Request{Op: "chain_param", Node: 1, Key: "direction", Value: 1})
	before, _ := os.ReadFile(e.sharedPath())
	c, _, _, _ = e.sharedProfile(3)
	r := e.handle(Request{ID: 1, Global: true, Mode: 3, Revision: c.Revision, Op: "chain_param", Node: 1, Key: "direction", Value: .5})
	after, _ := os.ReadFile(e.sharedPath())
	if r["ok"] != false || !bytes.Equal(before, after) {
		t.Fatal("fractional direction altered saved config")
	}
	run(Request{Op: "chain_duplicate", Node: 1})
	run(Request{Op: "chain_param", Node: 2, Key: "mix", Value: .2})
	run(Request{Op: "chain_move", Node: 2, Value: 0})
	run(Request{Op: "preset_save", Name: "差分环绕"})
	next := newEngine(e.dir)
	next.maskPath = e.maskPath
	for mode := 0; mode < 4; mode++ {
		c, _, _, err := next.sharedProfile(mode)
		if err != nil || len(c.Nodes) != 2 || c.Nodes[0].ID != 2 || c.Nodes[0].Params[1] != .2 || c.Nodes[1].Params[1] != .35 || c.Nodes[1].Params[0] != 7.3 {
			t.Fatal("roundtrip/independence/shared", err, c)
		}
	}
	items := next.presets()
	if len(items) != 1 || items[0].Chain == nil || items[0].Chain.Nodes[0].Type != 13 {
		t.Fatal("preset roundtrip")
	}
	c, _, _, _ = e.sharedProfile(3)
	for _, v := range []float64{math.NaN(), math.Inf(1), -.1, 1.1, .5} {
		bad := c
		bad.Nodes = append([]ChainNode{}, c.Nodes...)
		bad.Nodes[0].Params[2] = v
		if e.validChain(bad) {
			t.Fatal("bad direction", v)
		}
	}
	for j := 5; j < 12; j++ {
		bad := c
		bad.Nodes = append([]ChainNode{}, c.Nodes...)
		bad.Nodes[0].Params[j] = .1
		if e.validChain(bad) {
			t.Fatal("nonzero unused parameter")
		}
	}
}
