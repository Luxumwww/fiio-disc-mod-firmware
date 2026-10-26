package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

const customMask uint32 = 4096
const chainSavedMax = 16
const chainActiveMax = 8

func chainActiveCount(c Chain) int {
	n := 0
	for _, v := range c.Nodes {
		if v.Enabled {
			n++
		}
	}
	return n
}
func chainCapacityError(c Chain) error {
	if len(c.Nodes) > chainSavedMax {
		return errors.New("最多保存16个音效，请先移除一项")
	}
	if chainActiveCount(c) > chainActiveMax {
		return errors.New("最多同时开启8个音效，请先关闭一项")
	}
	return nil
}

type ChainNode struct {
	ID      uint32      `json:"id"`
	Type    int         `json:"type"`
	Enabled bool        `json:"enabled"`
	Params  [12]float64 `json:"params"`
}
type Chain struct {
	Revision uint32      `json:"revision"`
	Nodes    []ChainNode `json:"nodes"`
}

func (e *engine) module(t int) (Effect, bool) {
	for _, f := range e.schema.Modules {
		if f.Index == t {
			return f, true
		}
	}
	return Effect{}, false
}
func (e *engine) validChain(c Chain) bool {
	if chainCapacityError(c) != nil {
		return false
	}
	seen := map[uint32]bool{}
	for _, n := range c.Nodes {
		f, ok := e.module(n.Type)
		if !ok || n.ID == 0 || n.ID > 2147483647 || seen[n.ID] {
			return false
		}
		seen[n.ID] = true
		for i, x := range n.Params {
			if i < len(f.Params) {
				p := f.Params[i]
				if !finiteRange(x, p.Min, p.Max) {
					return false
				}
			} else if x != 0 {
				return false
			}
		}
		if n.Type == 12 && n.Params[3] != float64(int(n.Params[3])) {
			return false
		}
		if n.Type == 13 && n.Params[2] != float64(int(n.Params[2])) {
			return false
		}
		if n.Type == 14 && (n.Params[3] != float64(int(n.Params[3])) || n.Params[7] != float64(int(n.Params[7])) || n.Params[11] != float64(int(n.Params[11]))) {
			return false
		}
	}
	return true
}
func chainText(c Chain) []byte {
	var b strings.Builder
	fmt.Fprintf(&b, "1 %d\n", len(c.Nodes))
	for _, n := range c.Nodes {
		v := 0
		if n.Enabled {
			v = 1
		}
		fmt.Fprintf(&b, "%d %d %d", n.ID, n.Type, v)
		for _, x := range n.Params {
			fmt.Fprintf(&b, " %.6g", x)
		}
		b.WriteByte('\n')
	}
	return []byte(b.String())
}
func hashText(b []byte) uint32 {
	h := uint32(2166136261)
	for _, x := range b {
		h = (h ^ uint32(x)) * 16777619
	}
	return h
}
func (e *engine) parseChain(b []byte) (Chain, error) {
	c := Chain{Nodes: []ChainNode{}}
	f := strings.Fields(string(b))
	if len(f) < 2 || f[0] != "1" {
		return c, errors.New("chain format")
	}
	n, err := strconv.Atoi(f[1])
	if err != nil || n < 0 || n > chainSavedMax || len(f) != 2+n*15 {
		return c, errors.New("chain size")
	}
	for i := 0; i < n; i++ {
		a := 2 + i*15
		id, x := strconv.ParseUint(f[a], 10, 31)
		if x != nil {
			return c, x
		}
		t, x := strconv.Atoi(f[a+1])
		if x != nil {
			return c, x
		}
		v, x := strconv.Atoi(f[a+2])
		if x != nil || v < 0 || v > 1 {
			return c, errors.New("chain switch")
		}
		node := ChainNode{ID: uint32(id), Type: t, Enabled: v == 1}
		for j := 0; j < 12; j++ {
			node.Params[j], x = strconv.ParseFloat(f[a+3+j], 64)
			if x != nil {
				return c, x
			}
		}
		c.Nodes = append(c.Nodes, node)
	}
	if !e.validChain(c) {
		return c, errors.New("invalid chain")
	}
	return c, nil
}
func (e *engine) activeChain(mode int) Chain {
	b, _ := os.ReadFile(filepath.Join(e.dir, fmt.Sprintf("chain-%d.conf", mode)))
	c, err := e.parseChain(b)
	if err != nil {
		return Chain{Nodes: []ChainNode{}}
	}
	c.Revision = hashText(b)
	return c
}
func (e *engine) draftChain(mode int) Chain {
	b, _ := os.ReadFile(filepath.Join(e.dir, fmt.Sprintf("chain-%d.draft", mode)))
	var c Chain
	if json.Unmarshal(b, &c) == nil && e.validChain(c) {
		if c.Nodes == nil {
			c.Nodes = []ChainNode{}
		}
		return c
	}
	return e.activeChain(mode)
}
func (e *engine) saveDraft(mode int, c Chain) error {
	if err := chainCapacityError(c); err != nil {
		return err
	}
	if !e.validChain(c) {
		return errors.New("invalid chain")
	}
	b, _ := json.Marshal(c)
	return atomicFile(filepath.Join(e.dir, fmt.Sprintf("chain-%d.draft", mode)), b)
}
func (e *engine) applyChain(mode int, c Chain) error {
	if err := chainCapacityError(c); err != nil {
		return err
	}
	if !e.validChain(c) {
		return errors.New("invalid chain")
	}
	path := filepath.Join(e.dir, fmt.Sprintf("chain-%d.conf", mode))
	old, existed := os.ReadFile(path)
	if err := atomicFile(path, chainText(c)); err != nil {
		return err
	}
	m := e.masks()
	m[mode] = customMask
	if err := e.saveMasks(m); err != nil {
		if existed == nil {
			atomicFile(path, old)
		} else {
			os.Remove(path)
		}
		return err
	}
	return nil
}
func audioState() map[string]string {
	b, _ := os.ReadFile("/tmp/disc-runtime/audio-dsp.state")
	s := map[string]string{}
	for _, line := range strings.Split(string(b), "\n") {
		v := strings.SplitN(line, "=", 2)
		if len(v) == 2 {
			s[v[0]] = v[1]
		}
	}
	return s
}
func (e *engine) consoleStatus(mode int) map[string]any {
	eq, err := e.eq.get()
	out := map[string]any{"version": "0.3.0", "mode": mode, "masks": e.masks(), "draft": e.draftChain(mode), "active": e.activeChain(mode), "audio": audioState(), "eq": eq}
	out["draft_hash"] = hashText(chainText(e.draftChain(mode)))
	b, _ := os.ReadFile("/proc/uptime")
	parts := strings.Fields(string(b))
	if len(parts) > 0 {
		out["uptime"], _ = strconv.ParseFloat(parts[0], 64)
	}
	if err != nil {
		out["eq_error"] = err.Error()
	}
	return out
}
func (e *engine) chainOperation(req Request, out map[string]any) error {
	if req.Op == "console_status" {
		out["console"] = e.consoleStatus(req.Mode)
		return nil
	}
	c := e.draftChain(req.Mode)
	if req.Revision != c.Revision {
		return errors.New("信号链已改变，请刷新")
	}
	if req.Op == "chain_import" {
		c.Nodes = []ChainNode{}
		p := e.parameters(req.Mode)
		m := e.masks()[req.Mode]
		for _, t := range []int{2, 0, 4, 3, 5, 6, 7, 8, 9, 10, 11} {
			if m&(1<<t) != 0 {
				if len(c.Nodes) >= 8 {
					return errors.New("现有音效超过8项")
				}
				c.Nodes = append(c.Nodes, ChainNode{ID: uint32(len(c.Nodes) + 1), Type: t, Enabled: true, Params: p[t]})
			}
		}
	}
	at := -1
	for i, n := range c.Nodes {
		if n.ID == req.Node {
			at = i
		}
	}
	nextID := uint32(1)
	for _, n := range c.Nodes {
		if n.ID >= nextID {
			nextID = n.ID + 1
		}
	}
	switch req.Op {
	case "chain_import":
	case "chain_add":
		f, ok := e.module(req.Effect)
		if !ok || len(c.Nodes) >= chainSavedMax || nextID > 2147483647 {
			return errors.New("模块无效或已保存16项")
		}
		n := ChainNode{ID: nextID, Type: req.Effect, Enabled: true}
		for i, p := range f.Params {
			n.Params[i] = p.Default
		}
		c.Nodes = append(c.Nodes, n)
	case "chain_clear":
		c.Nodes = []ChainNode{}
	case "chain_duplicate":
		if at < 0 || len(c.Nodes) >= chainSavedMax || nextID > 2147483647 {
			return errors.New("模块不存在或已保存16项")
		}
		n := c.Nodes[at]
		n.ID = nextID
		c.Nodes = append(c.Nodes, n)
	case "chain_remove":
		if at < 0 {
			return errors.New("模块不存在")
		}
		c.Nodes = append(c.Nodes[:at], c.Nodes[at+1:]...)
	case "chain_move":
		to := int(req.Value)
		if at < 0 || req.Value != float64(to) || to < 0 || to >= len(c.Nodes) {
			return errors.New("无效位置")
		}
		n := c.Nodes[at]
		c.Nodes = append(c.Nodes[:at], c.Nodes[at+1:]...)
		c.Nodes = append(c.Nodes, ChainNode{})
		copy(c.Nodes[to+1:], c.Nodes[to:])
		c.Nodes[to] = n
	case "chain_toggle":
		if at < 0 {
			return errors.New("模块不存在")
		}
		c.Nodes[at].Enabled = req.Enabled
	case "chain_param":
		if at < 0 {
			return errors.New("模块不存在")
		}
		f, _ := e.module(c.Nodes[at].Type)
		found := false
		for i, p := range f.Params {
			if p.Key == req.Key && finiteRange(req.Value, p.Min, p.Max) {
				c.Nodes[at].Params[i] = req.Value
				found = true
			}
		}
		if !found {
			return errors.New("无效参数")
		}
	case "chain_reset":
		if at < 0 {
			return errors.New("模块不存在")
		}
		f, _ := e.module(c.Nodes[at].Type)
		c.Nodes[at].Params = [12]float64{}
		for i, p := range f.Params {
			c.Nodes[at].Params[i] = p.Default
		}
	case "chain_apply":
		if err := e.applyChain(req.Mode, c); err != nil {
			return err
		}
		out["console"] = e.consoleStatus(req.Mode)
		return nil
	case "chain_legacy":
		m := e.masks()
		m[req.Mode] = 0
		if err := e.saveMasks(m); err != nil {
			return err
		}
		out["console"] = e.consoleStatus(req.Mode)
		return nil
	default:
		return errors.New("unknown chain operation")
	}
	c.Revision++
	if c.Revision == 0 {
		c.Revision = 1
	}
	if err := e.saveDraft(req.Mode, c); err != nil {
		return err
	}
	out["draft"] = c
	return nil
}
