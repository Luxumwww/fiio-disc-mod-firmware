#!/bin/sh
# One-shot backend fallback after the native ON message did not raise wlan0.
unset LD_PRELOAD
set -eu
root=/usr/data/disc-runtime
ram=/tmp/disc-runtime
pid=${1:-}
case "$pid" in ''|*[!0-9]*) exit 2;;esac
[ "$(pidof mq_ui)" = "$pid" ] || exit 0
mode=$("$root/service/ui-mode" "$pid")
case "$mode" in 0|1|2|4) :;;*) exit 0;;esac
[ -f "$root/components/disc-bt-meter/enabled" ] && [ ! -f "$root/components/disc-bt-meter/disabled" ] || exit 0
read -r tag active restore pending manual attempts < "$root/module-data/disc-bt-meter/bt-wifi-policy"
[ "$tag:$active:$pending:$manual" = BT-WIFI-1:0:2:0 ] || exit 0
mkdir "$ram/bt-wifi-restore.lock" 2>/dev/null || exit 0
trap 'rmdir "$ram/bt-wifi-restore.lock" 2>/dev/null || true' EXIT
# Never duplicate an existing supplicant. Stock script owns rfkill/interface,
# WPA and DHCP; settings/UI were already synchronized by the native ON path.
[ -z "$(pidof wpa_supplicant 2>/dev/null || true)" ] || exit 0
[ "$(pidof mq_ui)" = "$pid" ] || exit 0
mode=$("$root/service/ui-mode" "$pid")
[ "$mode" != 3 ] || exit 0
exec_script=/usr/bin/wifi_up.sh
[ -x "$exec_script" ] || exit 1
sh "$exec_script" > "$ram/bt-wifi-restore.log" 2>&1
# A quick return to BT may race the stock script's asynchronous startup.
# Undo only our late startup when automatic OFF still owns the radio intent.
if [ "$(pidof mq_ui)" = "$pid" ];then
 mode=$("$root/service/ui-mode" "$pid")
 read -r tag active restore pending manual attempts < "$root/module-data/disc-bt-meter/bt-wifi-policy"
 if [ "$mode:$tag:$active:$manual" = 3:BT-WIFI-1:1:0 ];then
  sh /usr/bin/wifi_down.sh >> "$ram/bt-wifi-restore.log" 2>&1
 fi
fi
