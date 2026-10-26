package main

import "strings"

func publicOnlineError(msg string) string {
	lower := strings.ToLower(msg)
	if strings.Contains(lower, "http://") || strings.Contains(lower, "https://") || strings.Contains(lower, "dial tcp") || strings.Contains(lower, "timeout") || strings.Contains(lower, "deadline exceeded") {
		return "连接暂时失败，请稍后重试"
	}
	return clean(msg)
}
