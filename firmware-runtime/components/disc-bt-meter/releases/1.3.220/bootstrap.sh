#!/bin/sh
unset LD_PRELOAD
set -eu
root=/usr/data/disc-runtime
ram=/tmp/disc-runtime
release=$(pwd -P)
case "$release" in "$root/components/disc-bt-meter/releases/"*|/tmp/sdcard/DISC_MODULES/disc-bt-meter/*) :;;*) exit 2;;esac
test "$(cat "$root/service/API")" = 6
test "$(sha256sum /usr/bin/mq_ui | cut -d' ' -f1)" = f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226
test "$(sha256sum /usr/bin/mq_player | cut -d' ' -f1)" = a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc
sha256sum -c SHA256SUMS >/dev/null
test "$(sha256sum /usr/data/lib/librt.so.1 | cut -d' ' -f1)" = "$(sha256sum bridge/librt-ui-loader.so.1 | cut -d' ' -f1)"
test "$(sha256sum "$root/service/runtime.sh" | cut -d' ' -f1)" = "$(sha256sum framework/runtime.sh | cut -d' ' -f1)"
test "$(sha256sum "$root/service/mode-policy.sh" | cut -d' ' -f1)" = "$(sha256sum framework/mode-policy.sh | cut -d' ' -f1)"
test "$(sha256sum "$root/service/usb-manager.sh" | cut -d' ' -f1)" = "$(sha256sum framework/usb-manager.sh | cut -d' ' -f1)"
menu=$(sha256sum /usr/data/disc-settings/module-ui.so | cut -d' ' -f1)
expected=$(sha256sum bridge/disc-settings.so | cut -d' ' -f1)
if [ "$menu" = "$expected" ];then exit 0;fi
test "$menu" = 4da94dbf3e6df0d0489a609e4242638b42d153ac43aef6269d423fc5769e7b03
test ! -e "$ram/ram-dop-start"
ui=$(pidof mq_ui);player=$(pidof mq_player)
test "$(readlink /proc/$ui/exe)" = /usr/bin/mq_ui
test "$(readlink /proc/$player/exe)" = /usr/bin/mq_player
mkdir -p "$root/module-data/disc-audio-dsp"
cp bridge/disc-settings.so /usr/data/disc-settings/module-ui.so.dsp-next
test "$(sha256sum /usr/data/disc-settings/module-ui.so.dsp-next | cut -d' ' -f1)" = "$expected"
chmod 644 /usr/data/disc-settings/module-ui.so.dsp-next
mv /usr/data/disc-settings/module-ui.so.dsp-next /usr/data/disc-settings/module-ui.so
printf '1.10.220\n' > /usr/data/disc-settings/VERSION.next
mv /usr/data/disc-settings/VERSION.next /usr/data/disc-settings/VERSION
sync
if [ "${DISC_DEFER_UI_RELOAD:-0}" = 1 ];then echo "Installed; reboot required to load the UAC fix";exit 0;fi
kill -TERM "$ui"
echo 'Online music host installed; controlled UI/player reload requested'
