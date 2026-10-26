#!/bin/sh
unset LD_PRELOAD
set -eu
sha256sum -c SHA256SUMS >/dev/null
test "$(uname -r)" = 4.4.94
grep -q "#236 " /proc/version
test "$(cat /usr/data/disc-settings/VERSION)" = 1.10.53
test -f /usr/data/disc-runtime/components/disc-usbdac-dop/disabled
test -f /usr/data/disc-runtime/components/disc-usbdac-d2p/disabled
echo "USB DSD native output engine ready, no PCM conversion."
