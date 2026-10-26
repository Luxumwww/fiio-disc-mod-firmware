#!/bin/sh
unset LD_PRELOAD
set -eu
test "$(cat /usr/data/disc-runtime/service/API)" = 6
sha256sum -c SHA256SUMS >/dev/null
echo "Generic radial stereo FFT spectrum ABI1. Requires BT1.3.32/UI1.10.32 host. No audio consumer, daemon or driver."
