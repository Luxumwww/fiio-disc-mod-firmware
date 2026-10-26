package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"syscall"
	"testing"
)

func TestLegacyProfileAndPresetMigration(t *testing.T) {
	dir := t.TempDir()
	e := newEngine(dir)
	e.maskPath = filepath.Join(dir, "masks")
	os.WriteFile(e.maskPath, []byte("0 16 16 48\n"), 0600)
	legacy := e.defaults()
	legacy[4][0] = 2.2
	var b strings.Builder
	b.WriteString("1")
	for i := 0; i < 8; i++ {
		for j := 0; j < 12; j++ {
			fmt.Fprintf(&b, " %.6g", legacy[i][j])
		}
	}
	os.WriteFile(filepath.Join(dir, "dsp-3.params"), []byte(b.String()), 0600)
	p := e.parameters(3)
	if p != legacy {
		t.Fatal("legacy mode lost parameters or new defaults")
	}
	old := []map[string]any{{"ID": "abcdef", "Name": "旧预设", "Mode": 3, "Mask": 16, "Params": legacy[:8]}}
	raw, _ := json.Marshal(old)
	os.WriteFile(filepath.Join(dir, "presets.json"), raw, 0600)
	restored := newEngine(dir)
	restored.maskPath = e.maskPath
	items := restored.presets()
	if len(items) != 1 || items[0].Params != legacy {
		t.Fatal("old presets migration")
	}
	projection, _ := os.ReadFile(filepath.Join(dir, "preset-abcdef.profile"))
	if !strings.HasPrefix(string(projection), "DSP-PRESET-2 16\n2 ") {
		t.Fatal("native projection missing")
	}
	r := restored.handle(Request{ID: 1, Op: "preset_apply", Mode: 1, Preset: "abcdef"})
	if r["ok"] != true {
		t.Fatal(r)
	}
	if restored.parameters(1) != legacy || restored.masks() != [4]uint32{0, 16, 16, 48} {
		t.Fatal("bank isolation")
	}
	// Apply and save preserve new effect parameters, after a complete restart.
	r = restored.handle(Request{ID: 2, Op: "dsp_param", Mode: 1, Effect: 9, Key: "time", Value: 700})
	if r["ok"] != true {
		t.Fatal(r)
	}
	r = restored.handle(Request{ID: 3, Op: "dsp_toggle", Mode: 1, Effect: 9, Enabled: true})
	if r["ok"] != true {
		t.Fatal(r)
	}
	r = restored.handle(Request{ID: 4, Op: "preset_save", Mode: 1, Name: "回声测试"})
	if r["ok"] != true {
		t.Fatal(r)
	}
	r = restored.handle(Request{ID: 5, Op: "preset_save", Mode: 1, Name: "回声测试"})
	if r["ok"] != true || len(restored.presets()) != 2 {
		t.Fatal("same bank/name duplicates", r)
	}
	next := newEngine(dir)
	next.maskPath = e.maskPath
	if next.parameters(1)[9][0] != 700 || next.masks()[1] != 528 || len(next.presets()) != 2 {
		t.Fatal("restart did not retain new values")
	}
	id := next.presets()[1].ID
	r = next.handle(Request{ID: 6, Op: "preset_delete", Mode: 1, Preset: id})
	if r["ok"] != true || len(next.presets()) != 1 {
		t.Fatal(r)
	}
	index, _ := os.ReadFile(filepath.Join(dir, "presets.index"))
	if strings.Contains(string(index), id) {
		t.Fatal("native index kept deleted preset")
	}
}
func TestPresetValidationAndCrossProcessLock(t *testing.T) {
	e := newEngine(t.TempDir())
	e.maskPath = filepath.Join(e.dir, "masks")
	for _, name := range []string{"", "new\nline", "ta\tb"} {
		if e.handle(Request{ID: 1, Op: "preset_save", Name: name})["ok"] != false {
			t.Fatal("control character allowed")
		}
	}
	for _, effect := range []int{1, 12, -1} {
		if e.handle(Request{ID: 1, Op: "dsp_toggle", Effect: effect})["ok"] != false {
			t.Fatal("invalid effect")
		}
	}
	lock, err := os.OpenFile(filepath.Join(e.dir, "controls.lock"), os.O_RDWR, 0600)
	if err != nil {
		t.Fatal(err)
	}
	defer lock.Close()
	if syscall.Flock(int(lock.Fd()), syscall.LOCK_EX|syscall.LOCK_NB) != nil {
		t.Fatal("lock")
	}
	if e.handle(Request{ID: 1, Op: "preset_save", Name: "争用"})["ok"] != false {
		t.Fatal("cross process lock ignored")
	}
	syscall.Flock(int(lock.Fd()), syscall.LOCK_UN)
	if len(e.presets()) != 0 {
		t.Fatal("busy request mutated storage")
	}
}
