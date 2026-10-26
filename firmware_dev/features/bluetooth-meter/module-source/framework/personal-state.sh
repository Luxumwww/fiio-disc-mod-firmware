#!/bin/sh
unset LD_PRELOAD
umask 077
data=${DISC_PERSONAL_DATA:-/usr/data}
ram=${DISC_PERSONAL_RAM:-/tmp/disc-runtime}
tf=${DISC_PERSONAL_TF:-/tmp/sdcard}
service=${DISC_PERSONAL_SERVICE:-$data/disc-runtime/service}
[ -d "$ram" ] || mkdir -p "$ram"
if [ -f "$ram/wifi-secret" ];then
 born='';[ ! -f "$ram/wifi-secret.at" ] || read -r born < "$ram/wifi-secret.at"
 case "$born" in ''|*[!0-9]*) born=0;;esac
 age=$(( $(cut -d. -f1 /proc/uptime) - born ))
 [ "$age" -le 60 ] || rm -f "$ram/wifi-secret" "$ram/wifi-secret.at"
fi
"$service/wifi-catalog.sh"
conf=$data/disc-runtime/wifi/catalog.conf
if [ -f "$conf" ];then
 hash=$(sha256sum "$conf");hash=${hash%% *}
 list_hash='';[ ! -f "$ram/wifi-list.hash" ] || read -r list_hash < "$ram/wifi-list.hash"
 if [ "$list_hash" != "$hash" ];then
  if LD_PRELOAD="$service/libdisc-wifi-config.so" /bin/true list "$conf" > "$ram/wifi-list.raw";then
   sed "s/^/$hash-/" "$ram/wifi-list.raw" > "$ram/wifi-list.new"
   mv "$ram/wifi-list.new" "$ram/wifi-list";printf '%s\n' "$hash" > "$ram/wifi-list.hash"
  else : > "$ram/wifi-list";printf 'Wi-Fi 配置格式暂不支持\n' > "$ram/personal-result";fi
 fi
else : > "$ram/wifi-list";rm -f "$ram/wifi-list.hash";fi
: > "$ram/backup-list.new"
if [ "${DISC_PERSONAL_TEST:-0}" = 1 ] || awk -v p="$tf" '$2==p {ok=1} END {exit !ok}' /proc/mounts;then
 for dir in "$tf/DISC_DATA_BACKUP"/*;do
  [ -d "$dir" ] && [ ! -L "$dir" ] && [ -f "$dir/META" ] || continue
  id=${dir##*/};case "$id" in *[!0-9-]*|'') continue;;esac
  printf '%s\n' "$id" >> "$ram/backup-list.new"
 done
fi
sort -r "$ram/backup-list.new" > "$ram/backup-list";rm -f "$ram/backup-list.new" "$ram/wifi-list.raw"
