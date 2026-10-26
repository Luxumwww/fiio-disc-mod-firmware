#!/bin/sh
# One-shot integration; no resident module process and no device reboot.
unset LD_PRELOAD
set -eu
umask 077
root=/usr/data/disc-runtime
ram=/tmp/disc-runtime
release=$(pwd -P)
case "$release" in "$root/components/disc-bt-meter/releases/"*|/tmp/sdcard/DISC_MODULES/disc-bt-meter/*) :;;*) exit 2;;esac
test "$(cat "$root/service/API")" = 6
test "$(sha256sum /usr/bin/mq_ui | cut -d' ' -f1)" = f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226
sha256sum -c SHA256SUMS >/dev/null
loader=$(sha256sum /usr/data/lib/librt.so.1 | cut -d' ' -f1)
expected_loader=$(sha256sum bridge/librt-ui-loader.so.1 | cut -d' ' -f1)
expected_menu=$(sha256sum bridge/disc-settings.so | cut -d' ' -f1)
menu='';test ! -f /usr/data/disc-settings/module-ui.so || menu=$(sha256sum /usr/data/disc-settings/module-ui.so | cut -d' ' -f1)
framework=$(sha256sum "$root/service/runtime.sh" | cut -d' ' -f1)
expected_framework=$(sha256sum framework/runtime.sh | cut -d' ' -f1)
if [ "$loader:$menu:$framework" = "$expected_loader:$expected_menu:$expected_framework" ];then exit 0;fi
case "$loader" in 1ccdf3b3ceb637bcc2ca6a72a8e37bf8d5008c6c532cb160f80e179c368f08fb|"$expected_loader") :;;*) echo 'unknown loader preserved' >&2;exit 3;;esac
case "$menu" in ''|"$expected_menu") :;;*) echo 'unknown module UI preserved' >&2;exit 3;;esac
case "$framework" in 7f235c3b219694b5e2e1483b3d8cb57515a66604382e6e48a35dd0fd74249c90|8353f9aaf10fbba74fc7df781329125197a8436952b08a09102d10c5c468564b|00ab7aeb3688bd054173c54ae75789bc09c6f96476a0c49d6b3714f2d86ab9a8|e24673565583934d7070aa090f7efffe5ac22c7e26c65304f0a5c60f3ceb31e6|"$expected_framework") :;;*) echo 'unknown runtime preserved' >&2;exit 3;;esac
test ! -d "$ram/personal-lock"
mkdir "$ram/bt-install.lock" || exit 4
watcher=''
cleanup(){ [ -z "$watcher" ] || kill -CONT "$watcher" 2>/dev/null || true;rmdir "$ram/bt-install.lock" 2>/dev/null || true; }
trap cleanup EXIT
trap 'exit 130' INT TERM
stamp=$(cut -d. -f1 /proc/uptime)-$$
backup=$root/bt-before-$stamp
stage=$root/incoming/bt-service-$stamp
test ! -e "$backup" && test ! -e "$stage"
mkdir "$backup"
cp /usr/data/lib/librt.so.1 "$backup/librt.so.1"
cp /usr/data/disc-settings.so "$backup/disc-settings.so"
cp /usr/data/disc-settings/VERSION "$backup/menu-VERSION"
test ! -f /usr/data/disc-settings/module-ui.so || cp /usr/data/disc-settings/module-ui.so "$backup/module-ui.so"
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
cp bridge/disc-settings.so /usr/data/disc-settings/module-ui.so.bt-new
cp bridge/librt-ui-loader.so.1 /usr/data/lib/librt.so.1.bt-new
test "$(sha256sum /usr/data/disc-settings/module-ui.so.bt-new | cut -d' ' -f1)" = "$expected_menu"
test "$(sha256sum /usr/data/lib/librt.so.1.bt-new | cut -d' ' -f1)" = "$expected_loader"
chmod 644 /usr/data/disc-settings/module-ui.so.bt-new /usr/data/lib/librt.so.1.bt-new
# TERM can leave the old shell's empty RAM lock. Remove it only after the
# verified runtime process exits, and preserve all configuration/modules.
p=$(cat "$ram/manager.pid")
case "$p" in ''|*[!0-9]*) exit 5;;esac
grep -q '/usr/data/disc-runtime/service/runtime.sh' /proc/$p/cmdline || exit 5
kill -TERM "$p"
n=0;while [ -d /proc/$p ] && [ "$n" -lt 12 ];do sleep 1;n=$((n+1));done
test ! -d /proc/$p
test ! -d "$ram/lock" || rmdir "$ram/lock"
mv "$root/service" "$backup/service"
mv "$stage" "$root/service"
# Dependency first, then loader. Immutable firmware deliberately preserves
# a different, user-owned loader on ordinary boots.
mv /usr/data/disc-settings/module-ui.so.bt-new /usr/data/disc-settings/module-ui.so
mv /usr/data/lib/librt.so.1.bt-new /usr/data/lib/librt.so.1
printf '1.8.1\n' > /usr/data/disc-settings/VERSION
printf '%s\n' "$backup" > "$ram/bt-install-backup"
sync
nohup setsid "$root/service/runtime.sh" > /tmp/disc-runtime-launch.log 2>&1 </dev/null &
n=0;ready=0
while [ "$n" -lt 20 ];do
 sleep 1;n=$((n+1));newp=$(cat "$ram/manager.pid")
 if [ "$newp" != "$p" ] && [ -d /proc/$newp ] && [ "$(cat "$ram/bt-module.path" 2>/dev/null)" = "$release/libdisc-bt-meter.so" ];then ready=1;break;fi
done
test "$ready" = 1
if [ "$loader:$menu" != "$expected_loader:$expected_menu" ];then
 ui=$(pidof mq_ui);player=$(pidof mq_player)
 test "$(readlink /proc/$ui/exe)" = /usr/bin/mq_ui
 test "$(readlink /proc/$player/exe)" = /usr/bin/mq_player
 # Bound the pause of the exact stock restart loop, with an independent
 # resume even if this installer is killed during the UI reload.
 for candidate in $(pidof sh);do
  cmd=$(tr '\000' ' ' < /proc/$candidate/cmdline)
  if [ "$cmd" = '/bin/sh /usr/project/fiio_init.sh ' ];then test -z "$watcher";watcher=$candidate;fi
 done
 test -n "$watcher"
 kill -STOP "$watcher"
 (sleep 10;kill -CONT "$watcher" 2>/dev/null || true) &
 kill -TERM "$ui"
 n=0;while [ -d /proc/$ui ] && [ "$n" -lt 3 ];do sleep 1;n=$((n+1));done
 test ! -d /proc/$ui
 LD_LIBRARY_PATH=/usr/lib:/usr/lib/pulseaudio/:/usr/data/lib:/usr/data/lib/ffmpeg nohup setsid /usr/bin/mq_ui >/tmp/disc-bt-ui.log 2>&1 </dev/null &
 sleep 3
 newui=$(pidof mq_ui)
 test "$newui" != "$ui"
 test "$(readlink /proc/$newui/exe)" = /usr/bin/mq_ui
 test "$(pidof mq_player)" = "$player"
 kill -CONT "$watcher";watcher=''
fi
echo 'Bluetooth module integrated; stock player retained and backups preserved'
