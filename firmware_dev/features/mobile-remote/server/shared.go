package main

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"
)

func (e *engine) sharedPath() string { return filepath.Join(e.dir, "shared-profile.conf") }
func (e *engine) sharedSupported() bool {
	b, _ := os.ReadFile(e.audioPath)
	return strings.Contains("\n"+string(b), "\nshared_supported=1\n")
}
func (e *engine) sharedSeedMode(fallback int) int {
	b, _ := os.ReadFile(e.audioPath)
	for _, line := range strings.Split(string(b), "\n") {
		var mode int
		if _, err := fmt.Sscanf(line, "work_mode=%d", &mode); err == nil {
			switch mode {
			case 1:
				return 3
			case 2:
				return 0
			case 4:
				return 1
			case 5:
				return 2
			}
		}
	}
	return fallback
}
func (e *engine) legacyChain(mode int) (Chain, error) {
	c := Chain{Nodes: []ChainNode{}}
	m := e.masks()[mode]
	if m&customMask != 0 {
		return e.activeChain(mode), nil
	}
	p := e.parameters(mode)
	for _, t := range []int{2, 0, 4, 3, 5, 6, 7, 8, 9, 10, 11} {
		if m&(1<<t) != 0 {
			if len(c.Nodes) == 8 {
				return c, errors.New("原配置超过8个音效，请先减少音效")
			}
			c.Nodes = append(c.Nodes, ChainNode{ID: uint32(len(c.Nodes) + 1), Type: t, Enabled: true, Params: p[t]})
		}
	}
	return c, nil
}
func (e *engine) sharedProfile(mode int) (Chain, bool, bool, error) {
	b, err := os.ReadFile(e.sharedPath())
	if err == nil {
		parts := strings.SplitN(string(b), "\n", 2)
		if len(parts) != 2 || (parts[0] != "DSP-SHARED-1 0" && parts[0] != "DSP-SHARED-1 1") {
			return Chain{}, false, true, errors.New("统一音效配置无效")
		}
		c, err := e.parseChain([]byte(parts[1]))
		c.Revision = hashText(b)
		return c, parts[0] == "DSP-SHARED-1 1", true, err
	}
	if !os.IsNotExist(err) {
		return Chain{}, false, false, err
	}
	mode = e.sharedSeedMode(mode)
	c, err := e.legacyChain(mode)
	c.Revision = 0
	return c, e.masks()[mode] != 0, false, err
}
func (e *engine) saveShared(c Chain, enabled bool) error {
	if err := chainCapacityError(c); err != nil {
		return err
	}
	if !e.validChain(c) {
		return errors.New("无效音效配置")
	}
	v := 0
	if enabled {
		v = 1
	}
	b := append([]byte(fmt.Sprintf("DSP-SHARED-1 %d\n", v)), chainText(c)...)
	return atomicFile(e.sharedPath(), b)
}
func (e *engine) sharedStatus(mode int) map[string]any {
	out := e.consoleStatus(mode)
	c, on, configured, err := e.sharedProfile(mode)
	out["scope"] = "all"
	out["shared_supported"] = e.sharedSupported()
	out["shared_configured"] = configured
	out["master"] = on
	out["draft"] = c
	out["active"] = c
	out["draft_hash"] = hashText(chainText(c))
	out["presets"] = presetSummaries(e.presets())
	if err != nil {
		out["profile_error"] = err.Error()
	}
	return out
}
func (e *engine) sharedOperation(req Request, out map[string]any) error {
	if req.Op == "console_status" {
		out["console"] = e.sharedStatus(req.Mode)
		return nil
	}
	if !e.sharedSupported() {
		return errors.New("播放器音效服务需要更新，请先完成播放器升级")
	}
	c, on, configured, err := e.sharedProfile(req.Mode)
	if err != nil {
		return err
	}
	if req.Op == "presets" {
		out["presets"] = presetSummaries(e.presets())
		return nil
	}
	if req.Op == "preset_save" {
		name := strings.TrimSpace(req.Name)
		if !validPresetName(name) {
			return errors.New("名称需为1至32个字")
		}
		items := e.presets()
		id := fmt.Sprintf("%x", time.Now().UnixNano())
		at := -1
		for i, p := range items {
			if p.Global && p.Name == name {
				id = p.ID
				at = i
				break
			}
		}
		p := preset{ID: id, Name: name, Mode: 3, Mask: customMask, Params: e.defaults(), Chain: &c, Global: true}
		if at >= 0 {
			items[at] = p
		} else {
			if len(items) == 24 {
				return errors.New("最多保存24个预设")
			}
			items = append(items, p)
		}
		if err = e.persistPresets(items); err != nil {
			return err
		}
		if !configured {
			if err = e.saveShared(c, on); err != nil {
				return err
			}
		}
		out["presets"] = presetSummaries(items)
		return nil
	}
	if req.Op == "preset_apply" {
		found := false
		for _, p := range e.presets() {
			if p.ID != req.Preset {
				continue
			}
			found = true
			if p.Chain != nil {
				c = *p.Chain
			} else {
				c = Chain{Nodes: []ChainNode{}}
				for _, t := range []int{2, 0, 4, 3, 5, 6, 7, 8, 9, 10, 11} {
					if p.Mask&(1<<t) != 0 {
						if len(c.Nodes) == 8 {
							return errors.New("预设超过8个音效")
						}
						c.Nodes = append(c.Nodes, ChainNode{ID: uint32(len(c.Nodes) + 1), Type: t, Enabled: true, Params: p.Params[t]})
					}
				}
			}
			break
		}
		if !found {
			return errors.New("预设不存在")
		}
		if err = e.saveShared(c, true); err != nil {
			return err
		}
		out["console"] = e.sharedStatus(req.Mode)
		return nil
	}
	if req.Revision != c.Revision {
		return errors.New("音效设置已变化，正在刷新")
	}
	c.Nodes = append([]ChainNode{}, c.Nodes...)
	at := -1
	id := uint32(1)
	for i, n := range c.Nodes {
		if n.ID == req.Node {
			at = i
		}
		if n.ID >= id {
			id = n.ID + 1
		}
	}
	switch req.Op {
	case "chain_master":
		on = req.Enabled
	case "chain_add", "chain_duplicate":
		if len(c.Nodes) >= chainSavedMax || id > 2147483647 {
			return errors.New("最多保存16个音效，请先移除一项")
		}
		var n ChainNode
		if req.Op == "chain_duplicate" {
			if at < 0 {
				return errors.New("音效不存在")
			}
			n = c.Nodes[at]
		} else {
			f, ok := e.module(req.Effect)
			if !ok {
				return errors.New("音效不存在")
			}
			n.Type = req.Effect
			n.Enabled = req.Enabled
			for j, p := range f.Params {
				n.Params[j] = p.Default
			}
		}
		n.ID = id
		c.Nodes = append(c.Nodes, n)
		if !configured && n.Enabled {
			on = true
		}
	case "chain_toggle":
		if at < 0 {
			return errors.New("音效不存在")
		}
		c.Nodes[at].Enabled = req.Enabled
		if req.Enabled {
			on = true
		}
	case "chain_param":
		if at < 0 {
			return errors.New("音效不存在")
		}
		f, _ := e.module(c.Nodes[at].Type)
		ok := false
		for j, p := range f.Params {
			if p.Key == req.Key && finiteRange(req.Value, p.Min, p.Max) {
				c.Nodes[at].Params[j] = req.Value
				ok = true
			}
		}
		if !ok {
			return errors.New("参数超出范围")
		}
	case "chain_reset":
		if at < 0 {
			return errors.New("音效不存在")
		}
		f, _ := e.module(c.Nodes[at].Type)
		c.Nodes[at].Params = [12]float64{}
		for j, p := range f.Params {
			c.Nodes[at].Params[j] = p.Default
		}
	case "chain_remove":
		if at < 0 {
			return errors.New("音效不存在")
		}
		c.Nodes = append(c.Nodes[:at], c.Nodes[at+1:]...)
	case "chain_move":
		to := int(req.Value)
		if at < 0 || req.Value != float64(to) || to < 0 || to >= len(c.Nodes) {
			return errors.New("无效顺序")
		}
		n := c.Nodes[at]
		c.Nodes = append(c.Nodes[:at], c.Nodes[at+1:]...)
		c.Nodes = append(c.Nodes, ChainNode{})
		copy(c.Nodes[to+1:], c.Nodes[to:])
		c.Nodes[to] = n
	case "chain_clear":
		c.Nodes = []ChainNode{}
	default:
		return errors.New("无效操作")
	}
	if err = e.saveShared(c, on); err != nil {
		return err
	}
	c, on, _, err = e.sharedProfile(req.Mode)
	if err != nil {
		return err
	}
	out["draft"] = c
	out["master"] = on
	out["saved_hash"] = hashText(chainText(c))
	return nil
}
