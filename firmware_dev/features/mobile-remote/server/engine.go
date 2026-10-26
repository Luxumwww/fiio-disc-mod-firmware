package main

import (
	"embed"
	"encoding/json"
	"errors"
	"fmt"
	"math"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"
	"unicode"
)

//go:embed schema.json
var schemaFiles embed.FS

type Param struct {
	Key, Label, Unit        string
	Min, Max, Step, Default float64
}
type Effect struct {
	Index     int
	Key, Name string
	Params    []Param
}
type Schema struct {
	Version int
	Effects []Effect
	Modules []Effect
}
type Request struct {
	ID       int     `json:"id"`
	Op       string  `json:"op"`
	Mode     int     `json:"mode"`
	Effect   int     `json:"effect"`
	Key      string  `json:"key"`
	Value    float64 `json:"value"`
	Enabled  bool    `json:"enabled"`
	Band     Band    `json:"band"`
	Preset   string  `json:"preset"`
	Name     string  `json:"name"`
	Node     uint32  `json:"node"`
	Revision uint32  `json:"revision"`
	Global   bool    `json:"global"`
}
type engine struct {
	mu        sync.Mutex
	dir       string
	schema    Schema
	maskPath  string
	eq        fiio
	audioPath string
}

func abs(v float64) float64 {
	if v < 0 {
		return -v
	}
	return v
}
func finiteRange(v, lo, hi float64) bool {
	return !math.IsNaN(v) && !math.IsInf(v, 0) && v >= lo && v <= hi
}
func atomicFile(path string, b []byte) error {
	f, e := os.CreateTemp(filepath.Dir(path), ".remote-")
	if e != nil {
		return e
	}
	defer os.Remove(f.Name())
	if _, e = f.Write(b); e != nil {
		f.Close()
		return e
	}
	if e = f.Sync(); e != nil {
		f.Close()
		return e
	}
	if e = f.Close(); e != nil {
		return e
	}
	return os.Rename(f.Name(), path)
}
func newEngine(dir string) *engine {
	e := &engine{dir: dir, maskPath: "/usr/data/disc-runtime/module-data/disc-audio-dsp/settings", audioPath: "/tmp/disc-runtime/audio-dsp.state"}
	b, _ := schemaFiles.ReadFile("schema.json")
	json.Unmarshal(b, &e.schema)
	os.MkdirAll(dir, 0700)
	e.syncPresetView(e.presets())
	return e
}
func (e *engine) parameters(mode int) (p [12][12]float64) {
	for _, fx := range e.schema.Effects {
		for i, v := range fx.Params {
			p[fx.Index][i] = v.Default
		}
	}
	b, err := os.ReadFile(filepath.Join(e.dir, fmt.Sprintf("dsp-%d.params", mode)))
	if err != nil {
		return
	}
	fields := strings.Fields(string(b))
	rows := 12
	if len(fields) == 97 && fields[0] == "1" {
		rows = 8
	} else if len(fields) != 145 || fields[0] != "2" {
		return
	}
	saved := p
	for i := 0; i < rows; i++ {
		for j := 0; j < 12; j++ {
			v, ex := strconv.ParseFloat(fields[1+i*12+j], 64)
			if ex != nil {
				return
			}
			saved[i][j] = v
		}
	}
	if !e.validParameters(saved) {
		return
	}
	return saved
}
func (e *engine) validParameters(p [12][12]float64) bool {
	var counts [12]int
	for _, fx := range e.schema.Effects {
		counts[fx.Index] = len(fx.Params)
		for i, v := range fx.Params {
			if !finiteRange(p[fx.Index][i], v.Min, v.Max) {
				return false
			}
		}
	}
	for i := 0; i < 12; i++ {
		for j := counts[i]; j < 12; j++ {
			if p[i][j] != 0 {
				return false
			}
		}
	}
	return true
}
func (e *engine) saveParameters(mode int, p [12][12]float64) error {
	if mode < 0 || mode > 3 || !e.validParameters(p) {
		return errors.New("invalid DSP profile")
	}
	var b strings.Builder
	b.WriteString("2")
	for i := 0; i < 12; i++ {
		for j := 0; j < 12; j++ {
			fmt.Fprintf(&b, " %.6g", p[i][j])
		}
	}
	b.WriteByte('\n')
	return atomicFile(filepath.Join(e.dir, fmt.Sprintf("dsp-%d.params", mode)), []byte(b.String()))
}
func (e *engine) masks() (v [4]uint32) {
	b, _ := os.ReadFile(e.maskPath)
	fmt.Sscanf(string(b), "%d %d %d %d", &v[0], &v[1], &v[2], &v[3])
	for i := range v {
		v[i] &= 8189
	}
	return
}
func (e *engine) saveMasks(v [4]uint32) error {
	return atomicFile(e.maskPath, []byte(fmt.Sprintf("%d %d %d %d\n", v[0]&8189, v[1]&8189, v[2]&8189, v[3]&8189)))
}
func (e *engine) status() map[string]any {
	m := e.masks()
	params := make([][12][12]float64, 4)
	for i := 0; i < 4; i++ {
		params[i] = e.parameters(i)
	}
	eq, err := e.eq.get()
	state := map[string]any{"masks": m, "params": params, "eq": eq, "version": "0.8.1"}
	if err != nil {
		state["eq_error"] = "均衡器暂未连接，请刷新"
	}
	b, _ := os.ReadFile("/tmp/disc-runtime/audio-dsp.state")
	state["audio"] = string(b)
	return state
}

