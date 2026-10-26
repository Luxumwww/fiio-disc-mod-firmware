#!/bin/sh
unset LD_PRELOAD
umask 077
data=${DISC_PERSONAL_DATA:-/usr/data}
ram=${DISC_PERSONAL_RAM:-/tmp/disc-runtime}
service=${DISC_PERSONAL_SERVICE:-$data/disc-runtime/service}
catalog=$data/disc-runtime/wifi/catalog.conf
conf=$data/wpa_supplicant.conf
arg=$1;hash=${arg%-*};index=${arg##*-}
[ "$(sha256sum "$catalog" | cut -d' ' -f1)" = "$hash" ] || exit 3
new=$data/.disc-wifi-connect.new
before=$ram/wifi-connect-before
helper(){ LD_PRELOAD="$service/libdisc-wifi-config.so" /bin/true "$@"; }
helper single "$catalog" "$index" "$conf" "$new" || exit 4
cp "$conf" "$before" || exit 4
orig=$(sha256sum "$conf" | cut -d' ' -f1)
staged=$(sha256sum "$new" | cut -d' ' -f1)
changed=0
cleanup(){
 if [ "$changed" = 1 ] && [ "$(sha256sum "$conf" | cut -d' ' -f1)" = "$staged" ];then
  cp "$before" "$new" && chmod 600 "$new" && mv "$new" "$conf"
  wpa_cli -i wlan0 reconfigure >/dev/null 2>&1
  wpa_cli -i wlan0 reconnect >/dev/null 2>&1
 fi
 rm -f "$new" "$before"
}
trap cleanup EXIT
trap 'exit 6' HUP INT TERM
if [ "${DISC_PERSONAL_TEST:-0}" != 1 ];then
 : > /usr/data/disc-settings.wifi-enable
 n=0
 until wpa_cli -i wlan0 status 2>/dev/null | grep -q '^wpa_state=';do
  n=$((n+1));[ "$n" -lt 20 ] || exit 6;sleep 1
 done
fi
[ "$(sha256sum "$conf" | cut -d' ' -f1)" = "$orig" ] || exit 3
[ "$(sha256sum "$catalog" | cut -d' ' -f1)" = "$hash" ] || exit 3
chmod 600 "$new";mv "$new" "$conf" || exit 4;changed=1;sync
if [ "${DISC_PERSONAL_TEST:-0}" = 1 ];then changed=0;exit 0;fi
wpa_cli -i wlan0 reconfigure 2>/dev/null | grep -qx OK || exit 6
wpa_cli -i wlan0 select_network 0 2>/dev/null | grep -qx OK || exit 6
wpa_cli -i wlan0 reconnect >/dev/null 2>&1
n=0
until wpa_cli -i wlan0 status 2>/dev/null | grep -qx 'wpa_state=COMPLETED';do
 n=$((n+1));[ "$n" -lt 35 ] || exit 6;sleep 1
done
# Publish only after selecting this single saved network completed successfully.
changed=0
wpa_cli -i wlan0 save_config >/dev/null 2>&1
