#!/bin/sh
unset LD_PRELOAD
umask 077
data=${DISC_PERSONAL_DATA:-/usr/data}
ram=${DISC_PERSONAL_RAM:-/tmp/disc-runtime}
service=${DISC_PERSONAL_SERVICE:-$data/disc-runtime/service}
catalog=$data/disc-runtime/wifi/catalog.conf
conf=$data/wpa_supplicant.conf
pending=$ram/wifi-capture
mkdir -p "$ram" "$data/disc-runtime/wifi"
chmod 700 "$data/disc-runtime/wifi"
[ ! -e "$ram/personal-busy" ] || exit 0
mkdir "$ram/wifi-catalog-lock" 2>/dev/null || exit 0
trap 'rm -f "$catalog.new";rmdir "$ram/wifi-catalog-lock" 2>/dev/null' EXIT
helper(){ LD_PRELOAD="$service/libdisc-wifi-config.so" /bin/true "$@"; }
if [ -f "$pending" ];then
 born=$(cat "$ram/wifi-capture.at" 2>/dev/null)
 case "$born" in ''|*[!0-9]*) born=0;;esac
 [ $(( $(cut -d. -f1 /proc/uptime) - born )) -le 90 ] || rm -f "$pending" "$ram/wifi-capture.at"
fi
[ -f "$conf" ] || exit 0
if [ "${DISC_PERSONAL_TEST:-0}" != 1 ] && [ -f "$catalog" ];then
 wpa_cli -i wlan0 status 2>/dev/null | grep -qx 'wpa_state=COMPLETED' || exit 0
fi
hash=$(sha256sum "$conf" | cut -d' ' -f1)
matched=0
if [ -f "$pending" ] && helper verify "$conf" "$pending";then matched=1;fi
if [ "$matched" = 0 ] && [ -f "$catalog" ] && [ "$(cat "$ram/wifi-import.hash" 2>/dev/null)" = "$hash" ];then exit 0;fi
if helper merge "$catalog" "$conf" "$pending" "$catalog.new";then
 if [ -f "$catalog" ] && cmp -s "$catalog.new" "$catalog";then rm -f "$catalog.new";
 else chmod 600 "$catalog.new";mv "$catalog.new" "$catalog";sync;fi
 printf '%s\n' "$hash" > "$ram/wifi-import.hash"
 [ "$matched" = 0 ] || rm -f "$pending" "$ram/wifi-capture.at"
else
 code=$?;[ "$code" != 7 ] || printf '已保存 20 个网络，请先删除不用的网络\n' > "$ram/wifi-catalog-error"
fi
