#!/bin/sh
unset LD_PRELOAD
id=$1
case "$id" in ''|*[!a-zA-Z0-9._-]*|.*|*..*) exit 2;;esac
dir=/usr/data/disc-runtime/components/$id
[ ! -f "$dir/disabled" ] || exit 3
release=$(readlink -f "$dir/current") || exit 1
case "$release" in
 /tmp/sdcard/DISC_MODULES/*)
  /usr/data/disc-runtime/service/bundle-check.sh "$release" || exit 4
  [ "$(cat "$release/MODULE_ID")" = "$id" ] || exit 2;;
 /usr/data/disc-runtime/components/$id/releases/*) :;;
 *) exit 2;;
esac
entry=$(cat "$dir/current/ENTRY")
case "$entry" in ''|/*|*..*|*[!a-zA-Z0-9_./-]*) exit 2;;esac
cd "$dir/current" || exit 1
DISC_MODULE_DATA=/usr/data/disc-runtime/module-data/$id
mkdir -p "$DISC_MODULE_DATA" || exit 1
export DISC_MODULE_DATA
exec "./$entry"
