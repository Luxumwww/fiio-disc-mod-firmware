#!/bin/sh
unset LD_PRELOAD
set -eu
test "$(cat /usr/data/disc-runtime/service/API)" = 6
sha256sum -c SHA256SUMS >/dev/null
echo 'Generic stereo XY visualization ABI1. Requires compatible UI host 1.10.14+; no audio consumer, daemon or driver.'
