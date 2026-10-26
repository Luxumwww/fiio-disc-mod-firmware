#!/bin/sh
# Personal-data snapshots only. Firmware, device identity and module binaries
# are deliberately outside the whitelist. SQLite uses the native backup API.
unset LD_PRELOAD
umask 077
data=${DISC_PERSONAL_DATA:-/usr/data}
ram=${DISC_PERSONAL_RAM:-/tmp/disc-runtime}
tf=${DISC_PERSONAL_TF:-/tmp/sdcard}
service=${DISC_PERSONAL_SERVICE:-$data/disc-runtime/service}
txn=$data/.disc-personal-restore
pending_path=''
trap 'if [ -n "$pending_path" ];then case "$pending_path" in "$tf/DISC_DATA_BACKUP/."*.pending) rm -rf "$pending_path";;esac;fi' EXIT
scope='wpa_supplicant.conf
bluetooth.db
storebluetooth
fiio/db/sysconfig.db
fiio/db/theme.db
fiio/db/song.db
fiio/custom_gif.gif
fiio/wifi_transfer/pic/lock_screen
fiio/bt_last_connect
fiio/bt_name.txt
fiio/bt_addr
disc-runtime/debug.conf
disc-runtime/modules-autoload
disc-runtime/wifi/catalog.conf'
result(){ mkdir -p "$ram";printf '%s\n' "$*" > "$ram/personal-result.new";mv "$ram/personal-result.new" "$ram/personal-result"; }
board(){ sha256sum "$data/macaddr.txt" 2>/dev/null | cut -d' ' -f1; }
tf_ready(){ [ "${DISC_PERSONAL_TEST:-0}" = 1 ] || awk -v p="$tf" '$2==p {ok=1} END {exit !ok}' /proc/mounts; }
inventory(){ (cd "$1" && find data -type f -exec sha256sum '{}' \; | LC_ALL=C sort); }
names_valid(){
 local bad
 bad=$(find "$1" -exec sh -c '
  for p do
   case "$p" in *"\\"*|*".."*) echo bad;continue;;esac
   if printf "%s" "$p" | LC_ALL=C grep "[[:cntrl:]]" >/dev/null;then echo bad;fi
   [ -d "$p" ] || [ -f "$p" ] || echo bad
   [ ! -L "$p" ] || echo bad
  done' sh '{}' +)
 [ -z "$bad" ]
}
decode_names(){
 local p name decoded parent
 find "$1" -depth -mindepth 1 -print > "$ram/personal-decode-paths"
 while IFS= read -r p;do
  name=${p##*/};decoded=$(printf '%s' "$name" | sed 's/%3A/:/g;s/%25/%/g')
  [ "$name" = "$decoded" ] || {
   parent=${p%/*};[ ! -e "$parent/$decoded" ] || return 23
   mv "$p" "$parent/$decoded" || return 29
  }
 done < "$ram/personal-decode-paths"
}
paths_valid(){
 local bad p rel root ok
 [ -d "$1/data" ] && [ ! -L "$1/data" ] || return 1
 # Check names using find's argument interface, so embedded newlines cannot
 # bypass a line-based path check. No links or special files are accepted.
 names_valid "$1/data" || return 1
 # Every payload path must be under a known personal-data root.
 find "$1/data" -mindepth 1 -print > "$ram/personal-paths"
 while IFS= read -r p;do
  rel=${p#"$1/data/"};ok=0
  while IFS= read -r root;do
   case "$rel" in "$root"|"$root"/*) ok=1;;esac
   case "$root" in "$rel"/*) ok=1;;esac
  done <<EOF
$scope
EOF
  [ "$ok" = 1 ] || return 1
 done < "$ram/personal-paths"
 while IFS= read -r root;do
  [ ! -e "$1/data/$root" ] || case "$root" in
   storebluetooth|fiio/wifi_transfer/pic/lock_screen) [ -d "$1/data/$root" ] || return 1;;
   *) [ -f "$1/data/$root" ] || return 1;;
  esac
 done <<EOF
$scope
EOF
}
validate(){
 local dir id db
 dir=$1
 [ -d "$dir" ] && [ ! -L "$dir" ] && [ -f "$dir/META" ] && [ ! -L "$dir/META" ] &&
 [ -f "$dir/SHA256SUMS" ] && [ ! -L "$dir/SHA256SUMS" ] || return 20
 [ "$(wc -c < "$dir/META")" -lt 512 ] && [ "$(wc -c < "$dir/SHA256SUMS")" -lt 262144 ] || return 20
 grep -qx 'format=DISC-PERSONAL-2' "$dir/META" && grep -qx 'encoding=percent25-colon3A' "$dir/META" && grep -qx 'base=257' "$dir/META" || return 21
 id=$(board);[ "${#id}" = 64 ] && grep -qx "board=$id" "$dir/META" || return 22
 paths_valid "$dir" || return 23
 [ "$(find "$dir/data" -type f | wc -l)" -le 1024 ] && [ "$(du -sk "$dir/data" | awk '{print $1}')" -le 40960 ] || return 24
 inventory "$dir" > "$ram/personal-sums.actual" || return 25
 cmp -s "$ram/personal-sums.actual" "$dir/SHA256SUMS" || return 25
 for db in bluetooth.db fiio/db/sysconfig.db fiio/db/theme.db fiio/db/song.db;do
  [ ! -f "$dir/data/$db" ] || [ "$(sqlite3 "$dir/data/$db" 'PRAGMA quick_check;' 2>/dev/null)" = ok ] || return 26
 done
}
snapshot(){
 local id base stamp name n temp rel dst src encoded
 tf_ready || return 10
 id=$(board);[ "${#id}" = 64 ] || return 11
 base=$tf/DISC_DATA_BACKUP
 [ ! -L "$base" ] || return 12
 mkdir -p "$base" || return 12
 stamp=$(date +%Y%m%d-%H%M%S);name=$stamp;n=2
 while [ -e "$base/$name" ] || [ -e "$base/.$name.pending" ];do name=$stamp-$(printf '%02d' "$n");n=$((n+1));done
 temp=$base/.$name.pending;mkdir -p "$temp/data" || return 12
 pending_path=$temp
 printf 'format=DISC-PERSONAL-2\nencoding=percent25-colon3A\nbase=257\nboard=%s\ncreated=%s\npurpose=%s\n' "$id" "$stamp" "${1:-manual}" > "$temp/META"
 while IFS= read -r rel;do
  [ ! -e "$data/$rel" ] || {
   [ ! -L "$data/$rel" ] || return 13
   dst=$temp/data/$rel;mkdir -p "${dst%/*}" || return 12
   case "$rel" in *.db)
    sqlite3 "$data/$rel" '.timeout 3000' ".backup '$dst'" >/dev/null 2>&1 || return 14;;
   *)
    # Stream files without duplicating all personal data in RAM or NAND.
    # Percent itself is escaped first, making the path mapping reversible.
    names_valid "$data/$rel" || return 13
    find "$data/$rel" -print > "$ram/personal-copy-paths"
    while IFS= read -r src;do
     encoded=$(printf '%s' "${src#"$data/"}" | sed 's/%/%25/g;s/:/%3A/g')
     dst=$temp/data/$encoded
     if [ -d "$src" ];then mkdir -p "$dst" || return 15
     else mkdir -p "${dst%/*}" && cp -p "$src" "$dst" || return 15;fi
    done < "$ram/personal-copy-paths";;esac
  }
 done <<EOF
$scope
EOF
 paths_valid "$temp" || return 13
 [ "$(du -sk "$temp/data" | awk '{print $1}')" -le 40960 ] || return 16
 inventory "$temp" > "$temp/SHA256SUMS" || return 15
 validate "$temp" || return $?
 sync
 mv "$temp" "$base/$name" || return 15
 pending_path=''
 sync
 snapshot_name=$name
}
rollback(){
 local index rel parent suffix
 [ -d "$txn" ] && [ ! -L "$txn" ] || return 0
 if [ -f "$txn/COMMITTED" ];then rm -rf "$txn";return 0;fi
 [ -f "$txn/READY" ] || { rm -rf "$txn";return 0; }
 index=0
 while IFS= read -r rel;do
  index=$((index+1))
  if [ -e "$txn/old/$index" ];then
   rm -rf "$data/$rel" "$data/$rel-wal" "$data/$rel-shm" "$data/$rel-journal"
   parent=${rel%/*};[ "$parent" = "$rel" ] || mkdir -p "$data/$parent"
   mv "$txn/old/$index" "$data/$rel" || return 30
  elif [ -f "$txn/absent/$index" ];then
   rm -rf "$data/$rel" "$data/$rel-wal" "$data/$rel-shm" "$data/$rel-journal"
  fi
  # Companions move before the base DB, so recovery also handles a power loss
  # between those moves while the base file still occupies its original path.
  for suffix in wal shm journal;do
   [ ! -e "$txn/old/$index-$suffix" ] || mv "$txn/old/$index-$suffix" "$data/$rel-$suffix" || return 30
  done
 done <<EOF
$scope
EOF
 sync;rm -rf "$txn";result '上次恢复未完成，已回到恢复前数据'
}
paused=''
resume(){ for p in $paused;do kill -CONT "$p" 2>/dev/null || true;done;paused=''; }
restore(){
 local dir required free protection index rel failed parent suffix pids guard p
 tf_ready || return 10
 case "$1" in ''|*[!0-9-]*|.*) return 19;;esac
 dir=$tf/DISC_DATA_BACKUP/$1
 validate "$dir" || return $?
 for parent in fiio fiio/db fiio/wifi_transfer fiio/wifi_transfer/pic disc-runtime;do
  [ ! -L "$data/$parent" ] || return 23
 done
 [ ! -e "$txn" ] || return 27
 required=$(du -sk "$dir/data" | awk '{print $1}')
 free=$(df -Pk "$data" | awk 'END {print $4}')
 [ "$free" -gt $((required+8192)) ] || return 28
 result '正在保存恢复前数据'
 snapshot before-restore || return $?
 protection=$snapshot_name
 mkdir -p "$txn/new" "$txn/old" "$txn/absent" || return 29
 cp -Rp "$dir/data/." "$txn/new/" || return 29
 # exFAT does not preserve POSIX modes. Restore private modes on NAND.
 find "$txn/new" -type d -exec chmod 700 '{}' \;
 find "$txn/new" -type f -exec chmod 600 '{}' \;
 # Verify the staged copy again before touching live data.
 mkdir "$txn/check";mv "$txn/new" "$txn/check/data"
 cp "$dir/META" "$dir/SHA256SUMS" "$txn/check/"
 validate "$txn/check" || { rm -rf "$txn";return 25; }
 mv "$txn/check/data" "$txn/new";rm -rf "$txn/check"
 decode_names "$txn/new" || { rm -rf "$txn";return 23; }
 index=0
 while IFS= read -r rel;do
  index=$((index+1));[ -e "$data/$rel" ] || : > "$txn/absent/$index"
 done <<EOF
$scope
EOF
 : > "$txn/READY";sync
 result '正在恢复，完成后自动重启'
 if [ "${DISC_PERSONAL_TEST:-0}" != 1 ];then
  # Stop writers without terminating them: the stock supervisor keeps its
  # watchdog alive. The manager/guard must not reload UI during the transaction.
  pids="$(pidof mq_ui mq_player wpa_supplicant bluetoothd bt_server 2>/dev/null) $(cat "$ram/manager.pid" 2>/dev/null)"
  guard=$(cat "$ram/module-usb-settings" 2>/dev/null);pids="$pids ${guard%% *}"
  for p in $pids;do case "$p" in ''|*[!0-9]*) continue;;esac
   [ "$p" = "$$" ] || { kill -STOP "$p" 2>/dev/null && paused="$paused $p"; }
  done
 fi
 trap 'rollback;resume;exit 32' HUP INT TERM
 index=0;failed=0
 while IFS= read -r rel;do
  index=$((index+1))
  # Keep old SQLite companions for rollback, but never replay them against
  # the restored database. Writers remain suspended until reboot/rollback.
  case "$rel" in *.db)
   for suffix in wal shm journal;do
    [ ! -e "$data/$rel-$suffix" ] || mv "$data/$rel-$suffix" "$txn/old/$index-$suffix" || failed=1
   done;;esac
  [ "$failed" = 0 ] || break
  if [ -e "$data/$rel" ];then mv "$data/$rel" "$txn/old/$index" || { failed=1;break; };fi
  if [ -e "$txn/new/$rel" ];then
   parent=${rel%/*};[ "$parent" = "$rel" ] || mkdir -p "$data/$parent"
   mv "$txn/new/$rel" "$data/$rel" || { failed=1;break; }
  fi
 done <<EOF
$scope
EOF
 if [ "$failed" = 1 ];then rollback;resume;return 30;fi
 sync;: > "$txn/COMMITTED";sync
 result "恢复完成；保护备份 $protection"
 if [ "${DISC_PERSONAL_TEST:-0}" = 1 ];then rm -rf "$txn";return 0;fi
 # A graceful UI exit would save stale preferences over restored databases.
 reboot -f
 # If reboot fails, undo rather than continue with stale process state.
 rm -f "$txn/COMMITTED";rollback;resume;return 31
}
delete_backup(){
 local id dir base gone
 tf_ready || return 10
 id=$1;case "$id" in ''|*[!0-9-]*|.*) return 19;;esac
 base=$tf/DISC_DATA_BACKUP;dir=$base/$id
 [ ! -L "$base" ] && [ ! -L "$dir" ] && [ -d "$dir" ] && [ -f "$dir/META" ] || return 20
 [ "$(readlink -f "$dir")" = "$base/$id" ] || return 23
 grep -qx 'format=DISC-PERSONAL-2' "$dir/META" || return 21
 grep -qx "board=$(board)" "$dir/META" || return 22
 gone=$base/.deleted-$id
 [ ! -e "$gone" ] || return 34
 mv "$dir" "$gone" || return 34
 # Both absolute names are checked children of the fixed backup directory.
 rm -rf "$gone" || return 34
 sync;result '备份已删除'
}
case "${1:-}" in
 backup) result '正在备份到 TF 卡';snapshot manual;code=$?;[ "$code" != 0 ] || result "备份完成：$snapshot_name";exit "$code";;
 restore) restore "$2";exit $?;;
 delete) delete_backup "$2";exit $?;;
 recover)
  # Immutable boot normally calls this before stock processes exist. The
  # editable-service fallback must not repair databases under a running UI.
  if [ "${DISC_PERSONAL_TEST:-0}" != 1 ] && [ -f "$txn/READY" ] && [ ! -f "$txn/COMMITTED" ] && pidof mq_ui >/dev/null;then
   for p in $(pidof mq_ui mq_player wpa_supplicant bluetoothd bt_server 2>/dev/null);do
    kill -STOP "$p" 2>/dev/null && paused="$paused $p"
   done
   rollback || { resume;exit 30; };sync;reboot -f;resume;exit 31
  fi
  rollback;;
 validate) validate "$2";exit $?;;
 *) exit 2;;
esac
