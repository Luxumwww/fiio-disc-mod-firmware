package main

import (
	"bytes"
	"encoding/base64"
	"errors"
	"image"
	_ "image/png"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/guohuiyuan/music-lib/kugou"
	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/netease"
	"github.com/guohuiyuan/music-lib/provider"
	"github.com/guohuiyuan/music-lib/qq"
	qrcode "github.com/skip2/go-qrcode"
)

const qrSize = 196

func (s *server) loadCredentials(dir string) {
	s.credentials = dir
	os.MkdirAll(dir, 0700)
	for source := range s.providers {
		b, _ := os.ReadFile(filepath.Join(dir, source+".cookie"))
		if len(b) > 16384 {
			continue
		}
		cookie := strings.TrimSpace(string(b))
		switch source {
		case "netease":
			s.providers[source] = netease.New(cookie)
		case "qq":
			s.providers[source] = qq.New(cookie)
		case "kugou":
			s.providers[source] = kugou.New(cookie)
		}
	}
}
func qrPixels(session *model.QRLoginSession, client *http.Client) ([]byte, error) {
	var img image.Image
	if session.URL != "" {
		qr, e := qrcode.New(session.URL, qrcode.Medium)
		if e != nil {
			return nil, e
		}
		img = qr.Image(qrSize)
	} else {
		var data []byte
		if strings.HasPrefix(session.ImageURL, "data:image/png;base64,") {
			b, e := base64.StdEncoding.DecodeString(strings.TrimPrefix(session.ImageURL, "data:image/png;base64,"))
			if e != nil {
				return nil, e
			}
			data = b
		} else if strings.HasPrefix(session.ImageURL, "https://") {
			resp, e := client.Get(session.ImageURL)
			if e != nil {
				return nil, e
			}
			defer resp.Body.Close()
			if resp.StatusCode != 200 {
				return nil, errors.New("二维码图片下载失败")
			}
			data, _ = io.ReadAll(io.LimitReader(resp.Body, (1<<20)+1))
		} else {
			return nil, errors.New("平台没有返回二维码")
		}
		if len(data) > 1<<20 {
			return nil, errors.New("二维码图片过大")
		}
		cfg, _, e := image.DecodeConfig(bytes.NewReader(data))
		if e != nil || cfg.Width < 1 || cfg.Height < 1 || cfg.Width > 1024 || cfg.Height > 1024 {
			return nil, errors.New("二维码图片格式无效")
		}
		img, _, e = image.Decode(bytes.NewReader(data))
		if e != nil {
			return nil, e
		}
	}
	bounds := img.Bounds()
	b := make([]byte, qrSize*qrSize*4)
	for y := 0; y < qrSize; y++ {
		for x := 0; x < qrSize; x++ {
			r, g, z, _ := img.At(bounds.Min.X+x*bounds.Dx()/qrSize, bounds.Min.Y+y*bounds.Dy()/qrSize).RGBA()
			i := (y*qrSize + x) * 4
			b[i] = byte(z >> 8)
			b[i+1] = byte(g >> 8)
			b[i+2] = byte(r >> 8)
			b[i+3] = 255
		}
	}
	return b, nil
}
func (s *server) startLogin(source string) error {
	p, ok := s.providers[source].(provider.QRLoginProvider)
	if !ok {
		return errors.New("该平台不支持扫码登录")
	}
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return errors.New("正在处理，请稍后")
	}
	session, e := p.CreateQRLogin()
	if e != nil {
		return e
	}
	pixels, e := qrPixels(session, s.client)
	if e != nil {
		return e
	}
	if e = atomicWrite(filepath.Join(s.dir, "login.bgra"), pixels); e != nil {
		return e
	}
	s.qr = session
	return nil
}
func (s *server) checkLogin(source string) (string, string, error) {
	if s.qr == nil || s.qr.Source != source {
		return "error", "", errors.New("请重新生成二维码")
	}
	if s.qr.ExpiresAt > 0 && time.Now().Unix() > s.qr.ExpiresAt {
		return "expired", "二维码已过期，请刷新", nil
	}
	p, ok := s.providers[source].(provider.QRLoginProvider)
	if !ok {
		return "error", "", errors.New("不支持扫码登录")
	}
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return "waiting", "正在处理，请稍后", nil
	}
	r, e := p.CheckQRLogin(s.qr.Key)
	if e != nil {
		return "error", "", e
	}
	switch r.Status {
	case model.QRLoginStatusSuccess:
		s.cancelPrefetch()
		if r.Cookie == "" || len(r.Cookie) > 16384 {
			return "error", "", errors.New("登录返回无效凭据")
		}
		if e = atomicWrite(filepath.Join(s.credentials, source+".cookie"), []byte(r.Cookie)); e != nil {
			return "error", "", errors.New("登录成功，但账号保存失败")
		}
		s.invalidateAccountPages(source)
		s.qr = nil
		return "logged-in", "登录成功，账号已保存", nil
	case model.QRLoginStatusScanned:
		return "waiting", "已扫码，请在手机确认", nil
	case model.QRLoginStatusWaiting:
		return "waiting", "用对应音乐应用扫码登录", nil
	case model.QRLoginStatusExpired:
		return "expired", "二维码已过期，请刷新", nil
	default:
		return "error", "", errors.New("平台拒绝登录，请刷新后重试")
	}
}
func (s *server) accountMessage(source string) string {
	b, _ := os.ReadFile(filepath.Join(s.credentials, source+".cookie"))
	if len(b) > 0 {
		return "已保存登录"
	}
	return "未登录"
}
func (s *server) logout(source string) error {
	s.cancelPrefetch()
	if s.providers[source] == nil {
		return errors.New("不支持的平台")
	}
	select {
	case s.gate <- struct{}{}:
		defer func() { <-s.gate }()
	default:
		return errors.New("正在处理，请稍后")
	}
	if e := os.Remove(filepath.Join(s.credentials, source+".cookie")); e != nil && !os.IsNotExist(e) {
		return e
	}
	switch source {
	case "netease":
		s.providers[source] = netease.New("")
	case "qq":
		s.providers[source] = qq.New("")
	case "kugou":
		s.providers[source] = kugou.New("")
	}
	s.invalidateAccountPages(source)
	s.qr = nil
	return nil
}
