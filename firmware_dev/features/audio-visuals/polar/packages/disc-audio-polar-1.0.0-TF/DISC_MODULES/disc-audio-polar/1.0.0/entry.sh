#!/bin/sh
unset LD_PRELOAD
set -eu
test "$(cat /usr/data/disc-runtime/service/API)" = 6
sha256sum -c SHA256SUMS >/dev/null
echo "Generic stereo polar waveform ABI1. Requires BT1.3.31/UI1.10.31 host. No separate audio consumer, daemon or driver."
