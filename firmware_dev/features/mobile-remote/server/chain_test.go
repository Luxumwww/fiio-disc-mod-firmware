package main

import (
	"encoding/json"
	"math"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestChainIndependentOrderRevisionAndPersistence(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	e.saveMasks([4]uint32{16, 32, 64, 256})
	op := func(q Request) map[string]any {
		q.ID = 1
		q.Mode = 3
		q.Revision = e.draftChain(3).Revision
		r := e.handle(q)
		if r["ok"] != true {
			t.Fatal(r)
		}
		return r
	}
	op(Request{Op: "chain_add", Effect: 12})
	op(Request{Op: "chain_duplicate", Node: 1})
	op(Request{Op: "chain_param", Node: 2, Key: "gain", Value: 3})
	c := e.draftChain(3)
	if c.Nodes[0].Params[1] != 0 || c.Nodes[1].Params[1] != 3 {
		t.Fatal("duplicate parameters aliased")
	}
	stale := c.Revision
	op(Request{Op: "chain_move", Node: 2, Value: 0})
	if e.draftChain(3).Nodes[0].ID != 2 {
		t.Fatal("order not changed")
	}
	if e.handle(Request{ID: 1, Op: "chain_remove", Mode: 3, Node: 1, Revision: stale})["ok"] != false {
		t.Fatal("stale mutation accepted")
	}
	c = e.draftChain(3)
	text := chainText(c)
	parsed, err := e.parseChain(text)
	if err != nil || len(parsed.Nodes) != 2 || parsed.Nodes[0].Params[1] != 3 {
		t.Fatal("format roundtrip", err)
	}
	if err = e.applyChain(3, c); err != nil {
		t.Fatal(err)
	}
	if e.masks() != [4]uint32{16, 32, 64, 4096} {
		t.Fatal("mode masks damaged")
	}
	if e.parameters(3) != e.defaults() {
		t.Fatal("legacy profile overwritten")
	}
	op(Request{Op: "preset_save", Name: "双EQ", Key: "chain"})
	next := newEngine(e.dir)
	next.maskPath = e.maskPath
	items := next.presets()
	if len(items) != 1 || items[0].Chain == nil || items[0].Chain.Nodes[0].ID != 2 {
		t.Fatal("saved chain missing")
	}
	b, _ := os.ReadFile(filepath.Join(e.dir, "preset-"+items[0].ID+".profile"))
	if !strings.HasPrefix(string(b), "DSP-PRESET-3\n") {
		t.Fatal("offline projection")
	}
	if next.draftChain(3).Nodes[0].Params[1] != 3 {
		t.Fatal("restart lost parameters")
	}
	c.Nodes[0].Params[1] = math.NaN()
	if e.validChain(c) {
		t.Fatal("NaN accepted")
	}
	c = e.draftChain(3)
	c.Nodes[1].ID = c.Nodes[0].ID
	if e.validChain(c) {
		t.Fatal("duplicate identity accepted")
	}
	for _, b := range []string{"1 1", "1 9", "1 0 extra", "1 -1"} {
		if _, err = e.parseChain([]byte(b)); err == nil {
			t.Fatal("bad format accepted")
		}
	}
}
func TestBoundedConsoleChainPayload(t *testing.T) {
	e := newEngine(t.TempDir())
	c := Chain{Nodes: []ChainNode{}}
	for i := 0; i < 8; i++ {
		f, _ := e.module(12)
		n := ChainNode{ID: uint32(i + 1), Type: 12, Enabled: true}
		for j, p := range f.Params {
			n.Params[j] = p.Default
		}
		c.Nodes = append(c.Nodes, n)
	}
	if !e.validChain(c) {
		t.Fatal("valid max chain")
	}
	b, _ := json.Marshal(map[string]any{"draft": c, "active": c, "eq": Equalizer{Bands: make([]Band, 10)}, "audio": audioState()})
	if len(b) > 12000 {
		t.Fatal(len(b))
	}
	c.Nodes = append(c.Nodes, c.Nodes[0])
	if e.validChain(c) {
		t.Fatal("overfull accepted")
	}
}
