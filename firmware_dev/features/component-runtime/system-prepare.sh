#!/bin/sh
# Runs synchronously BEFORE stock UI starts. Read-only firmware seed survives reset.
unset LD_PRELOAD
umask 077
seed=/usr/project/disc-system-v1
root=/usr/data/disc-runtime
[ -d "$seed" ] && (cd "$seed" && sha256sum -c SHA256SUMS >/tmp/disc-system-integrity.log 2>&1) || exit 1
mkdir -p "$root/components" "$root/incoming" /usr/data/disc-settings /usr/data/lib || exit 1
# Reset/missing service is reconstructed; retain an intact user-updated service.
if [ "$(cat "$root/service/API" 2>/dev/null)" != "$(cat "$seed/service/API")" ] || ! (cd "$root/service" && sha256sum -c SHA256SUMS >/dev/null 2>&1);then
 [ ! -e "$root/service" ] || mv "$root/service" "$root/service-replaced-$(cut -d. -f1 /proc/uptime)" || exit 1
 cp -a "$seed/service" "$root/service.new" && mv "$root/service.new" "$root/service" || exit 1
fi
# Roll back any interrupted restore before stock UI/player open their databases.
if [ -d /usr/data/.disc-personal-restore ];then "$root/service/personal-backup.sh" recover || exit 1;fi
if [ ! -f "$root/debug.conf" ];then
 printf 'usb=0\nwireless=0\n' > "$root/debug.conf.new" && mv "$root/debug.conf.new" "$root/debug.conf" || exit 1
fi
# Explicit manual disable survives reboot; factory reset removes this marker.
[ ! -f /usr/data/disc-settings.disabled ] || exit 0
bridge=/usr/data/lib/librt.so.1
expected=$(sha256sum "$seed/librt.so.1" | cut -d' ' -f1)
if [ -e "$bridge" ] && [ "$(sha256sum "$bridge" | cut -d' ' -f1)" != "$expected" ];then
 echo 'Unowned loader bridge retained; menu restoration skipped' >&2;exit 1
fi
if [ "$(cat /usr/data/disc-settings/VERSION 2>/dev/null)" != "$(cat "$seed/VERSION")" ] || [ "$(sha256sum /usr/data/disc-settings.so 2>/dev/null | cut -d' ' -f1)" != "$(sha256sum "$seed/disc-settings.so" | cut -d' ' -f1)" ] || [ ! -f "$bridge" ];then
 cp "$seed/disc-settings.so" /usr/data/disc-settings.so.new && mv /usr/data/disc-settings.so.new /usr/data/disc-settings.so || exit 1
 cp "$seed/librt.so.1" "$bridge.new" && mv "$bridge.new" "$bridge" || exit 1
 cp "$seed/VERSION" /usr/data/disc-settings/VERSION || exit 1
fi
cp "$seed/uninstall.sh" /usr/data/disc-settings/uninstall.sh || exit 1
: > /usr/data/disc-settings.enabled
dir=$root/components/usb-settings
mkdir -p "$dir/releases/builtin" || exit 1
cp "$seed/guard.sh" "$dir/releases/builtin/guard.sh"
printf 'guard.sh\n' > "$dir/releases/builtin/ENTRY"
printf 'service\n' > "$dir/releases/builtin/KIND"
ln -s releases/builtin "$dir/current.builtin-new" && "$root/service/atomic-rename" "$dir/current.builtin-new" "$dir/current" || exit 1
: > "$dir/enabled"
sync
