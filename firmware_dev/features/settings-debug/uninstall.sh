#!/bin/sh
unset LD_PRELOAD LD_LIBRARY_PATH
bridge=/usr/data/lib/librt.so.1
expected=1ccdf3b3ceb637bcc2ca6a72a8e37bf8d5008c6c532cb160f80e179c368f08fb
mkdir -p /usr/data/disc-settings
: > /usr/data/disc-settings.disabled
rm -f /usr/data/disc-settings.enabled /usr/data/disc-settings.open
if [ -f "$bridge" ];then
 [ "$(sha256sum "$bridge" | cut -d' ' -f1)" = "$expected" ] || { echo 'Refusing to replace an unowned library';exit 1; }
 mv "$bridge" /usr/data/disc-settings/disabled-librt.so.1 || exit 1
fi
/usr/data/disc-runtime/service/componentctl.sh disable usb-settings 2>/dev/null || true
sync
p=$(pidof mq_ui);[ -z "$p" ] || kill -TERM $p
echo 'USB settings extension disabled; stock UI will reload. Debug configuration preserved.'
