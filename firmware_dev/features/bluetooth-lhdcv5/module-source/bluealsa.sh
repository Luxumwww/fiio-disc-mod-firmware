#!/bin/sh
unset LD_PRELOAD
root=/usr/data/disc-runtime/components/disc-bt-lhdcv5
original=/tmp/disc-runtime/lhdcv5-original/bluealsa
release=$(readlink -f "$root/current")
# The service entry keeps a read-only bind alias to the original ROM program.
# Disabled/safe/missing/invalid extension means ordinary stock BlueALSA.
if [ ! -f "$root/enabled" ] || [ -f "$root/disabled" ] || [ -f /tmp/sdcard/DISC_MODULES_SAFE_MODE ];then
 exec "$original" "$@"
fi
case "$release" in /usr/data/disc-runtime/components/disc-bt-lhdcv5/releases/*) :;;*) exec "$original" "$@";;esac
if ! (cd "$release" && sha256sum -c launch.SHA256SUMS >/dev/null 2>&1);then
 echo 'LHDCV5 extension checksum failed; using stock BlueALSA' > /tmp/disc-runtime/lhdcv5-launch.log
 exec "$original" "$@"
fi
ready='';[ ! -f /tmp/disc-runtime/lhdcv5-ready ] || read -r ready < /tmp/disc-runtime/lhdcv5-ready
if [ "$ready" != "$release" ];then
 if ! DISC_LHDCV5_SELFTEST=1 LD_PRELOAD="$release/libdisc-lhdcv5-stock.so" "$original" --version > /tmp/disc-runtime/lhdcv5-launch.log 2>&1;then
  echo 'LHDCV5 extension check failed; using stock BlueALSA' >> /tmp/disc-runtime/lhdcv5-launch.log
  exec "$original" "$@"
 fi
fi
export LD_PRELOAD="$release/libdisc-lhdcv5-stock.so"
exec "$original" "$@" --codec=LHDC-V5
