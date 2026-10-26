package main

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"
)

func TestParameterProfilesIsolationValidationAndRestart(t *testing.T) {
	e := newEngine(t.TempDir())
	original := e.parameters(3)
	r := e.handle(Request{ID: 1, Op: "dsp_param", Mode: 3, Effect: 4, Key: "decay", Value: 2.2})
	if r["ok"] != true {
		t.Fatal(r)
	}
	next := newEngine(e.dir)
	if next.parameters(3)[4][0] != 2.2 || next.parameters(0) != original {
		t.Fatal("mode persistence/isolation")
	}
	for _, bad := range []Request{{ID: 2, Op: "dsp_param", Mode: 3, Effect: 4, Key: "decay", Value: 100}, {ID: 3, Op: "dsp_param", Mode: 3, Effect: 1, Key: "decay", Value: 1}, {ID: 4, Op: "dsp_param", Mode: 4, Effect: 4, Key: "decay", Value: 2}} {
		if e.handle(bad)["ok"] != false {
			t.Fatal("invalid accepted")
		}
	}
	if next.parameters(3)[4][0] != 2.2 {
		t.Fatal("invalid write changed settings")
	}
	if e.handle(Request{ID: 5, Op: "dsp_reset", Mode: 3, Effect: 4})["ok"] != true || e.parameters(3) != original {
		t.Fatal("reset")
	}
	path := filepath.Join(e.dir, "dsp-3.params")
	os.WriteFile(path, []byte("1 NaN"), 0600)
	if e.parameters(3) != original {
		t.Fatal("corrupt file accepted")
	}
}
func TestSchemaAndBoundedPresetReply(t *testing.T) {
	e := newEngine(t.TempDir())
	count := 0
	for _, fx := range e.schema.Effects {
		if fx.Index == 1 {
			t.Fatal("retired effect")
		}
		count += len(fx.Params)
	}
	if count != 62 {
		t.Fatal(count)
	}
	items := make([]preset, 24)
	for i := range items {
		items[i] = preset{ID: "0123456789abcdef", Name: "测试预设", Mode: i % 4, Params: e.parameters(i % 4)}
	}
	b, _ := json.Marshal(presetSummaries(items))
	if len(b) > 4096 {
		t.Fatal(len(b))
	}
	for _, mode := range []int{255, 160, 169, 0, 10} {
		if !eqModeValid(mode) {
			t.Fatal(mode)
		}
	}
	for _, mode := range []int{240, 254, 7, 159, 170, -1} {
		if eqModeValid(mode) {
			t.Fatal(mode)
		}
	}
}
