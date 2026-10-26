package main

import (
	"crypto/aes"
	"encoding/hex"
	"encoding/json"
	"github.com/guohuiyuan/music-lib/model"
	"github.com/guohuiyuan/music-lib/netease"
	"github.com/guohuiyuan/music-lib/utils"
	"io"
	"net/http"
	"net/url"
	"strings"
	"testing"
)

type authenticatedQualityTransport struct {
	calls       []string
	validHeader bool
}

func (p *authenticatedQualityTransport) RoundTrip(r *http.Request) (*http.Response, error) {
	p.calls = append(p.calls, r.URL.Path)
	body := `{"code":403,"data":[]}`
	if strings.Contains(r.URL.Path, "player/url/v1") {
		encoded, _ := io.ReadAll(r.Body)
		form, _ := url.ParseQuery(string(encoded))
		encrypted, _ := hex.DecodeString(form.Get("params"))
		block, _ := aes.NewCipher([]byte("e82ckenh8dichen8"))
		plain := make([]byte, len(encrypted))
		if len(encrypted) > 0 && len(encrypted)%16 == 0 {
			for i := 0; i < len(encrypted); i += 16 {
				block.Decrypt(plain[i:i+16], encrypted[i:i+16])
			}
		}
		parts := strings.Split(string(plain), "-36cd479b6b5-")
		if len(parts) == 3 {
			var data struct {
				IDs    string            `json:"ids"`
				Header map[string]string `json:"header"`
			}
			if json.Unmarshal([]byte(parts[1]), &data) == nil {
				p.validHeader = data.IDs == "[3362629877]" && data.Header["MUSIC_U"] == "test-auth-token" && data.Header["__csrf"] == "test-csrf" && data.Header["appver"] != ""
			}
		}
		body = `{"code":200,"data":[{"code":200,"url":"https://audio.example/track.flac","type":"flac"}]}`
	}
	return &http.Response{StatusCode: 200, Header: make(http.Header), Body: io.NopCloser(strings.NewReader(body))}, nil
}
func TestAuthenticatedNeteaseHighQualityLargeIDWithoutVIPPrecheck(t *testing.T) {
	transport := &authenticatedQualityTransport{}
	utils.ConfigureEmbeddedHTTP(&http.Client{Transport: transport}, 2<<20)
	song := model.Song{Source: "netease", ID: "3362629877", Extra: map[string]string{"netease_level": "hires"}}
	u, e := netease.New("MUSIC_U=test-auth-token; __csrf=test-csrf").GetDownloadURL(&song)
	if e != nil || !transport.validHeader || !strings.HasSuffix(u, ".flac") || song.Ext != "flac" || len(transport.calls) != 1 || !strings.Contains(transport.calls[0], "player/url/v1") {
		t.Fatal("account/32bit precheck incorrectly downgraded quality", u, e, transport.calls)
	}
}
