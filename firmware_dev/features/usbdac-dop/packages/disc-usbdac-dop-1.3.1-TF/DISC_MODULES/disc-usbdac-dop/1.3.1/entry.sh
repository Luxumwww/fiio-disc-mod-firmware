#!/bin/sh
unset LD_PRELOAD
set -eu
sha256sum -c SHA256SUMS >/dev/null
test "$(uname -r)" = 4.4.94
grep -q "#236 " /proc/version
echo "USB DAC local DSD output reuse, host1.10.50, no resident process."
