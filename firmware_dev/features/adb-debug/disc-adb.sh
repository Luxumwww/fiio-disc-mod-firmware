#!/bin/sh
# Experimental USB-only ADB. Stock kernel, adbd, UI and player are retained.
# No TCP listener, LD_PRELOAD, watchdog changes or application termination.
unset LD_PRELOAD ADB_TRACE
umask 077
PATH=/usr/bin:/bin:/usr/sbin:/sbin
export PATH
disc_state=/tmp/disc-adb
disc_config=/sys/kernel/config
disc_udcs=/sys/class/udc
disc_android=/sys/class/android_usb/android0
disc_ffs=/dev/usb-ffs/adb
disc_sd=/tmp/sdcard
disc_gadget=$disc_config/usb_gadget/disc_adb
disc_pid='';disc_mode='';disc_owned_ffs=0;disc_owned_config=0
disc_android_saved=0;disc_last='';disc_card_dir=''

disc_log() {
 [ "$(wc -c < "$disc_state/startup.log")" -lt 262144 ] || return 0
 read -r disc_up disc_rest < /proc/uptime
 printf 'uptime=%s %s\n' "$disc_up" "$*" >> "$disc_state/startup.log"
}
disc_write() { printf '%s' "$2" > "$1"; }
disc_card_ready() {
 [ -d "$disc_sd" ] && [ ! -L "$disc_sd" ] || return 1
 awk -v target="$disc_sd" '$2==target && $4 ~ /(^|,)rw(,|$)/ {ok=1} END {exit !ok}' /proc/mounts
}
disc_disabled() {
 [ -f /usr/data/DISC_ADB_DISABLE ] && return 0
 disc_card_ready && [ -f "$disc_sd/DISC_ADB_DISABLE" ]
}
disc_save() {
 disc_card_ready || return 0
 disc_root=$disc_sd/DISC_ADB
 [ ! -L "$disc_root" ] || return 0
 if [ -z "$disc_card_dir" ]; then
  mkdir -p "$disc_root" || return 0
  disc_count=0
  for disc_old in "$disc_root"/*; do [ -d "$disc_old" ] && disc_count=$((disc_count+1)); done
  [ "$disc_count" -lt 20 ] || return 0
  disc_card_dir=$disc_root/$disc_boot
  [ ! -L "$disc_card_dir" ] || { disc_card_dir=''; return 0; }
  mkdir -p "$disc_card_dir" || { disc_card_dir=''; return 0; }
 fi
 [ -d "$disc_card_dir" ] && [ ! -L "$disc_card_dir" ] || return 0
 for disc_file in startup.log capabilities.txt kernel.txt daemon.txt errors.txt; do
  [ ! -L "$disc_card_dir/$disc_file" ] || continue
  [ -f "$disc_state/$disc_file" ] && tail -c 262144 "$disc_state/$disc_file" > "$disc_card_dir/$disc_file"
 done
 sync
}
disc_daemon_start() {
 [ -e "$disc_ffs/ep0" ] || [ -e /dev/android_adb ] || { disc_log 'FAIL no USB endpoint; not starting adbd'; return 1; }
 # A pre-existing daemon is left alone. We only manage the PID we start.
 if pidof adbd >/dev/null 2>&1; then disc_log 'FAIL existing adbd; refusing to kill it'; return 1; fi
 /usr/bin/adbd > "$disc_state/daemon.txt" 2>&1 &
 disc_pid=$!
 printf '%s\n' "$disc_pid" > "$disc_state/adbd.pid"
 disc_log "adbd started pid=$disc_pid"
}
disc_daemon_stop() {
 if [ -n "$disc_pid" ] && kill -0 "$disc_pid" 2>/dev/null; then
  disc_cmd=$(cat /proc/$disc_pid/comm 2>/dev/null)
  if [ "$disc_cmd" = adbd ]; then
   kill "$disc_pid" 2>/dev/null
   sleep 1
   [ "$(cat /proc/$disc_pid/comm 2>/dev/null)" != adbd ] || kill -9 "$disc_pid" 2>/dev/null
  fi
 fi
 disc_pid=''
}
disc_unbind_others() {
 for disc_other in "$disc_config"/usb_gadget/*; do
  [ "$disc_other" != "$disc_gadget" ] && [ -f "$disc_other/UDC" ] || continue
  disc_bound=$(cat "$disc_other/UDC")
  [ -n "$disc_bound" ] || continue
  # Only remember a gadget once; keep the original binding for restoration.
  grep -Fqx "$disc_other|$disc_bound" "$disc_state/previous-bindings" ||
   printf '%s|%s\n' "$disc_other" "$disc_bound" >> "$disc_state/previous-bindings"
  disc_write "$disc_other/UDC" '' || return 1
  disc_log "unbound stock gadget $disc_other ($disc_bound)"
 done
}
disc_restore() {
 while IFS='|' read -r disc_old_gadget disc_old_udc; do
  [ -f "$disc_old_gadget/UDC" ] || continue
  disc_in_use=0
  for disc_other in "$disc_config"/usb_gadget/*; do
   [ -f "$disc_other/UDC" ] || continue
   [ "$(cat "$disc_other/UDC")" != "$disc_old_udc" ] || disc_in_use=1
  done
  if [ "$disc_in_use" -eq 0 ]; then
   disc_write "$disc_old_gadget/UDC" "$disc_old_udc" && disc_log "restored $disc_old_gadget"
  fi
 done < "$disc_state/previous-bindings"
}
disc_cleanup() {
 if [ "$disc_mode" = legacy ]; then
  disc_write "$disc_android/enable" 0
 fi
 if [ "$disc_owned_config" -eq 1 ] && [ -f "$disc_gadget/UDC" ]; then disc_write "$disc_gadget/UDC" ''; fi
 disc_daemon_stop
 if [ "$disc_owned_ffs" -eq 1 ]; then umount "$disc_ffs"; disc_owned_ffs=0; fi
 if [ "$disc_owned_config" -eq 1 ]; then
  # Remove only our named objects, never recursively delete stock gadgets.
  rm -f "$disc_gadget/configs/c.1/adb"
  rmdir "$disc_gadget/functions/ffs.adb" "$disc_gadget/configs/c.1/strings/0x409" \
   "$disc_gadget/configs/c.1" "$disc_gadget/strings/0x409" "$disc_gadget" 2>/dev/null
  disc_owned_config=0
 fi
 if [ "$disc_android_saved" -eq 1 ]; then
  for disc_attr in idVendor idProduct functions; do
   disc_write "$disc_android/$disc_attr" "$(cat "$disc_state/legacy-$disc_attr")"
  done
  disc_write "$disc_android/enable" "$(cat "$disc_state/legacy-enable")"
  disc_android_saved=0
 fi
 disc_restore
 disc_mode=''
}
disc_config_start() {
 [ -d "$disc_udcs" ] || return 1
 disc_udc=''
 for disc_entry in "$disc_udcs"/*; do
  [ -e "$disc_entry" ] || continue
  [ -z "$disc_udc" ] || { disc_log 'FAIL more than one UDC; ambiguous controller'; return 1; }
  disc_udc=${disc_entry##*/}
 done
 [ -n "$disc_udc" ] || { disc_log 'FAIL no USB device controller'; return 1; }
 mkdir -p "$disc_config" || return 1
 if ! awk -v t="$disc_config" '$2==t && $3=="configfs" {ok=1} END {exit !ok}' /proc/mounts; then
  mount -t configfs none "$disc_config" || return 1
 fi
 [ ! -e "$disc_gadget" ] || { disc_log 'FAIL disc_adb gadget already exists'; return 1; }
 mkdir "$disc_gadget" || return 1
 disc_owned_config=1
 # FunctionFS availability is tested BEFORE disturbing the active stock gadget.
 mkdir "$disc_gadget/functions/ffs.adb" || { disc_log 'FAIL kernel has no configfs ffs function'; return 1; }
 disc_write "$disc_gadget/idVendor" 0x18d1 || return 1
 disc_write "$disc_gadget/idProduct" 0x4ee7 || return 1
 disc_write "$disc_gadget/bcdUSB" 0x0200 || return 1
 disc_write "$disc_gadget/bcdDevice" 0x0100 || return 1
 mkdir "$disc_gadget/strings/0x409" "$disc_gadget/configs/c.1" || return 1
 disc_write "$disc_gadget/strings/0x409/serialnumber" disc-x2000-debug || return 1
 disc_write "$disc_gadget/strings/0x409/manufacturer" DISC-Debug || return 1
 disc_write "$disc_gadget/strings/0x409/product" DISC-ADB-Test || return 1
 disc_write "$disc_gadget/configs/c.1/MaxPower" 120 || return 1
 ln -s "$disc_gadget/functions/ffs.adb" "$disc_gadget/configs/c.1/adb" || return 1
 if awk -v t="$disc_ffs" '$2==t {ok=1} END {exit !ok}' /proc/mounts; then
  disc_log 'FAIL FunctionFS mount already used'; return 1
 fi
 mkdir -p "$disc_ffs" || return 1
 mount -t functionfs adb "$disc_ffs" || return 1
 disc_owned_ffs=1
 disc_daemon_start || return 1
 disc_wait=0
 while [ "$disc_wait" -lt 10 ]; do
  kill -0 "$disc_pid" 2>/dev/null || { disc_log 'FAIL adbd exited before endpoints'; return 1; }
  [ ! -e "$disc_ffs/ep1" ] || [ ! -e "$disc_ffs/ep2" ] || break
  sleep 1; disc_wait=$((disc_wait+1))
 done
 [ -e "$disc_ffs/ep1" ] && [ -e "$disc_ffs/ep2" ] || { disc_log 'FAIL FunctionFS descriptors not ready'; return 1; }
 disc_unbind_others || return 1
 disc_write "$disc_gadget/UDC" "$disc_udc" || { disc_log 'FAIL UDC bind'; return 1; }
 disc_mode=configfs
 disc_log "READY configfs/FunctionFS controller=$disc_udc (USB only)"
}
disc_legacy_start() {
 [ -f "$disc_android/functions" ] && [ -f "$disc_android/enable" ] || return 1
 for disc_attr in enable idVendor idProduct functions; do
  cat "$disc_android/$disc_attr" > "$disc_state/legacy-$disc_attr" || return 1
 done
 disc_android_saved=1;disc_mode=legacy
 disc_write "$disc_android/enable" 0 || return 1
 disc_write "$disc_android/functions" adb || return 1
 disc_write "$disc_android/idVendor" 18d1 || return 1
 disc_write "$disc_android/idProduct" 4ee7 || return 1
 # Legacy driver creates /dev/android_adb; stock daemon selects it if ep0 absent.
 disc_write "$disc_android/enable" 1 || return 1
 [ ! -e "$disc_ffs/ep0" ] || { disc_log 'FAIL lingering FunctionFS ep0 in legacy path'; return 1; }
 disc_daemon_start || return 1
 sleep 2
 kill -0 "$disc_pid" 2>/dev/null || { disc_log 'FAIL legacy adbd exited'; return 1; }
 disc_log 'READY legacy Android USB (USB only)'
}
disc_finish() {
 trap '' TERM INT
 disc_log 'STOP restoring USB configuration'
 disc_cleanup
 dmesg 2>/dev/null | tail -c 262144 > "$disc_state/kernel.txt"
 disc_save
 rmdir "$disc_state/lock" 2>/dev/null
 exit 0
}

