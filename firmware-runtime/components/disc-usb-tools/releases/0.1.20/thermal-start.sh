#!/bin/sh
unset LD_PRELOAD
case "$1:$2" in ''|*[!0-9:]*|:*|*:) exit 2;;esac
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P) || exit 2
case "$here" in /usr/data/disc-runtime/components/disc-usb-tools/releases/*) :;;*) exit 2;;esac
export DISC_THERMAL_OWNER=$1 DISC_THERMAL_TOKEN=$2
export LD_PRELOAD=$here/libthermal-worker.so
exec /bin/true
