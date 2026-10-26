#!/bin/sh
unset LD_PRELOAD
set -eu
test "$(cat /usr/data/disc-runtime/service/API)" = 6
sha256sum -c SHA256SUMS >/dev/null
echo 'Generic stereo XY visualization ABI1,1.0.6. Requires compatible UI host1.10.34 with reviewed library hash; no audio consumer, daemon or driver.'
