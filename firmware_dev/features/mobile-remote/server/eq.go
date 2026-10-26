package main

import (
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"math"
	"net"
	"os"
	"strconv"
	"strings"
	"time"
)

type Band struct {
	Position  int     `json:"position"`
	Frequency int     `json:"frequency"`
	Gain      float64 `json:"gain"`
	Q         float64 `json:"qValue"`
	Kind      int     `json:"filterType"`
}
type Equalizer struct {
	Mode   int     `json:"mode"`
	Master float64 `json:"master"`
	Bands  []Band  `json:"bands"`
}
type fiio struct {
	conn      net.Conn
	localMode bool
}

func (f *fiio) close() {
	if f.conn != nil {
		f.conn.Close()
		f.conn = nil
	}
}
func (f *fiio) ensure() error {
	if f.localMode {
		return nil
	}
	if _, e := os.Stat("/tmp/disc-runtime/remote-eq.ready"); e == nil {
		f.localMode = true
		return nil
	}
	if f.conn != nil {
		return nil
	}
	c, e := net.DialTimeout("tcp4", "127.0.0.1:12100", 2*time.Second)
	if e != nil {
		return e
	}
	f.conn = c
	_, e = f.request("0599", "0000", "a599")
	return e
}
func (f *fiio) write(tag, payload string) error {
	if f.localMode {
		_, e := f.local(tag, payload, "")
		return e
	}
	if len(payload) > 4096 {
		return errors.New("command too large")
	}
	f.conn.SetDeadline(time.Now().Add(3 * time.Second))
	b := []byte(fmt.Sprintf("%s%04X%s", tag, len(payload)+8, payload))
	for len(b) > 0 {
		n, e := f.conn.Write(b)
		if e != nil {
			f.close()
			return e
		}
		b = b[n:]
	}
	return nil
}
func (f *fiio) request(tag, payload, expected string) (string, error) {
	if f.localMode {
		return f.local(tag, payload, expected)
	}
	if f.conn == nil {
		return "", errors.New("EQ service unavailable")
	}
	if e := f.write(tag, payload); e != nil {
		return "", e
	}
	deadline := time.Now().Add(3 * time.Second)
	for time.Now().Before(deadline) {
		h := make([]byte, 8)
		if _, e := io.ReadFull(f.conn, h); e != nil {
			f.close()
			return "", e
		}
		size, e := strconv.ParseUint(string(h[4:]), 16, 16)
		if e != nil || size < 8 || size > 32768 {
			f.close()
			return "", errors.New("invalid native reply")
		}
		b := make([]byte, int(size)-8)
		if _, e = io.ReadFull(f.conn, b); e != nil {
			f.close()
			return "", e
		}
		if strings.EqualFold(string(h[:4]), expected) {
			return string(b), nil
		}
	}
	return "", errors.New("native reply timeout")
}
func (f *fiio) get() (Equalizer, error) {
	var eq Equalizer
	if e := f.ensure(); e != nil {
		return eq, e
	}
	mode, e := f.request("0639", "", "a639")
	if e != nil {
		return eq, e
	}
	v, e := strconv.ParseInt(mode, 16, 32)
	if e != nil {
		return eq, e
	}
	eq.Mode = int(v)
	raw, e := f.request("0628", "", "a628")
	if e != nil {
		return eq, e
	}
	if len(raw) < 8 || !strings.HasPrefix(raw, "0000") {
		return eq, errors.New("invalid EQ envelope")
	}
	data, e := hex.DecodeString(raw[4:])
	if e != nil || len(data) < 2 {
		return eq, errors.New("invalid EQ bands")
	}
	first, last := int(data[0]), int(data[1])
	if first > last || last > 9 || len(data) != 2+(last-first+1)*7 {
		return eq, errors.New("invalid EQ range")
	}
	for i := first; i <= last; i++ {
		b := data[2+(i-first)*7:]
		eq.Bands = append(eq.Bands, Band{i, int(binary.BigEndian.Uint16(b[2:4])), float64(int16(binary.BigEndian.Uint16(b[:2]))) / 10, float64(binary.BigEndian.Uint16(b[4:6])) / 100, int(b[6])})
	}
	master, e := f.request("0629", "", "a629")
	if e != nil {
		return eq, e
	}
	value, e := strconv.ParseUint(master, 16, 16)
	if e != nil {
		return eq, e
	}
	eq.Master = float64(int16(value)) / 10
	return eq, nil
}
func eqModeValid(mode int) bool {
	return mode == 255 || mode >= 160 && mode <= 169 || mode >= 0 && mode <= 10 && mode != 7
}
func (f *fiio) selectMode(mode int) error {
	if !eqModeValid(mode) {
		return errors.New("unknown EQ preset")
	}
	if e := f.ensure(); e != nil {
		return e
	}
	if e := f.write("0690", fmt.Sprintf("%04X", mode)); e != nil {
		return e
	}
	time.Sleep(100 * time.Millisecond)
	eq, e := f.get()
	if e == nil && eq.Mode != mode {
		e = errors.New("EQ preset did not apply")
	}
	return e
}
func (f *fiio) setBand(expected int, b Band) error {
	if expected < 160 || expected > 169 || b.Position < 0 || b.Position > 9 || b.Frequency < 20 || b.Frequency > 20000 || !finiteRange(b.Gain, -24, 12) || !finiteRange(b.Q, .1, 20) || b.Kind != 0 {
		return errors.New("invalid custom EQ band")
	}
	eq, e := f.get()
	if e != nil {
		return e
	}
	if eq.Mode != expected {
		return errors.New("EQ preset changed; refresh first")
	}
	payload, _ := json.Marshal([]map[string]any{{"position": b.Position, "frequency": b.Frequency, "gain": strconv.FormatFloat(b.Gain, 'f', 1, 64), "qValue": strconv.FormatFloat(b.Q, 'f', 2, 64), "filterType": 0}})
	if e = f.write("0678", "0001"+string(payload)); e != nil {
		return e
	}
	time.Sleep(100 * time.Millisecond)
	after, e := f.get()
	if e != nil {
		return e
	}
	for _, v := range after.Bands {
		if v.Position == b.Position {
			if v.Frequency != b.Frequency || abs(v.Gain-b.Gain) > .051 || abs(v.Q-b.Q) > .011 {
				return errors.New("EQ band did not apply")
			}
			return nil
		}
	}
	return errors.New("band missing after update")
}
func (f *fiio) setMaster(expected int, value float64) error {
	if expected < 160 || expected > 169 || !finiteRange(value, -24, 12) {
		return errors.New("select custom EQ before changing gain")
	}
	eq, e := f.get()
	if e != nil {
		return e
	}
	if eq.Mode != expected {
		return errors.New("EQ preset changed; refresh first")
	}
	if e = f.write("0630", fmt.Sprintf("%04X", uint16(int16(math.Round(value*10))))); e != nil {
		return e
	}
	time.Sleep(100 * time.Millisecond)
	after, e := f.get()
	if e == nil && abs(after.Master-value) > .11 {
		e = errors.New("EQ gain did not apply")
	}
	return e
}

func (f *fiio) resetGains(expected int) error {
	if expected < 160 || expected > 169 {
		return errors.New("select USER before reset")
	}
	eq, err := f.get()
	if err != nil {
		return err
	}
	if eq.Mode != expected || len(eq.Bands) != 10 {
		return errors.New("EQ changed or incomplete")
	}
	for _, b := range eq.Bands {
		b.Gain = 0
		if err = f.setBand(expected, b); err != nil {
			return err
		}
	}
	return nil
}
