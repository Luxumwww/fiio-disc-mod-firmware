#!/bin/sh
unset LD_PRELOAD
set -eu
test "$(cat /usr/data/disc-runtime/service/API)" = 6
sha256sum -c SHA256SUMS >/dev/null
exec sh ./record.sh
