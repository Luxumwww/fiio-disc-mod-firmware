package main

import (
	"context"
	"errors"
	"io"
	"net"
	"syscall"
)

// Native SetURI can close an in-flight HTTP connection while changing decoder
// state. Recheck the same URI before retrying Play; never restart a track that
// already started, and never retry a timeout (the command may still be running).
func rendererConnectionDropped(e error) bool {
	var timeout net.Error
	if errors.As(e, &timeout) && timeout.Timeout() {
		return false
	}
	return errors.Is(e, io.EOF) || errors.Is(e, io.ErrUnexpectedEOF) || errors.Is(e, syscall.ECONNREFUSED) || errors.Is(e, syscall.ECONNRESET)
}
func (s *server) recoverRendererPlay(ctx context.Context, control, uri string) (string, error) {
	next, e := s.readyControl(ctx)
	if e != nil {
		return control, e
	}
	info, e := soapFields(ctx, next, "GetMediaInfo")
	if e != nil {
		return control, e
	}
	if info["CurrentURI"] != uri {
		return control, errors.New("播放服务已切换，请重新选歌")
	}
	state, e := soapFields(ctx, next, "GetTransportInfo")
	if e != nil {
		return control, e
	}
	if state["CurrentTransportState"] == "PLAYING" {
		return next, nil
	}
	e = soap(ctx, next, "Play", "<Speed>1</Speed>")
	return next, e
}