type preset struct {
	ID, Name string
	Mode     int
	Mask     uint32
	Params   [12][12]float64
	Chain    *Chain `json:"Chain,omitempty"`
	Global   bool   `json:"Global,omitempty"`
}

func (e *engine) defaults() (p [12][12]float64) {
	for _, fx := range e.schema.Effects {
		for i, v := range fx.Params {
			p[fx.Index][i] = v.Default
		}
	}
	return
}
func validPresetID(id string) bool {
	if len(id) < 1 || len(id) > 32 {
		return false
	}
	for _, c := range id {
		if !(c >= '0' && c <= '9' || c >= 'a' && c <= 'f') {
			return false
		}
	}
	return true
}
func validPresetName(name string) bool {
	if n := len([]rune(name)); n < 1 || n > 32 {
		return false
	}
	for _, c := range name {
		if unicode.IsControl(c) {
			return false
		}
	}
	return true
}
func (e *engine) presets() []preset {
	path := filepath.Join(e.dir, "presets.json")
	st, err := os.Stat(path)
	if err != nil || st.Size() > 1<<20 {
		return nil
	}
	b, _ := os.ReadFile(path)
	var raw []struct {
		ID, Name string
		Mode     int
		Mask     uint32
		Params   [][]float64
		Chain    *Chain
		Global   bool
	}
	if json.Unmarshal(b, &raw) != nil || len(raw) > 24 {
		return nil
	}
	out := make([]preset, 0, len(raw))
	seen := map[string]bool{}
	for _, r := range raw {
		if !validPresetID(r.ID) || !validPresetName(r.Name) || seen[r.ID] || r.Mode < 0 || r.Mode > 3 || r.Mask&^uint32(8189) != 0 || (r.Chain == nil && len(r.Params) != 8 && len(r.Params) != 12) {
			continue
		}
		if r.Chain != nil && !e.validChain(*r.Chain) {
			continue
		}
		p := e.defaults()
		ok := true
		for i, row := range r.Params {
			if len(row) != 12 {
				ok = false
				break
			}
			copy(p[i][:], row)
		}
		if !ok || !e.validParameters(p) {
			continue
		}
		seen[r.ID] = true
		out = append(out, preset{r.ID, r.Name, r.Mode, r.Mask, p, r.Chain, r.Global})
	}
	return out
}
func parameterText(p [12][12]float64) []byte {
	var b strings.Builder
	b.WriteString("2")
	for i := 0; i < 12; i++ {
		for j := 0; j < 12; j++ {
			fmt.Fprintf(&b, " %.6g", p[i][j])
		}
	}
	b.WriteByte('\n')
	return []byte(b.String())
}
func writeChanged(path string, b []byte) error {
	old, _ := os.ReadFile(path)
	if string(old) == string(b) {
		return nil
	}
	return atomicFile(path, b)
}