# MAIN: tests source only the function definitions above this line.
mkdir -p "$disc_state" || exit 0
mkdir "$disc_state/lock" 2>/dev/null || exit 0
: > "$disc_state/startup.log"; : > "$disc_state/previous-bindings"
exec 2> "$disc_state/errors.txt"
disc_boot=$(cat /proc/sys/kernel/random/boot_id 2>/dev/null)
case "$disc_boot" in ''|*[!a-fA-F0-9-]*) disc_boot=unknown-boot;; esac
printf '%s\n' "$$" > "$disc_state/manager.pid"
trap disc_finish TERM INT
disc_log 'DISC ADB debug v1; default enabled; USB DAC/storage may be unavailable'
{
 uname -a
 printf '\n== mounts ==\n';cat /proc/mounts
 printf '\n== UDC ==\n';ls -l "$disc_udcs" 2>/dev/null
 printf '\n== legacy ==\n';ls -l "$disc_android" 2>/dev/null
 printf '\n== adbd dependencies ==\n';ls -l /usr/bin/adbd /lib/ld-linux-mipsn8.so.1
} > "$disc_state/capabilities.txt"
# Let stock UI/player initialize and TF mount; boot never waits on this process.
disc_wait=0
while [ "$disc_wait" -lt 20 ]; do
 if disc_disabled; then disc_log 'DISABLED marker found'; disc_save; rmdir "$disc_state/lock"; exit 0; fi
 sleep 1;disc_wait=$((disc_wait+1))
