#!/bin/sh
# Independent diagnostic process. No LD_PRELOAD, audio reads, watchdog changes,
# mounting, or application restarts. Writes only to an opted-in, mounted TF card.
unset LD_PRELOAD
umask 077
disc_lock=/tmp/disc-tf-log.lock
mkdir "$disc_lock" 2>/dev/null || exit 0
trap 'rmdir "$disc_lock" 2>/dev/null' EXIT
disc_sd=/tmp/sdcard
disc_ram=/tmp/disc-tf-log-early.txt
disc_prev='';disc_tick=0;disc_saved=0
disc_boot=$(cat /proc/sys/kernel/random/boot_id 2>/dev/null)
case "$disc_boot" in ''|*[!a-fA-F0-9-]*) exit 0;; esac
disc_uptime(){ read -r disc_up disc_rest < /proc/uptime; printf '%s' "$disc_up"; }
disc_state(){
 disc_ui=$(pidof mq_ui 2>/dev/null);disc_player=$(pidof mq_player 2>/dev/null)
 printf 'ui=%s player=%s' "${disc_ui:-none}" "${disc_player:-none}"
}
disc_card_ready(){
 [ -d "$disc_sd" ] && [ ! -L "$disc_sd" ] || return 1
 awk -v target="$disc_sd" '$2==target && $4 ~ /(^|,)rw(,|$)/ {ok=1} END {exit !ok}' /proc/mounts || return 1
 [ -f "$disc_sd/DISC_LOG_ENABLE" ] && [ ! -L "$disc_sd/DISC_LOG_ENABLE" ]
}
disc_append(){
 [ -f "$disc_events" ] && [ ! -L "$disc_events" ] || return 1
 disc_size=$(wc -c < "$disc_events")
 [ "$disc_size" -lt 262144 ] || return 1
 printf 'uptime=%s %s\n' "$(disc_uptime)" "$*" >> "$disc_events"
}
disc_snapshot(){
 {
  printf 'boot_id=%s uptime=%s\n' "$disc_boot" "$(disc_uptime)"
  disc_state; printf '\n'
  cat /proc/meminfo
  printf '\n== load ==\n';cat /proc/loadavg
  printf '\n== mounts ==\n';cat /proc/mounts
 } > "$disc_dir/system-latest.txt"
 dmesg 2>/dev/null | tail -c 524288 > "$disc_dir/kernel-latest.txt"
 # This file is written by the original stock supervisor; copy, never clear.
 if [ -f /usr/data/fiio/log/process_failed.txt ]; then
  tail -c 131072 /usr/data/fiio/log/process_failed.txt > "$disc_dir/process_failed.txt"
 fi
 # Only log text, never databases, credentials or all of userdata.
 disc_copied=0
 for disc_file in /usr/data/fiio/log/*; do
  [ -f "$disc_file" ] && [ ! -L "$disc_file" ] || continue
  disc_name=${disc_file##*/}
  case "$disc_name" in *ui*.log|*player*.log|*ui*.txt|*player*.txt)
   case "$disc_name" in *[!a-zA-Z0-9_.-]*) continue;; esac
   [ "$disc_copied" -lt 8 ] || break
   tail -c 131072 "$disc_file" > "$disc_dir/stock-$disc_name"
   disc_copied=$((disc_copied+1));;
  esac
 done
}
printf 'logger=disc-tf-log-v1 boot_id=%s started_uptime=%s\n' "$disc_boot" "$(disc_uptime)" > "$disc_ram"
# Preserve early process transitions in RAM while the original firmware mounts TF.
# With no card/marker, stop after two minutes and do not write to persistent storage.
while [ "$disc_tick" -lt 60 ]; do
 disc_current=$(disc_state)
 if [ "$disc_current" != "$disc_prev" ]; then
  printf 'uptime=%s %s\n' "$(disc_uptime)" "$disc_current" >> "$disc_ram"
  disc_prev=$disc_current
 fi
 if disc_card_ready; then disc_saved=1;break;fi
 disc_tick=$((disc_tick+1));sleep 2
done
[ "$disc_saved" -eq 1 ] || exit 0
disc_root="$disc_sd/DISC_LOGS"
[ ! -L "$disc_root" ] || exit 0
mkdir -p "$disc_root" || exit 0
# Stop instead of deleting user logs. Keep at most ten boot sessions until reviewed.
disc_dirs=0
for disc_old in "$disc_root"/*;do [ -d "$disc_old" ] && disc_dirs=$((disc_dirs+1));done
[ "$disc_dirs" -lt 10 ] || exit 0
disc_dir="$disc_root/$disc_boot"
mkdir "$disc_dir" || exit 0
disc_events="$disc_dir/events.txt"
cp "$disc_ram" "$disc_events" || exit 0
disc_append "TF_LOG_STARTED wall_clock=$(date '+%Y-%m-%dT%H:%M:%S%z')"
disc_snapshot;sync
disc_tick=0
while disc_card_ready;do
 disc_current=$(disc_state)
 if [ "$disc_current" != "$disc_prev" ];then
  disc_append "PROCESS_CHANGED $disc_current" || break
  disc_prev=$disc_current
  disc_snapshot;sync
 fi
 if [ $((disc_tick%5)) -eq 0 ];then
  disc_append "HEARTBEAT $disc_current" || break
  disc_snapshot;sync
 fi
 disc_tick=$((disc_tick+1));sleep 2
done
exit 0