// The native menu reads a bounded, validated projection, including while BLE
// is disabled. Publish its index last so it never points at an incomplete file.
func (e *engine) syncPresetView(items []preset) error {
	var index strings.Builder
	index.WriteString("2\n")
	for _, p := range items {
		b := append([]byte(fmt.Sprintf("DSP-PRESET-2 %d\n", p.Mask&4093)), parameterText(p.Params)...)
		if p.Chain != nil {
			b = append([]byte("DSP-PRESET-3\n"), chainText(*p.Chain)...)
		}
		if ex := writeChanged(filepath.Join(e.dir, "preset-"+p.ID+".profile"), b); ex != nil {
			return ex
		}
		fmt.Fprintf(&index, "%s\t%d\t%s\n", p.ID, p.Mode, p.Name)
	}
	return writeChanged(filepath.Join(e.dir, "presets.index"), []byte(index.String()))
}
func (e *engine) persistPresets(items []preset) error {
	b, _ := json.Marshal(items)
	if ex := atomicFile(filepath.Join(e.dir, "presets.json"), b); ex != nil {
		return ex
	}
	return e.syncPresetView(items)
}
func presetSummaries(items []preset) []map[string]any {
	out := make([]map[string]any, 0, len(items))
	for _, p := range items {
		count := 0
		if p.Chain != nil {
			count = len(p.Chain.Nodes)
		} else {
			for i := 0; i < 12; i++ {
				if p.Mask&(1<<i) != 0 {
					count++
				}
			}
		}
		out = append(out, map[string]any{"ID": p.ID, "Name": p.Name, "Mode": p.Mode, "Count": count, "Global": p.Global})
	}
	return out
}
func (e *engine) handle(req Request) (out map[string]any) {
	e.mu.Lock()
	defer e.mu.Unlock()
	lock, ex := os.OpenFile(filepath.Join(e.dir, "controls.lock"), os.O_CREATE|os.O_RDWR, 0600)
	if ex != nil {
		return map[string]any{"id": req.ID, "ok": false, "error": "设置目录不可用"}
	}
	defer lock.Close()
	if syscall.Flock(int(lock.Fd()), syscall.LOCK_EX|syscall.LOCK_NB) != nil {
		return map[string]any{"id": req.ID, "ok": false, "error": "播放器正在应用设置，请稍后重试"}
	}
	defer syscall.Flock(int(lock.Fd()), syscall.LOCK_UN)

	out = map[string]any{"id": req.ID, "ok": false}
	var err error
	if req.ID < 1 || req.ID > 2147483647 {
		out["error"] = "无效请求"
		return
	}
	if req.Mode < 0 || req.Mode > 3 {
		out["error"] = "无效工作模式"
		return
	}
	if req.Global && (strings.HasPrefix(req.Op, "chain_") || req.Op == "console_status" || req.Op == "preset_save" || req.Op == "preset_apply" || req.Op == "presets") {
		err = e.sharedOperation(req, out)
		if err != nil {
			out["error"] = err.Error()
		} else {
			out["ok"] = true
		}
		return
	}
	if strings.HasPrefix(req.Op, "chain_") || req.Op == "console_status" {
		err = e.chainOperation(req, out)
		if err != nil {
			out["error"] = err.Error()
		} else {
			out["ok"] = true
		}
		return
	}
	switch req.Op {
	case "status":
		out["state"] = e.status()
	case "eq_select":
		err = e.eq.selectMode(int(req.Value))
		if err == nil {
			eq, ex := e.eq.get()
			out["eq"] = eq
			err = ex
		}
	case "eq_band":
		err = e.eq.setBand(int(req.Value), req.Band)
		if err == nil {
			eq, ex := e.eq.get()
			out["eq"] = eq
			err = ex
		}
	case "eq_reset":
		err = e.eq.resetGains(req.Effect)
		if err == nil {
			var ex error
			out["eq"], ex = e.eq.get()
			err = ex
		}
	case "eq_master":
		err = e.eq.setMaster(req.Effect, req.Value)
		if err == nil {
			eq, ex := e.eq.get()
			out["eq"] = eq
			err = ex
		}
	case "dsp_toggle":
		valid := false
		for _, fx := range e.schema.Effects {
			if fx.Index == req.Effect {
				valid = true
			}
		}
		if !valid {
			err = errors.New("unknown effect")
			break
		}
		m := e.masks()
		if req.Enabled {
			m[req.Mode] &= ^customMask
			m[req.Mode] |= 1 << req.Effect
		} else {
			m[req.Mode] &= ^(uint32(1) << req.Effect)
		}
		err = e.saveMasks(m)
		out["masks"] = m
	case "dsp_param":
		p := e.parameters(req.Mode)
		found := false
		for _, fx := range e.schema.Effects {
			if fx.Index != req.Effect {
				continue
			}
			for i, v := range fx.Params {
				if v.Key == req.Key && finiteRange(req.Value, v.Min, v.Max) {
					p[fx.Index][i] = req.Value
					found = true
				}
			}
		}
		if !found {
			err = errors.New("invalid parameter")
		} else {
			err = e.saveParameters(req.Mode, p)
		}
		out["params"] = p
	case "dsp_reset":
		p := e.parameters(req.Mode)
		found := false
		for _, fx := range e.schema.Effects {
			if fx.Index == req.Effect {
				for i, v := range fx.Params {
					p[fx.Index][i] = v.Default
				}
				found = true
			}
		}
		if !found {
			err = errors.New("unknown effect")
		} else {
			err = e.saveParameters(req.Mode, p)
		}
		out["params"] = p
	case "presets":
		out["presets"] = presetSummaries(e.presets())
	case "preset_save":
		name := strings.TrimSpace(req.Name)
		if !validPresetName(name) {
			err = errors.New("name length")
			break
		}
		items := e.presets()
		for i, p := range items {
			if p.Name == name && p.Mode == req.Mode {
				items[i] = e.snapshotPreset(p.ID, name, req.Mode, req.Key == "chain")
				err = e.persistPresets(items)
				out["presets"] = presetSummaries(items)
				returnWith := err
				if returnWith != nil {
					out["error"] = "预设保存失败"
				} else {
					out["ok"] = true
				}
				return
			}
		}
		if len(items) >= 24 {
			err = errors.New("preset limit")
			break
		}
		id := fmt.Sprintf("%x", time.Now().UnixNano())
		items = append(items, e.snapshotPreset(id, name, req.Mode, req.Key == "chain"))
		err = e.persistPresets(items)
		out["presets"] = presetSummaries(items)
	case "preset_delete":
		items := e.presets()
		found := false
		next := make([]preset, 0, len(items))
		for _, p := range items {
			if p.ID == req.Preset {
				found = true
			} else {
				next = append(next, p)
			}
		}
		if !found {
			err = errors.New("preset not found")
		} else {
			err = e.persistPresets(next)
			if err == nil {
				os.Remove(filepath.Join(e.dir, "preset-"+req.Preset+".profile"))
			}
		}
		out["presets"] = presetSummaries(next)
	case "preset_apply":
		found := false
		for _, p := range e.presets() {
			if p.ID == req.Preset {
				found = true
				if p.Chain != nil {
					err = e.applyChain(req.Mode, *p.Chain)
					if err == nil {
						draft := *p.Chain
						draft.Revision = e.draftChain(req.Mode).Revision + 1
						err = e.saveDraft(req.Mode, draft)
					}
					out["console"] = e.consoleStatus(req.Mode)
					break
				}
				err = e.saveParameters(req.Mode, p.Params)
				if err == nil {
					m := e.masks()
					m[req.Mode] = p.Mask & 4093
					err = e.saveMasks(m)
					out["masks"] = m
				}
				out["params"] = p.Params
				break
			}
		}
		if !found {
			err = errors.New("preset not found")
		}
	default:
		err = errors.New("unknown operation")
	}
	if err != nil {
		out["error"] = "设置未应用，请刷新后重试"
		out["detail"] = err.Error()
	} else {
		out["ok"] = true
	}
	return
}

func (e *engine) snapshotPreset(id, name string, mode int, chain bool) preset {
	p := preset{ID: id, Name: name, Mode: mode, Mask: e.masks()[mode], Params: e.parameters(mode)}
	if chain {
		c := e.draftChain(mode)
		p.Chain = &c
		p.Mask = customMask
	}
	return p
}
