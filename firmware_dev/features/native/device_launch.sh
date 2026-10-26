#!/bin/sh
# Scoped device startup. Called by the original application supervisor.
set -u
disc_role=${1:-}
case "$disc_role" in
 ui) disc_program=/usr/bin/mq_ui; disc_library=/usr/lib/libdisc-spectrum-ui.so ;;
 player) disc_program=/usr/bin/mq_player; disc_library=/usr/lib/libdisc-pcm-tap.so ;;
 *) exit 2 ;;
esac
disc_stock(){ unset LD_PRELOAD; exec "$disc_program"; }
if [ -f /usr/data/fiio/disc-spectrum.disable ] ||
   [ -f /tmp/sdcard/disc-spectrum-disable ] ||
   [ -f /tmp/disc-spectrum-fallback ]; then disc_stock; fi
disc_ui_hash=$(sha256sum /usr/bin/mq_ui 2>/dev/null);disc_ui_hash=${disc_ui_hash%% *}
disc_player_hash=$(sha256sum /usr/bin/mq_player 2>/dev/null);disc_player_hash=${disc_player_hash%% *}
if [ "$disc_ui_hash" != f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226 ] ||
   [ "$disc_player_hash" != a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc ]; then disc_stock; fi
if ! sha256sum -c /usr/project/disc-spectrum/checksums.sha256 >/dev/null 2>&1; then disc_stock; fi
# Three launches of the same role in under 60s indicate an unstable start. The
# existing stock supervisor kills/restarts both applications, so the next pair
# runs the original programs without extensions for the remainder of this boot.
disc_now=0;disc_unused=0
read -r disc_now disc_unused < /proc/uptime || disc_now=0
disc_now=${disc_now%%.*}
case "$disc_now" in ''|*[!0-9]*) disc_stock;; esac
disc_stamp=/tmp/disc-spectrum-attempt-$disc_role
disc_prev=0;disc_count=0
if [ -f "$disc_stamp" ]; then read -r disc_prev disc_count < "$disc_stamp" || disc_count=0; fi
case "$disc_prev:$disc_count" in *[!0-9:]*|:*) disc_prev=0;disc_count=0;; esac
if [ "$disc_now" -lt "$disc_prev" ] || [ $((disc_now-disc_prev)) -ge 60 ]; then disc_count=0; fi
disc_count=$((disc_count+1))
if [ "$disc_count" -ge 3 ]; then
 : > /tmp/disc-spectrum-fallback
 echo 'DISC spectrum: restart fallback to stock applications' >&2
 disc_stock
fi
printf '%s %s\n' "$disc_now" "$disc_count" > "$disc_stamp" || disc_stock
printf 'V2.57\n' > /usr/data/fiio/disc-spectrum.activate || disc_stock
export LD_PRELOAD="$disc_library"
exec "$disc_program"
