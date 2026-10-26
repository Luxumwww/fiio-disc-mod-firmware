#!/bin/sh
unset LD_PRELOAD
umask 077
root=/usr/data/disc-runtime
base=$root/components
rename=$root/service/atomic-rename
ram=/tmp/disc-runtime
id=$2
valid() { case "$1" in ''|*[!a-zA-Z0-9._-]*|.*|*..*) return 1;;esac;[ "${#1}" -le 64 ]; }
relative() { case "$1" in ''|/*|*..*|*[!a-zA-Z0-9_./-]*) return 1;;esac; }
case "$1" in
 list)
  for dir in "$base"/*;do
   [ -d "$dir" ] || continue
   v=$(readlink "$dir/current" 2>/dev/null);entry=$(cat "$dir/current/ENTRY" 2>/dev/null);kind=$(cat "$dir/current/KIND" 2>/dev/null)
   enabled=0;[ ! -f "$dir/enabled" ] || enabled=1
   [ "$kind" != tool ] || { enabled=1;[ ! -f "$dir/disabled" ] || enabled=0; }
   printf '%s|%s|%s|%s|%s\n' "${dir##*/}" "${v#releases/}" "$kind" "$enabled" "$entry"
  done;exit 0;;
esac
valid "$id" || exit 2
dir=$base/$id
stop_live() {
 file=/tmp/disc-runtime/module-$id
 [ -f "$file" ] || return 0
 read -r pid born < "$file"
 case "$pid:$born" in *[!0-9:]*|:*) return 0;;esac
 [ -f /proc/$pid/stat ] || return 0
 set -- $(awk '{print $5,$22}' /proc/$pid/stat)
 [ "$1:$2" = "$pid:$born" ] || return 0
 kill -TERM -- "-$pid" 2>/dev/null
 sleep 1
 [ ! -f /proc/$pid/stat ] || { [ "$(awk '{print $22}' /proc/$pid/stat)" != "$born" ] || kill -KILL -- "-$pid" 2>/dev/null; }
}
case "$1" in
 install|install-tf)
  action=$1
  ver=$3;entry=$4;kind=$5
  valid "$ver" && relative "$entry" || exit 2
  case "$kind" in tool|service|ui) :;; *) exit 2;; esac
  [ ! -L "$base" ] && [ ! -L "$dir" ] || exit 2
  mkdir -p "$dir/releases" || exit 1
  stage=/usr/data/disc-runtime/incoming/$id-$ver
  if [ "$action" = install-tf ];then
   stage=$6
   [ "$stage" = "/tmp/sdcard/DISC_MODULES/$id/$ver" ] || exit 2
   "$root/service/bundle-check.sh" "$stage" || exit 4
  fi
  [ -d "$stage" ] && [ ! -L "$stage" ] || exit 1
  [ ! -e "$dir/releases/$ver" ] || { echo 'version already installed' >&2;exit 3; }
  count=0;for old in "$dir/releases"/*;do [ ! -d "$old" ] || count=$((count+1));done
  [ "$count" -lt 3 ] || { echo 'three versions retained; remove an unused version first' >&2;exit 3; }
  (cd "$stage" && sha256sum -c SHA256SUMS) || { echo 'package checksum failed' >&2;exit 4; }
  [ -f "$stage/$entry" ] && [ ! -L "$stage/$entry" ] || exit 2
  [ "$(cat "$stage/ENTRY")" = "$entry" ] && [ "$(cat "$stage/KIND")" = "$kind" ] || exit 2
  chmod 755 "$stage/$entry" || exit 1
  # New release becomes visible only after all hashes pass.
  if [ "$action" = install-tf ];then ln -s "$stage" "$dir/releases/$ver" || exit 1
  else mv "$stage" "$dir/releases/$ver" || exit 1;fi
  stop_live
  previous=$(readlink "$dir/current" 2>/dev/null)
  if [ -n "$previous" ]; then ln -s "$previous" "$dir/previous.new" && "$rename" "$dir/previous.new" "$dir/previous" || exit 1;fi
  ln -s "releases/$ver" "$dir/current.new" && "$rename" "$dir/current.new" "$dir/current" && sync || exit 1
  rm -f "$ram/attempts-$id"
  echo "installed $id@$ver";;
 rollback)
  previous=$(readlink "$dir/previous" 2>/dev/null);current=$(readlink "$dir/current" 2>/dev/null)
  [ -d "$dir/$previous" ] && [ -n "$previous" ] || exit 3
  stop_live
  ln -s "$previous" "$dir/current.new" && "$rename" "$dir/current.new" "$dir/current" || exit 1
  ln -s "$current" "$dir/previous.new" && "$rename" "$dir/previous.new" "$dir/previous" && sync
  rm -f "$ram/attempts-$id"
  echo "rolled back $id";;
 enable|disable)
  [ -d "$dir/current" ] || exit 3
  if [ "$1" = enable ]; then
   case "$(cat "$dir/current/KIND")" in service|tool) :;;*) echo 'UI requires display integration' >&2;exit 3;;esac
   rm -f "$ram/attempts-$id"
   rm -f "$dir/disabled"
   : > "$dir/enabled"
  else : > "$dir/disabled";rm -f "$dir/enabled";stop_live;fi
  sync;;
 run)
  [ ! -f "$dir/disabled" ] || { echo 'module disabled' >&2;exit 3; }
  [ "$(cat "$dir/current/KIND" 2>/dev/null)" = tool ] || { echo 'only tools run through this synchronous entry' >&2;exit 3; }
  entry=$(cat "$dir/current/ENTRY");relative "$entry" || exit 2
  exec "$root/service/module-run.sh" "$id";;
 *) echo 'usage: componentctl.sh list/install/rollback/enable/disable/run' >&2;exit 2;;
esac
