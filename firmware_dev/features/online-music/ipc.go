package main

import "strings"

// Remove line terminators, never an empty final argument such as control/next.
func parseRequest(b []byte) []string {
	return strings.SplitN(strings.TrimRight(string(b), "\r\n"), "\t", 4)
}
