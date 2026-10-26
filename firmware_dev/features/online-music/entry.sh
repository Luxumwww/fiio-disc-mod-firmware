#!/bin/sh
unset LD_PRELOAD
set -eu
umask 077
cd "$(dirname "$0")"
sha256sum -c SHA256SUMS >/dev/null
mkdir -p /tmp/disc-online-music
chmod 700 ./disc-online-music
exec ./disc-online-music -state /tmp/disc-online-music
