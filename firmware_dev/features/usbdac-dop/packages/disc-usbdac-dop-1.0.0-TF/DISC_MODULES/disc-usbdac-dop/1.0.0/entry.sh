#!/bin/sh
unset LD_PRELOAD
set -eu
sha256sum -c SHA256SUMS >/dev/null
test "$(uname -r)" = 4.4.94
grep -q "#236 " /proc/version
echo "USB DAC DSD128 DoP compatibility, shared UI1.10.36 host. No resident process."
