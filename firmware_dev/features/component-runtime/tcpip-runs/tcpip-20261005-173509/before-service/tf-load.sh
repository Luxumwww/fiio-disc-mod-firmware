#!/bin/sh
unset LD_PRELOAD
umask 077
root=/usr/data/disc-runtime
service=$root/service
ram=/tmp/disc-runtime
card=/tmp/sdcard
mkdir -p "$ram" "$root/incoming" "$root/components" || exit 1
mkdir "$ram/tf-load.lock" 2>/dev/null || exit 0
trap 'rmdir "$ram/tf-load.lock" 2>/dev/null' EXIT
result(){ printf '%s\n' "$*" > "$ram/tf-load.result.new";mv "$ram/tf-load.result.new" "$ram/tf-load.result"; }
result '正在检查 TF 模块'
[ "$(sed -n 's/^work_mode=//p' "$ram/state" 2>/dev/null)" != 4 ] || { result '请先退出存储模式';exit 1; }
awk -v target="$card" '$2==target {ok=1} END {exit !ok}' /proc/mounts || { result '请插入 TF 卡';exit 1; }
[ -d "$card/DISC_MODULES" ] && [ ! -L "$card/DISC_MODULES" ] || { result 'TF 卡没有模块目录';exit 1; }
ok=0;failed=0;skipped=0
for source in "$card/DISC_MODULES"/*/*;do
 [ -d "$source" ] || continue
 if ! "$service/bundle-check.sh" "$source";then failed=$((failed+1));continue;fi
 id=$(cat "$source/MODULE_ID");ver=$(cat "$source/MODULE_VERSION")
 [ "$source" = "$card/DISC_MODULES/$id/$ver" ] || { failed=$((failed+1));continue; }
 # System components cannot be replaced through this importer.
 [ "$id" != usb-settings ] && [ "$id" != disc-runtime ] || { failed=$((failed+1));continue; }
 dir=$root/components/$id
 [ ! -L "$dir" ] || { failed=$((failed+1));continue; }
 if [ -e "$dir/releases/$ver" ] || [ -L "$dir/releases/$ver" ];then skipped=$((skipped+1));continue;fi
 entry=$(cat "$source/ENTRY");kind=$(cat "$source/KIND");storage=$(cat "$source/STORAGE");startup=$(cat "$source/STARTUP")
 if [ "$storage" = nand ];then
  size=$(du -sk "$source" | awk '{print $1}');free=$(df -Pk "$root" | awk 'END {print $4}')
  [ "$free" -ge $((size+8192)) ] || { failed=$((failed+1));continue; }
  stage=$root/incoming/$id-$ver
  [ ! -e "$stage" ] || { failed=$((failed+1));continue; }
  mkdir "$stage" && cp -R "$source"/. "$stage"/ && "$service/bundle-check.sh" "$stage" || { failed=$((failed+1));continue; }
  if ! "$service/componentctl.sh" install "$id" "$ver" "$entry" "$kind";then failed=$((failed+1));continue;fi
 else
  # Register a validated card path, retaining no copy of the payload in NAND.
  "$service/componentctl.sh" install-tf "$id" "$ver" "$entry" "$kind" "$source" || { failed=$((failed+1));continue; }
 fi
 [ "$startup" != 1 ] || "$service/componentctl.sh" enable "$id" || { failed=$((failed+1));continue; }
 ok=$((ok+1))
done
sync
result "已加载 $ok，已有 $skipped，失败 $failed"
[ "$failed" = 0 ]
