#!/bin/sh
unset LD_PRELOAD
set -eu
sha256sum -c SHA256SUMS >/dev/null
test "$(uname -r)" = 4.4.94
grep -q "#236 " /proc/version
test "$(cat /usr/data/disc-settings/VERSION)" = 1.10.59
for m in disc-usbdac-dop disc-usbdac-d2p disc-usbdac-dsd-engine; do test -f /usr/data/disc-runtime/components/$m/disabled; done
echo "Independent USB DSD backend available."
