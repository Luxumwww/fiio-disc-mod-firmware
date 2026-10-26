#!/bin/sh
unset LD_PRELOAD
set -eu
sha256sum -c SHA256SUMS >/dev/null
test "$(uname -r)" = 4.4.94
grep -q "#236 " /proc/version
echo "USB DAC classified DSD64/128 DoP, shared host UI1.10.42. No resident process."
