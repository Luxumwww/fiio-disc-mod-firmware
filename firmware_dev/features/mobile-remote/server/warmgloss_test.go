package main

import (
	"bytes"
	"math"
	"os"
	"path/filepath"
	"testing"
)

func TestWarmGlossIndependentPersistentAndValidated(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	e.audioPath = filepath.Join(e.dir, "audio")
	if err := os.WriteFile(e.audioPath, []byte("shared_supported=1\nwork_mode=1\n"), 0600); err != nil {
		t.Fatal(err)
	}
	f, ok := e.module(16)
	if !ok || f.Key != "warmgloss" || f.Name != "暖润光泽" || len(f.Params) != 8 {
		t.Fatal("new independent controls missing")
	}
	// A chain-only module must never enter the fixed 12-row legacy table.
	for _, legacy := range e.schema.Effects {
		if legacy.Index >= 12 {
			t.Fatal("chain-only module leaked into legacy parameter table")
		}
	}
	run := func(q Request) {
		q.ID, q.Global, q.Mode = 1, true, 3
		c, _, _, err := e.sharedProfile(3)
		if err != nil {
			t.Fatal(err)
		}
		q.Revision = c.Revision
		if reply := e.handle(q); reply["ok"] != true {
			t.Fatal(reply)
		}
	}
	run(Request{Op: "chain_add", Effect: 3, Enabled: false})
	run(Request{Op: "chain_add", Effect: 15, Enabled: false})
	c, _, _, _ := e.sharedProfile(3)
	legacyTube, stereoFX := c.Nodes[0], c.Nodes[1]
	run(Request{Op: "chain_add", Effect: 16, Enabled: false})
	c, master, _, err := e.sharedProfile(0)
	defaults := [12]float64{.75, .8, 3, -.75, 160, -1, .55, .5}
	if err != nil || master || c.Nodes[2].Params != defaults || c.Nodes[2].Enabled {
		t.Fatal("new module defaults or shared scope", c, master, err)
	}
	run(Request{Op: "chain_param", Node: 3, Key: "bass", Value: -3})
	run(Request{Op: "chain_param", Node: 3, Key: "sag", Value: .65})
	run(Request{Op: "chain_param", Node: 3, Key: "recovery", Value: .8})
	run(Request{Op: "chain_duplicate", Node: 3})
	run(Request{Op: "chain_param", Node: 4, Key: "gloss", Value: 3.2})
	run(Request{Op: "chain_param", Node: 4, Key: "recovery", Value: .2})
	run(Request{Op: "preset_save", Name: "暖润光泽试听"})
	c, _, _, _ = e.sharedProfile(3)
	if c.Nodes[0] != legacyTube || c.Nodes[1] != stereoFX || c.Nodes[2].Params[2] != 3 || c.Nodes[3].Params[2] != 3.2 || c.Nodes[2].Params[6] != .65 || c.Nodes[2].Params[7] != .8 || c.Nodes[3].Params[7] != .2 {
		t.Fatal("independent instances changed other effects", c)
	}
	before, err := os.ReadFile(e.sharedPath())
	if err != nil {
		t.Fatal(err)
	}
	for _, invalid := range []Request{
		{Key: "amount", Value: -0.01}, {Key: "tube", Value: 1.01},
		{Key: "gloss", Value: 4.1}, {Key: "bass", Value: .1},
		{Key: "bass_freq", Value: 59}, {Key: "output", Value: .1},
		{Key: "sag", Value: 1.01}, {Key: "recovery", Value: -0.01},
		{Key: "amount", Value: math.NaN()}, {Key: "gloss", Value: math.Inf(1)},
	} {
		invalid.ID, invalid.Global, invalid.Mode, invalid.Op, invalid.Node = 1, true, 3, "chain_param", 3
		invalid.Revision = c.Revision
		if reply := e.handle(invalid); reply["ok"] != false {
			t.Fatal("invalid value accepted", invalid, reply)
		}
		after, err := os.ReadFile(e.sharedPath())
		if err != nil || !bytes.Equal(before, after) {
			t.Fatal("invalid value modified persisted profile", err)
		}
	}
	// All eight declared endpoints are accepted, while unused parameter slots stay zero.
	for at, p := range f.Params {
		for _, endpoint := range []float64{p.Min, p.Max} {
			candidate := Chain{Nodes: []ChainNode{{ID: 1, Type: 16, Params: defaults}}}
			candidate.Nodes[0].Params[at] = endpoint
			if !e.validChain(candidate) {
				t.Fatal("declared endpoint rejected", at, endpoint)
			}
		}
	}
	bad := Chain{Nodes: []ChainNode{{ID: 1, Type: 16, Params: defaults}}}
	bad.Nodes[0].Params[8] = .1
	if e.validChain(bad) {
		t.Fatal("unused parameter slot accepted")
	}
	next := newEngine(e.dir)
	c, master, _, err = next.sharedProfile(1)
	if err != nil || master || len(c.Nodes) != 4 || c.Nodes[2].Type != 16 || c.Nodes[2].Params[3] != -3 || c.Nodes[3].Params[2] != 3.2 || c.Nodes[2].Params[6] != .65 || c.Nodes[2].Params[7] != .8 || c.Nodes[3].Params[7] != .2 {
		t.Fatal("restart/shared mode persistence", c, err)
	}
	presets := next.presets()
	if len(presets) != 1 || presets[0].Chain == nil || presets[0].Chain.Nodes[2].Type != 16 {
		t.Fatal("offline preset failed to retain new module")
	}
	run(Request{Op: "chain_reset", Node: 3})
	c, _, _, _ = e.sharedProfile(3)
	if c.Nodes[2].Params != defaults || c.Nodes[2].Enabled || c.Nodes[3].Params[2] != 3.2 || c.Nodes[0] != legacyTube || c.Nodes[1] != stereoFX {
		t.Fatal("reset affected another module or switch", c)
	}
}

