#!/bin/sh
unset LD_PRELOAD
root=/usr/data/disc-runtime
ram=/tmp/disc-runtime
mkdir -p "$ram"
if [ -f "$ram/module-command" ];then
 IFS='|' read -r action id < "$ram/module-command"
 rm -f "$ram/module-command"
 case "$id" in ''|*[!a-zA-Z0-9._-]*|.*|*..*|usb-settings|disc-runtime) action=invalid;;esac
 case "$action" in
  enable|disable)
   if "$root/service/componentctl.sh" "$action" "$id" > "$ram/module-command.log" 2>&1;then
    printf '操作已完成\n' > "$ram/module-result"
   else printf '操作失败，请检查模块\n' > "$ram/module-result";fi;;
 esac
fi
: > "$ram/modules.list.new"
count=0
for dir in "$root/components"/*;do
 [ -d "$dir" ] || continue
 id=${dir##*/};case "$id" in usb-settings|disc-runtime) continue;;esac
 kind=$(cat "$dir/current/KIND" 2>/dev/null)
 # Keep missing-card registrations visible; KIND may be unavailable then.
 enabled=0;[ ! -f "$dir/enabled" ] || enabled=1
 [ "$kind" != tool ] || { enabled=1;[ ! -f "$dir/disabled" ] || enabled=0; }
 location=NAND;target=$(readlink "$dir/current" 2>/dev/null)
 [ ! -L "$dir/$target" ] || location=TF
 state=idle
 [ -d "$dir/current" ] || state=missing
 if [ -f "$ram/module-$id" ];then
  read -r pid born < "$ram/module-$id"
  case "$pid:$born" in *[!0-9:]*|:*) :;;*)
   [ "$(awk '{print $22}' /proc/$pid/stat 2>/dev/null)" != "$born" ] || state=running;;esac
 fi
 if { [ "$id" = disc-adb-tcpip ] || [ "$id" = disc-bt-meter ] || [ "$id" = disc-hwaccel ]; } && [ -f "$ram/integrated-$id" ];then
  read -r applied < "$ram/integrated-$id"
  [ "$applied" != "$target" ] || state=running
 fi
 [ "$enabled" != 0 ] || state=disabled
 [ "$state" = running ] || [ "$enabled" = 0 ] || [ ! -f /tmp/sdcard/DISC_MODULES_SAFE_MODE ] || state=safe
 printf '%s|%s|%s|%s|%s|%s\n' "$id" "$enabled" "$kind" "$location" "$state" "${target#releases/}" >> "$ram/modules.list.new"
 count=$((count+1));[ "$count" -lt 96 ] || break
done
mv "$ram/modules.list.new" "$ram/modules.list"
# Use df's filesystem counters, not the sum of physical flash partitions.
{
 df -Pk /usr/data | awk 'END {printf "nand=%s,%s,%s\n",$2,$3,$4}'
 if awk '$2=="/tmp/sdcard" {ok=1} END {exit !ok}' /proc/mounts;then
  df -Pk /tmp/sdcard | awk 'END {printf "tf=%s,%s,%s\n",$2,$3,$4}'
 else printf 'tf=absent\n';fi
} > "$ram/storage.state.new"
mv "$ram/storage.state.new" "$ram/storage.state"
