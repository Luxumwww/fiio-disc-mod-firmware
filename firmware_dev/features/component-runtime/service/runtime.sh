#!/bin/sh
unset LD_PRELOAD
umask 077
root=/usr/data/disc-runtime
service=$root/service
ram=/tmp/disc-runtime
. "$service/mode-policy.sh"
. "$service/debug-state.sh"
. "$service/debug-cleanup.sh"
. "$service/runtime-modules.sh"
mkdir -p "$ram" || exit 1
# Recover an interrupted personal-data transaction before starting modules.
if [ -d /usr/data/.disc-personal-restore ];then "$service/personal-backup.sh" recover;fi
mkdir "$ram/lock" 2>/dev/null || exit 0
trap 'rmdir "$ram/lock" 2>/dev/null' EXIT
echo $$ > "$ram/manager.pid"
: > "$ram/runtime.log"
log() { [ "$(wc -c < "$ram/runtime.log")" -ge 262144 ] || printf '%s %s\n' "$(cut -d' ' -f1 /proc/uptime)" "$*" >> "$ram/runtime.log"; }
mode='';netpid='';retry=0;next_retry=0;stable_since=0;tf_auto_done=0;tf_auto_allowed=0;transition='';apply_goal='';apply_pending=0
[ ! -f "$root/modules-autoload" ] || tf_auto_allowed=1
# Adopt the exact owned profile after a manager-only reload. A wireless daemon
# must not be mislabeled USB and unnecessarily torn down during audio playback.
for adopt in $(pidof adbd 2>/dev/null);do
 if debug_owned_profile "$adopt";then mode=$debug_profile;[ "$mode" != wireless ] || netpid=$adopt
 elif [ -f /tmp/disc-adb/adbd.pid ] && [ "$(cat /tmp/disc-adb/adbd.pid)" = "$adopt" ] && [ "$(readlink /proc/$adopt/exe)" = /usr/bin/adbd ];then mode=usb;fi
