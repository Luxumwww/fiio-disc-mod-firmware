#!/bin/sh
unset LD_PRELOAD DISC_TIME_TEST DISC_TIME_DATA DISC_TIME_RAM DISC_TIME_MODULE
set -eu
cd "$(dirname "$0")"
sha256sum -c SHA256SUMS >/dev/null
exec nice -n 15 sh ./time-sync.sh