func TestWarmGlossLegacySixParameterProfileRemainsUnchanged(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	e.audioPath = filepath.Join(e.dir, "audio")
	if err := os.WriteFile(e.audioPath, []byte("shared_supported=1\nwork_mode=1\n"), 0600); err != nil {
		t.Fatal(err)
	}
	oldText := []byte("DSP-SHARED-1 1\n1 1\n1 16 1 0.45 0.55 1.8 -1.5 160 -1 0 0 0 0 0 0\n")
	if err := os.WriteFile(e.sharedPath(), oldText, 0600); err != nil {
		t.Fatal(err)
	}
	oldParams := [12]float64{.45, .55, 1.8, -1.5, 160, -1}
	c, master, _, err := e.sharedProfile(3)
	if err != nil || !master || len(c.Nodes) != 1 || c.Nodes[0].Params != oldParams {
		t.Fatal("old six-parameter profile rejected or retuned", c, err)
	}
	before, err := os.ReadFile(e.sharedPath())
	if err != nil || !bytes.Equal(before, oldText) {
		t.Fatal("reading old profile modified it", err)
	}
	if reply := e.handle(Request{ID: 1, Global: true, Mode: 3, Revision: c.Revision, Op: "chain_param", Node: 1, Key: "recovery", Value: .5}); reply["ok"] != true {
		t.Fatal(reply)
	}
	c, _, _, err = e.sharedProfile(0)
	want := oldParams
	want[7] = .5
	if err != nil || c.Nodes[0].Params != want || c.Nodes[0].Params[6] != 0 {
		t.Fatal("one parameter change silently reset old voicing or enabled sag", c, err)
	}
	// Zero is a valid normalized endpoint, with no schema/loader exception.
	if reply := e.handle(Request{ID: 1, Global: true, Mode: 3, Revision: c.Revision, Op: "chain_param", Node: 1, Key: "recovery", Value: 0}); reply["ok"] != true {
		t.Fatal("zero endpoint rejected", reply)
	}
}
