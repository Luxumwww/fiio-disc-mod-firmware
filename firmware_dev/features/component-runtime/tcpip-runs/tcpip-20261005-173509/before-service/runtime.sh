#!/bin/sh
unset LD_PRELOAD
umask 077
root=/usr/data/disc-runtime
service=$root/service
ram=/tmp/disc-runtime
. "$service/mode-policy.sh"
. "$service/debug-state.sh"
mkdir -p "$ram" || exit 1
# Recover an interrupted personal-data transaction before starting modules.
if [ -d /usr/data/.disc-personal-restore ];then "$service/personal-backup.sh" recover;fi
mkdir "$ram/lock" 2>/dev/null || exit 0
trap 'rmdir "$ram/lock" 2>/dev/null' EXIT
echo $$ > "$ram/manager.pid"
: > "$ram/runtime.log"
log() { [ "$(wc -c < "$ram/runtime.log")" -ge 262144 ] || printf '%s %s\n' "$(cut -d' ' -f1 /proc/uptime)" "$*" >> "$ram/runtime.log"; }
mode='';netpid='';retry=0;next_retry=0;stable_since=0;tf_auto_done=0;tf_auto_allowed=0;transition=''
[ ! -f "$root/modules-autoload" ] || tf_auto_allowed=1
# Adopt the actually connected R1 service without disrupting the first install.
if [ -f /tmp/disc-adb/adbd.pid ] && pidof adbd >/dev/null;then mode=usb;fi
stop_debug() {
 if [ -f /tmp/disc-adb/manager.pid ];then
  old=$(cat /tmp/disc-adb/manager.pid)
  case "$old" in ''|*[!0-9]*) :;; *)
   cmd=$(tr '\000' ' ' < /proc/$old/cmdline 2>/dev/null)
   case "$cmd" in *'/usr/bin/disc-adb.sh'*|*'/usr/data/disc-runtime/service/usb-manager.sh'*) kill -TERM "$old" 2>/dev/null;;esac
  esac
  n=0;while [ -d /tmp/disc-adb/lock ] && [ "$n" -lt 8 ];do sleep 1;publish_state;n=$((n+1));done
 fi
 if [ -n "$netpid" ] && [ "$(cat /proc/$netpid/comm 2>/dev/null)" = adbd ];then
  kill "$netpid";sleep 1
  [ "$(cat /proc/$netpid/comm 2>/dev/null)" != adbd ] || kill -9 "$netpid"
 fi
 netpid=''
}
read_config() {
 usb=1;wifi=0
 v=$(sed -n 's/^usb=\([01]\)$/\1/p' "$root/debug.conf" 2>/dev/null);[ "$v" != 0 ] || usb=0
 v=$(sed -n 's/^wireless=\([01]\)$/\1/p' "$root/debug.conf" 2>/dev/null);[ "$v" != 1 ] || wifi=1
 choose_mode
}
apply() {
 log "debug mode $mode -> $desired"
 case "$mode:$desired" in usb:both|both:usb) transition=wireless;;*) transition=debug;;esac
 usb_active=0;wireless_active=0;publish_state
 stop_debug
 case "$desired" in
  usb|both)
   DISC_ADBD_BIN=$service/profiles/$desired/adbd DISC_ADB_POLICY=$service/libdisc-adb-policy.so DISC_ADB_START_DELAY=2 \
    "$service/usb-manager.sh" > "$ram/usb-manager-launch.log" 2>&1 &;;
  wireless)
   if [ -e /dev/usb-ffs/adb/ep0 ] || [ -e /dev/android_adb ];then
    log 'FAIL wireless-only: USB endpoint still exists';return 1
   fi
   LD_PRELOAD=$service/libdisc-adb-policy.so "$service/profiles/wireless/adbd" > "$ram/wireless.log" 2>&1 &
   netpid=$!;;
 esac
 mode=$desired;stable_since=$(cut -d. -f1 /proc/uptime)
}
publish_state() {
 # Timestamp the measurement publication, after any blocking transport teardown.
 updated=$(cut -d. -f1 /proc/uptime)
 {
  printf 'mode=%s\nadbd_pid=%s\nusb_active=%s\nwireless_active=%s\n' "$mode" "$(pidof adbd 2>/dev/null)" "$usb_active" "$wireless_active"
  printf 'requested_usb=%s\nrequested_wireless=%s\npause_reason=%s\nrescue=%s\nwork_mode=%s\nupdated=%s\ntransition=%s\n' "$requested_usb" "$requested_wifi" "$pause_reason" "$rescue" "$work_mode" "$updated" "$transition"
 } > "$ram/state.new"
 mv "$ram/state.new" "$ram/state"
}
modules() {
 for dir in "$root/components"/*;do
  [ -d "$dir" ] && [ -f "$dir/enabled" ] && [ "$(cat "$dir/current/KIND" 2>/dev/null)" = service ] || continue
  id=${dir##*/};pidfile=$ram/module-$id
  [ "$id" = usb-settings ] || [ ! -f /tmp/sdcard/DISC_MODULES_SAFE_MODE ] || continue
  logfile=$ram/component-$id.log
  [ ! -f "$logfile" ] || { [ "$(wc -c < "$logfile")" -lt 262144 ] || : > "$logfile"; }
  active=0
  if [ -f "$pidfile" ];then
   read -r pid born < "$pidfile"
   [ ! -f /proc/$pid/stat ] || [ "$(awk '{print $22}' /proc/$pid/stat)" != "$born" ] || active=1
  fi
  [ "$active" -eq 0 ] || continue
  countfile=$ram/attempts-$id;count=0;[ ! -f "$countfile" ] || count=$(cat "$countfile")
  [ "$count" -lt 3 ] || { log "component $id stopped after three starts this boot";continue; }
  echo $((count+1)) > "$countfile"
  setsid "$service/module-run.sh" "$id" > "$ram/component-$id.log" 2>&1 &
  pid=$!;sleep 1
  [ ! -f /proc/$pid/stat ] || printf '%s %s\n' "$pid" "$(awk '{print $22}' /proc/$pid/stat)" > "$pidfile"
  log "component $id started pid=$pid"
 done
}
log 'runtime v1.2 mode-aware debug started; original UI/audio untouched'
while :;do
 "$service/module-state.sh"
 "$service/personal-state.sh"
 if [ -f "$ram/personal-command" ] && [ ! -d "$ram/personal-lock" ];then
  "$service/personal-control.sh" > "$ram/personal-worker.log" 2>&1 &
 fi
 if [ "$tf_auto_done" = 0 ] && [ "$tf_auto_allowed" = 1 ] && [ -f "$root/modules-autoload" ] && [ ! -f /tmp/sdcard/DISC_MODULES_SAFE_MODE ] &&
    awk '$2=="/tmp/sdcard" {ok=1} END {exit !ok}' /proc/mounts;then
  tf_auto_done=1;: > "$ram/tf-load.request"
 fi
 # UI creates a one-shot request; file operations run outside the UI thread.
 if [ -f "$ram/tf-load.request" ];then
  rm -f "$ram/tf-load.request"
  "$service/tf-load.sh" > "$ram/tf-load.log" 2>&1 &
 fi
 read_config
 t=$(cut -d. -f1 /proc/uptime)
 if [ "$mode" != "$desired" ];then retry=0;next_retry=0;apply || { sleep 1;continue; };fi
 debug_health
 healthy=1
 case "$mode" in usb) [ "$usb_active" = 1 ] || healthy=0;;both) [ "$usb_active:$wireless_active" = 1:1 ] || healthy=0;;wireless) [ "$wireless_active" = 1 ] || healthy=0;;esac
 # Recover a dead/detached owned service without requiring OFF/ON clicks.
 # Give FunctionFS time to initialize, and back off after three failed starts.
 if [ "$healthy" = 0 ] && [ $((t-stable_since)) -gt 12 ] && [ "$t" -ge "$next_retry" ];then
  retry=$((retry+1));log "debug health retry=$retry mode=$mode"
  if [ "$retry" -le 3 ];then next_retry=$((t+15));else next_retry=$((t+60));fi
  apply || true
  debug_health
 elif [ "$healthy" = 1 ] && [ $((t-stable_since)) -gt 30 ];then retry=0;fi
 [ "$healthy" != 1 ] || transition=''
 publish_state
 modules
 sleep 1
done
