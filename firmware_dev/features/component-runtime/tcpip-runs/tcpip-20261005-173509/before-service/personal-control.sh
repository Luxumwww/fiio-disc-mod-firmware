#!/bin/sh
unset LD_PRELOAD
umask 077
data=${DISC_PERSONAL_DATA:-/usr/data}
ram=${DISC_PERSONAL_RAM:-/tmp/disc-runtime}
tf=${DISC_PERSONAL_TF:-/tmp/sdcard}
service=${DISC_PERSONAL_SERVICE:-$data/disc-runtime/service}
mkdir -p "$ram"
mkdir "$ram/personal-lock" 2>/dev/null || exit 1
trap 'rm -f "$ram/personal-busy";rmdir "$ram/personal-lock" 2>/dev/null' EXIT
: > "$ram/personal-busy"
command=${1:-};arg=${2:-}
if [ -z "$command" ];then
 mv "$ram/personal-command" "$ram/personal-command.processing" 2>/dev/null || exit 1
 IFS='|' read -r command arg < "$ram/personal-command.processing";rm -f "$ram/personal-command.processing"
fi
result(){ printf '%s\n' "$*" > "$ram/personal-result.new";mv "$ram/personal-result.new" "$ram/personal-result"; }
case "$command" in
 backup|restore|backup-delete)
  result '正在处理，请勿拔卡或关机'
  operation=$command;[ "$command" != backup-delete ] || operation=delete
  "$service/personal-backup.sh" "$operation" "$arg"
  code=$?
  case "$code" in
   0) :;;10) result '请插入并挂载 TF 卡';;12|15|16) result 'TF 卡写入失败或空间不足';;
   20|21|23|24|25|26) result '备份不完整或格式不匹配，未恢复';;
   22) result '此备份不属于当前播放器';;28) result '内置存储空间不足，未恢复';;
   *) result "操作未完成（$code），原数据保留";;
  esac
  ;;
 secret|forget|connect)
  case "$arg" in *[!a-f0-9-]*|'') result '网络标识无效';exit 2;;esac
  hash=${arg%-*};index=${arg##*-}
  [ "${#hash}" = 64 ] && [ "${#index}" -le 3 ] || exit 2
  case "$index" in *[!0-9]*|'') exit 2;;esac
  conf=$data/disc-runtime/wifi/catalog.conf
  [ "$(sha256sum "$conf" | cut -d' ' -f1)" = "$hash" ] || { result '网络列表已变化，请重新选择';exit 3; }
  if [ "$command" = connect ];then
   result '正在连接已保存的 Wi-Fi'
   "$service/wifi-connect.sh" "$arg";code=$?
   case "$code" in 0) result 'Wi-Fi 已连接';;3) result '网络列表已变化，请重新选择';;*) result '连接失败，已保留原网络配置';;esac
  elif [ "$command" = secret ];then
   rm -f "$ram/wifi-secret"
   if LD_PRELOAD="$service/libdisc-wifi-config.so" /bin/true secret "$conf" "$index" > "$ram/wifi-secret.new";then
    { printf '%s\n' "$arg";cat "$ram/wifi-secret.new"; } > "$ram/wifi-secret.publish"
    cut -d. -f1 /proc/uptime > "$ram/wifi-secret.at"
    mv "$ram/wifi-secret.publish" "$ram/wifi-secret";result '密码仅在当前页面显示'
   else result '无法读取此网络密码';fi
   rm -f "$ram/wifi-secret.new"
  else
   result '正在删除已保存网络'
   new=$data/disc-runtime/wifi/catalog.new
   active=$data/wpa_supplicant.conf
   active_new=$data/.disc-wifi-forget.new
   active_hash=$(sha256sum "$active" | cut -d' ' -f1)
   LD_PRELOAD="$service/libdisc-wifi-config.so" /bin/true prune "$conf" "$index" "$active" "$active_new" || exit 4
   LD_PRELOAD="$service/libdisc-wifi-config.so" /bin/true delete "$conf" "$index" "$new" || { result '配置读取失败，未删除';exit 4; }
   [ "$(sha256sum "$conf" | cut -d' ' -f1)" = "$hash" ] || { rm -f "$new";result '配置已变化，未删除';exit 3; }
   [ "$(sha256sum "$active" | cut -d' ' -f1)" = "$active_hash" ] || { rm -f "$new" "$active_new";result '连接配置已变化，未删除';exit 3; }
   changed=0;cmp -s "$active_new" "$active" || changed=1
   chmod 600 "$new" "$active_new";mv "$new" "$conf" || exit 4
   if [ "$changed" = 1 ];then mv "$active_new" "$active" || exit 4;else rm -f "$active_new";fi
   rm -f "$ram/wifi-capture" "$ram/wifi-capture.at" "$ram/wifi-import.hash";sync
   # Network IDs belong to the old configuration; rebuild the daemon's list.
   if [ "$changed" = 1 ] && [ "${DISC_PERSONAL_TEST:-0}" != 1 ] && pidof wpa_supplicant >/dev/null;then
    wpa_cli -i wlan0 reconfigure >/dev/null 2>&1 || { result '已删除，请重新开关 Wi-Fi';exit 0; }
   fi
   rm -f "$ram/wifi-secret";result '已删除该 Wi-Fi'
  fi
  ;;
 *) result '未知操作';exit 2;;
esac
"$service/personal-state.sh"
