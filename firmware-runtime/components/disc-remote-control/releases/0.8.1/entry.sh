#!/bin/sh
unset LD_PRELOAD
set -eu
umask 077
cd "$(dirname "$0")"
sha256sum -c SHA256SUMS >/dev/null
chmod 700 ./disc-remote-control
exec ./disc-remote-control
