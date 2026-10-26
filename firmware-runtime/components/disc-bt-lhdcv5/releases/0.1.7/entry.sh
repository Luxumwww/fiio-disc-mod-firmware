#!/bin/sh
unset LD_PRELOAD
set -eu
root=/usr/data/disc-runtime/components/disc-bt-lhdcv5
here=$(readlink -f "$root/current")
original_dir=/tmp/disc-runtime/lhdcv5-original
original=$original_dir/bluealsa
stock_sha=773320d3c61f35ece83becdada68e7417068e81dbd48b22bedc63a985404dd9d
case "$here" in "$root"/releases/*) :;;*) exit 2;;esac
cd "$here"
mkdir -p /tmp/disc-runtime
wrapper_sha=$(sha256sum bluealsa.sh);wrapper_sha=${wrapper_sha%% *}
ready='';[ ! -f /tmp/disc-runtime/lhdcv5-ready ] || read -r ready < /tmp/disc-runtime/lhdcv5-ready
if [ "${1:-}" = --prepare ] && [ "$ready" = "$here" ] && [ "$(sha256sum /usr/bin/bluealsa | cut -d' ' -f1)" = "$wrapper_sha" ];then exit 0;fi
lock=/tmp/disc-runtime/lhdcv5-prepare.lock
n=0
while ! mkdir "$lock" 2>/dev/null;do
 n=$((n+1));[ "$n" -lt 100 ] || exit 5
 usleep 10000
done
trap 'rmdir "$lock" 2>/dev/null || true' EXIT
trap 'exit 130' TERM INT
sha256sum -c SHA256SUMS >/dev/null
chmod 755 bluealsa.sh
mounted(){ awk -v path="$1" '$2==path {found=1} END {exit !found}' /proc/mounts; }
if ! mounted "$original";then
 current_sha=$(sha256sum /usr/bin/bluealsa);current_sha=${current_sha%% *}
 [ "$current_sha" = "$stock_sha" ] || { echo 'Original BlueALSA guard failed';exit 4; }
 mkdir -p "$original_dir"
 : > "$original"
 chmod 755 "$original"
 mount -o bind /usr/bin/bluealsa "$original"
fi
original_sha=$(sha256sum "$original");original_sha=${original_sha%% *}
[ "$original_sha" = "$stock_sha" ] || { echo 'Original alias guard failed';exit 4; }
if [ "$ready" != "$here" ];then
 DISC_LHDCV5_SELFTEST=1 LD_PRELOAD="$here/libdisc-lhdcv5-stock.so" "$original" --version > /tmp/disc-runtime/lhdcv5-install-check.log 2>&1
fi
current_sha=$(sha256sum /usr/bin/bluealsa);current_sha=${current_sha%% *}
if [ "$current_sha" != "$wrapper_sha" ];then
 [ "$current_sha" = "$stock_sha" ] || { echo 'BlueALSA launch path guard failed';exit 4; }
 mount -o bind "$here/bluealsa.sh" /usr/bin/bluealsa
fi
printf '%s\n' "$here" > /tmp/disc-runtime/lhdcv5-ready.next
mv /tmp/disc-runtime/lhdcv5-ready.next /tmp/disc-runtime/lhdcv5-ready
rmdir "$lock"
trap - EXIT TERM INT
if [ "${1:-}" = --prepare ];then echo 'LHDC V5 launch ready before native sink startup';exit 0;fi
cleanup(){
 trap - EXIT TERM INT
 current_sha=$(sha256sum /usr/bin/bluealsa);current_sha=${current_sha%% *}
 if [ "$current_sha" = "$wrapper_sha" ] && mounted /usr/bin/bluealsa;then umount /usr/bin/bluealsa || true;fi
 # The ROM alias can still back a playing process. Keep it until shutdown;
 # unbinding the wrapper restores the ordinary next-launch behavior.
 exit 0
}
trap cleanup EXIT TERM INT
echo 'LHDC V5 0.1.7 launch integration installed; existing audio process unchanged'
while [ -f "$root/enabled" ] && [ ! -f "$root/disabled" ];do sleep 30;done