done
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
 # Fast path: shell builtins avoid two sed processes on every health tick.
 if [ -f "$root/debug.conf" ];then
  while IFS= read -r config_line || [ -n "$config_line" ];do
   case "$config_line" in usb=0) usb=0;;wireless=1) wifi=1;;esac
  done < "$root/debug.conf"
 fi
 choose_mode
}
apply() {
 debug_health_snapshot=0
 log "debug mode $mode -> $desired"
 case "$mode:$desired" in usb:both|both:usb) transition=wireless;;*) transition=debug;;esac
 usb_active=0;wireless_active=0;publish_state
 stop_debug
 if ! disc_debug_cleanup;then log 'debug handoff pending: old endpoint/owner not released';return 1;fi
 policy=$service/libdisc-adb-policy.so
 [ -z "$disc_tcpip_library" ] || policy="$policy $disc_tcpip_library"
 case "$desired" in
  usb|both)
   DISC_ADBD_BIN=$service/profiles/$desired/adbd DISC_ADB_POLICY="$policy" DISC_ADB_START_DELAY=2 \
    "$service/usb-manager.sh" > "$ram/usb-manager-launch.log" 2>&1 &;;
  wireless)
   if [ -e /dev/usb-ffs/adb/ep0 ] || [ -e /dev/android_adb ];then
    log 'FAIL wireless-only: USB endpoint still exists';return 1
   fi
   LD_PRELOAD="$policy" "$service/profiles/wireless/adbd" > "$ram/wireless.log" 2>&1 &
   netpid=$!;;
 esac
 mode=$desired;stable_since=$(cut -d. -f1 /proc/uptime)
 disc_tcpip_loaded=$disc_tcpip_key;debug_priority_applied=${debug_priority_key:-}
}
publish_state() {
 # Timestamp the measurement publication, after any blocking transport teardown.
 read -r updated uptime_idle < /proc/uptime;updated=${updated%%.*}
 [ "${debug_health_snapshot:-0}" = 1 ] || adbd_pids=$(pidof adbd 2>/dev/null)
 {
  printf 'mode=%s\nadbd_pid=%s\nusb_active=%s\nwireless_active=%s\n' "$mode" "$adbd_pids" "$usb_active" "$wireless_active"
  printf 'requested_usb=%s\nrequested_wireless=%s\npause_reason=%s\nrescue=%s\nwork_mode=%s\nupdated=%s\ntransition=%s\n' "$requested_usb" "$requested_wifi" "$pause_reason" "$rescue" "$work_mode" "$updated" "$transition"
  printf 'usb_priority=%s\nusb_error=%s\n' "${debug_priority_state:-}" "${debug_priority_error:-}"
 } > "$ram/state.new"
 mv "$ram/state.new" "$ram/state"
 debug_health_snapshot=0
 runtime_modules_publish
}
debug_retry_later(){
 retry=$((retry+1))
 retry_now=$(cut -d. -f1 /proc/uptime)
 if [ "$retry" -le 3 ];then next_retry=$((retry_now+15));else next_retry=$((retry_now+60));fi
 log "debug handoff retry=$retry after=$next_retry mode=$mode desired=$desired"
}
debug_goal(){
 goal=$desired:$disc_tcpip_key:${debug_priority_key:-}
 if [ "$goal" != "$apply_goal" ];then apply_goal=$goal;retry=0;next_retry=0;apply_pending=0;fi
 if [ "$mode" != "$desired" ] || [ "$tcpip_restart" = 1 ] || [ "$tcpip_module_changed" = 1 ] || [ "${debug_priority_key:-}" != "${debug_priority_applied:-}" ];then apply_pending=1;fi
}
modules() {
 [ "$t" -ge "$runtime_services_next" ] || return 0
 runtime_services_next=$((t+5))
 for dir in "$root/components"/*;do
  kind='';[ ! -f "$dir/current/KIND" ] || read -r kind < "$dir/current/KIND"
  [ -d "$dir" ] && [ -f "$dir/enabled" ] && [ "$kind" = service ] || continue
  id=${dir##*/};pidfile=$ram/module-$id
  # Only this reviewed module has a runtime integration contract currently.
  if [ "$id" = disc-adb-tcpip ] && [ -f "$dir/current/INTEGRATION" ];then
   read -r integration < "$dir/current/INTEGRATION"
   [ "$integration" != runtime ] || continue
  fi
  integration='';[ ! -f "$dir/current/INTEGRATION" ] || read -r integration < "$dir/current/INTEGRATION"
  if [ "$id" = disc-bt-meter ] && [ "$integration" = ui ] && [ "$disc_bt_ready" = 1 ];then continue;fi
  if { [ "$id" = disc-hwaccel ] || [ "$id" = disc-audio-xy ] || [ "$id" = disc-audio-polar ] || [ "$id" = disc-audio-spectrum ]; } && [ "$integration" = ui-provider ];then continue;fi
  if { [ "$id" = disc-audio-polar ] || [ "$id" = disc-audio-spectrum ]; } && [ "$disc_bt_ready" = 1 ];then
   applied='';[ ! -f "$ram/integrated-$id" ] || read -r applied < "$ram/integrated-$id"
   [ "$applied" != "$(readlink "$dir/current")" ] || continue
  fi
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
# Expensive catalogs do not belong on the one-second transport-health path.
# Explicit UI commands bypass the timer; no command is delayed for a scan.
runtime_catalog_next=0;runtime_refresh_next=0;runtime_services_next=0
runtime_housekeeping(){
 runtime_first_refresh=0
 runtime_catalog_ran=0;runtime_personal_ran=0
 if [ "${runtime_timers_ready:-0}" != 1 ];then
  runtime_timers_ready=1;runtime_first_refresh=1
  runtime_personal_next=$((t+3));runtime_services_next=$((t+1))
 fi
 runtime_command_pending=0
 [ ! -f "$ram/module-command" ] || runtime_command_pending=1
 if [ "$runtime_command_pending" = 1 ] || [ "$t" -ge "$runtime_catalog_next" ];then
  "$service/module-state.sh"
  runtime_catalog_next=$((t+10))
  runtime_catalog_ran=1
 fi
 # Time rounding and loop delays eventually aligned the ordinary scans.
 # Defer an overdue ordinary scan to the next tick; explicit commands bypass.
 if { [ "$runtime_catalog_ran" = 0 ] && [ "$t" -ge "$runtime_personal_next" ]; } || [ -f "$ram/personal-command" ] || [ -f "$ram/wifi-capture" ];then
  "$service/personal-state.sh"
  runtime_personal_next=$((t+10))
  runtime_personal_ran=1
 fi
 tcpip_module_changed=0
 if [ "$runtime_command_pending" = 1 ] || [ "$runtime_first_refresh" = 1 ] || { [ "$t" -ge "$runtime_refresh_next" ] && { [ "$runtime_catalog_ran:$runtime_personal_ran" = 0:0 ] || [ -f "$ram/personal-command" ] || [ -f "$ram/wifi-capture" ]; }; };then
  runtime_modules_refresh
  runtime_refresh_next=$((t+5))
  [ "$runtime_first_refresh" != 1 ] || runtime_refresh_next=$((t+2))
 fi
 [ "$runtime_command_pending" != 1 ] || runtime_services_next=0
}
runtime_personal_next=0
log 'runtime v1.2 mode-aware debug started; original UI/audio untouched'
while :;do
 read -r t uptime_idle < /proc/uptime;t=${t%%.*}
 runtime_mode_snapshot=1;runtime_mode_cached=0
 runtime_housekeeping
 # A manager-only reload may adopt the daemon already mapped from this exact
 # immutable tcpip release. Avoid restarting its working wireless connection.
 if [ -n "$disc_tcpip_library" ] && [ -z "$disc_tcpip_loaded" ] && [ "$tcpip_module_changed" = 1 ];then
  for adopted in $(pidof adbd 2>/dev/null);do
   if awk -v path="$disc_tcpip_library" '$NF==path {found=1} END {exit !found}' /proc/$adopted/maps;then disc_tcpip_loaded=$disc_tcpip_key;tcpip_module_changed=0;break;fi
  done
 fi
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
 tcpip_restart=0
 [ -z "$disc_tcpip_library" ] || tcpip_requests
 read_config
 read -r t uptime_idle < /proc/uptime;t=${t%%.*}
 debug_goal
 if [ "$apply_pending" = 1 ];then
  if [ "$t" -lt "$next_retry" ];then debug_health;publish_state;modules;sleep 1;continue;fi
  if apply;then apply_pending=0;else debug_retry_later;debug_health;publish_state;modules;sleep 1;continue;fi
 fi
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
