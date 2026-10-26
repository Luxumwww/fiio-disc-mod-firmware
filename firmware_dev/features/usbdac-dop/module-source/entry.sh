#!/bin/sh
unset LD_PRELOAD
set -eu
sha256sum -c SHA256SUMS >/dev/null
test "$(uname -r)" = 4.4.94
grep -q "#236 " /proc/version
test "$(cat /usr/data/disc-settings/VERSION)" = 1.10.52
echo "USB DoP serial profile1.3.2 ready; no resident process."
