#!/bin/sh
unset LD_PRELOAD
set -eu
umask 077
root=/usr/data/disc-runtime
ram=/tmp/disc-runtime
release=$(pwd -P)
case "$release" in "$root/components/disc-bt-meter/releases/"*|/tmp/sdcard/DISC_MODULES/disc-bt-meter/*) :;;*) exit 2;;esac
test "$(cat "$root/service/API")" = 6
test "$(sha256sum /usr/bin/mq_ui | cut -d' ' -f1)" = f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226
test "$(sha256sum /usr/data/lib/librt.so.1 | cut -d' ' -f1)" = 1ccdf3b3ceb637bcc2ca6a72a8e37bf8d5008c6c532cb160f80e179c368f08fb
sha256sum -c SHA256SUMS >/dev/null
menu=$(sha256sum /usr/data/disc-settings.so | cut -d' ' -f1)
expected_menu=$(sha256sum bridge/disc-settings.so | cut -d' ' -f1)
framework=$(sha256sum "$root/service/runtime.sh" | cut -d' ' -f1)
expected_framework=$(sha256sum framework/runtime.sh | cut -d' ' -f1)
if [ "$menu:$framework" = "$expected_menu:$expected_framework" ];then exit 0;fi
case "$menu" in 0ba57235315d403de7c638b4c34774763e50e1f247f6283fdfd2aa1c147a7375|"$expected_menu") :;;*) echo 'unknown menu revision preserved' >&2;exit 3;;esac
case "$framework" in 8353f9aaf10fbba74fc7df781329125197a8436952b08a09102d10c5c468564b|00ab7aeb3688bd054173c54ae75789bc09c6f96476a0c49d6b3714f2d86ab9a8|e24673565583934d7070aa090f7efffe5ac22c7e26c65304f0a5c60f3ceb31e6|"$expected_framework") :;;*) echo 'unknown runtime revision preserved' >&2;exit 3;;esac
test ! -d "$ram/personal-lock"
mkdir "$ram/bt-install.lock" || exit 4
trap 'rmdir "$ram/bt-install.lock" 2>/dev/null' EXIT
stamp=$(cut -d. -f1 /proc/uptime)-$$
backup=$root/bt-before-$stamp
stage=$root/incoming/bt-service-$stamp
test ! -e "$backup" && test ! -e "$stage"
mkdir "$backup"
cp -a "$root/service" "$backup/service"
cp /usr/data/disc-settings.so "$backup/disc-settings.so"
cp /usr/data/disc-settings/VERSION "$backup/menu-VERSION"
cp -a "$root/service" "$stage"
for f in runtime.sh runtime-modules.sh module-state.sh;do cp "framework/$f" "$stage/$f.new";chmod 755 "$stage/$f.new";mv "$stage/$f.new" "$stage/$f";done
(
 cd "$stage"
 find . -type f ! -name SHA256SUMS ! -name SHA256SUMS.new | sort | while read -r f;do sha256sum "$f";done > SHA256SUMS.new
 mv SHA256SUMS.new SHA256SUMS
 sha256sum -c SHA256SUMS >/dev/null
 sh -n runtime.sh;sh -n runtime-modules.sh;sh -n module-state.sh
)
mkdir -p "$root/module-data/disc-bt-meter"
cp bridge/disc-settings.so /usr/data/disc-settings.so.bt-new
test "$(sha256sum /usr/data/disc-settings.so.bt-new | cut -d' ' -f1)" = "$expected_menu"
chmod 644 /usr/data/disc-settings.so.bt-new
# Controlled service replacement; preserve requests, configuration and all modules.
p=$(cat "$ram/manager.pid")
case "$p" in ''|*[!0-9]*) exit 5;;esac
grep -q '/usr/data/disc-runtime/service/runtime.sh' /proc/$p/cmdline || exit 5
kill -TERM "$p"
n=0;while [ -d /proc/$p ] && [ "$n" -lt 12 ];do sleep 1;n=$((n+1));done
test ! -d /proc/$p
mv "$root/service" "$backup/service-moved"
mv "$stage" "$root/service"
mv /usr/data/disc-settings.so.bt-new /usr/data/disc-settings.so
printf '1.8.0\n' > /usr/data/disc-settings/VERSION
printf '%s\n' "$backup" > "$ram/bt-install-backup"
sync
nohup setsid "$root/service/runtime.sh" > /tmp/disc-runtime-launch.log 2>&1 </dev/null &
sleep 3
if [ "$menu" != "$expected_menu" ];then
 # One initial menu reload; future module enable/disable needs no reload.
 ui=$(pidof mq_ui)
 test "$(readlink /proc/$ui/exe)" = /usr/bin/mq_ui
 kill -TERM "$ui"
fi
echo 'Bluetooth level module integrated; previous files preserved'
