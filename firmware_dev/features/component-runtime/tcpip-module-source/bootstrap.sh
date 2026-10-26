#!/bin/sh
unset LD_PRELOAD
set -e
umask 077
root=/usr/data/disc-runtime
ram=/tmp/disc-runtime
release=$(pwd -P)
case "$release" in "$root/components/disc-adb-tcpip/releases/"*|/tmp/sdcard/DISC_MODULES/disc-adb-tcpip/*) :;;*) exit 2;;esac
test "$(sha256sum /usr/bin/mq_ui | cut -d' ' -f1)" = f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226
test "$(cat "$root/service/API")" = 6
test ! -d "$ram/personal-lock"
sha256sum -c SHA256SUMS >/dev/null
current=$(sha256sum "$root/service/runtime.sh" | cut -d' ' -f1)
expected=$(sha256sum framework/runtime.sh | cut -d' ' -f1)
if [ "$current" = "$expected" ];then echo 'runtime integration already installed';exit 0;fi
case "$current" in
 00ab7aeb3688bd054173c54ae75789bc09c6f96476a0c49d6b3714f2d86ab9a8|e24673565583934d7070aa090f7efffe5ac22c7e26c65304f0a5c60f3ceb31e6) :;;
 *) echo 'unsupported runtime revision; preserved without modification' >&2;exit 3;;
esac
mkdir "$ram/tcpip-module-install.lock" || exit 4
trap 'rmdir "$ram/tcpip-module-install.lock" 2>/dev/null' EXIT
stamp=$(cut -d. -f1 /proc/uptime)-$$
stage=$root/incoming/tcpip-framework-$stamp
backup=$root/service-before-tcpip-module-$stamp
test ! -e "$stage" && test ! -e "$backup"
cp -a "$root/service" "$stage"
for file in runtime.sh runtime-modules.sh module-state.sh;do
 cp "framework/$file" "$stage/$file.new"
 chmod 755 "$stage/$file.new"
 mv "$stage/$file.new" "$stage/$file"
done
# Remove only our known standalone compatibility files, after checking ownership.
if [ -f "$stage/libdisc-adb-tcpip.so" ];then
 test "$(sha256sum "$stage/libdisc-adb-tcpip.so" | cut -d' ' -f1)" = a0f8d5abf09719cf68de928a0c9301e8ba08b772143736c07cee98159674716f
 rm "$stage/libdisc-adb-tcpip.so"
fi
if [ -f "$stage/tcpip-control.sh" ];then
 test "$(sha256sum "$stage/tcpip-control.sh" | cut -d' ' -f1)" = b74980c818b15a573867fe6419a043939ae8267bc662ebdccf3277f792524df9
 rm "$stage/tcpip-control.sh"
fi
(
 cd "$stage"
 find . -type f ! -name SHA256SUMS ! -name SHA256SUMS.new | sort | while read -r file;do sha256sum "$file";done > SHA256SUMS.new
 mv SHA256SUMS.new SHA256SUMS
 sha256sum -c SHA256SUMS >/dev/null
 sh -n runtime.sh;sh -n runtime-modules.sh;sh -n module-state.sh
)
# Detach from UI; stop only the existing runtime/ADB managers, never UI/player.
for pf in "$ram/manager.pid" /tmp/disc-adb/manager.pid;do
 p=$(cat "$pf" 2>/dev/null)
 case "$p" in ''|*[!0-9]*) continue;;esac
 cmd=$(tr '\000' ' ' < /proc/$p/cmdline 2>/dev/null)
 case "$cmd" in *'/usr/data/disc-runtime/service/runtime.sh'*|*'/usr/data/disc-runtime/service/usb-manager.sh'*) :;;*) exit 5;;esac
 kill -TERM "$p" 2>/dev/null || true
 n=0;while [ -d /proc/$p ] && [ "$n" -lt 12 ];do sleep 1;n=$((n+1));done
 test ! -d /proc/$p
done
for p in $(pidof adbd);do
 case "$(readlink /proc/$p/exe)" in "$root/service/profiles/"*/adbd) kill "$p";sleep 1;;*) exit 5;;esac
done
mv "$root/service" "$backup"
if ! mv "$stage" "$root/service";then mv "$backup" "$root/service";exit 6;fi
sync
rmdir "$ram/lock" /tmp/disc-adb/lock 2>/dev/null || true
printf '%s\n' "$backup" > "$ram/tcpip-module-backup"
nohup setsid "$root/service/runtime.sh" >/tmp/disc-runtime-launch.log 2>&1 </dev/null &
echo 'runtime integration installed; previous service preserved'