done
if ! disc_config_start; then
 disc_log 'configfs attempt failed; restoring before legacy fallback'
 disc_cleanup
 if ! disc_legacy_start; then
  disc_log 'FAIL neither USB ADB path could start'
  disc_finish
 fi
fi
dmesg 2>/dev/null | tail -c 262144 > "$disc_state/kernel.txt"
disc_save
disc_tick=0;disc_rebinds=0
while kill -0 "$disc_pid" 2>/dev/null; do
 disc_disabled && disc_finish
 if [ "$disc_mode" = configfs ]; then
  disc_current=$(cat "$disc_gadget/UDC" 2>/dev/null)
  if [ "$disc_current" != "$disc_udc" ]; then
   # Stock UI may take USB back on cable insertion. Bound retries avoid a busy fight.
   [ "$disc_rebinds" -lt 5 ] || { disc_log 'FAIL stock USB takeover persisted'; disc_finish; }
   disc_rebinds=$((disc_rebinds+1))
   disc_unbind_others && disc_write "$disc_gadget/UDC" "$disc_udc"
   disc_log "ADB rebind attempt=$disc_rebinds"
  fi
  disc_current=$(cat "$disc_udcs/$disc_udc/state" 2>/dev/null)
 else
  disc_current=$(cat "$disc_android/state" 2>/dev/null)
 fi
 if [ "$disc_current" != "$disc_last" ]; then disc_log "USB state=$disc_current";disc_last=$disc_current;disc_save;fi
 [ $((disc_tick%10)) -ne 0 ] || disc_save
 disc_tick=$((disc_tick+1));sleep 3
done
disc_log 'FAIL adbd exited after startup'
disc_finish
