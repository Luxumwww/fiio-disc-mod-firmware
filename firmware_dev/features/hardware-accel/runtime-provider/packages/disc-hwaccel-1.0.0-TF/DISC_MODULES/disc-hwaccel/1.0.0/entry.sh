#!/bin/sh
unset LD_PRELOAD
set -eu
test "$(cat /usr/data/disc-runtime/service/API)" = 6
sha256sum -c SHA256SUMS >/dev/null
echo 'DPU provider registered. Requires compatible UI host (1.10.13+) and enabled disc-hwaccel. No daemon or kernel load.'
