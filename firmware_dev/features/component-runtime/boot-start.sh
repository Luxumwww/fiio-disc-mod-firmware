#!/bin/sh
unset LD_PRELOAD
umask 077
seed=/usr/project/disc-runtime-v1
root=/usr/data/disc-runtime
# Let the original init start its UI/audio/USB services before debug adoption.
sleep 20
if [ ! -e "$root" ];then
 mkdir -m 700 "$root" || exit 1
 mkdir -m 700 "$root/components" "$root/incoming" || exit 1
 cp -a "$seed" "$root/service" || exit 1
fi
if [ -d "$root/service" ] && [ -f "$root/service/SHA256SUMS" ] &&
 (cd "$root/service" && sha256sum -c SHA256SUMS >/tmp/disc-runtime-integrity.log 2>&1);then
 exec "$root/service/runtime.sh"
fi
# Corrupt/missing editable service: retain an independent USB recovery entry.
echo 'runtime integrity failed; using builtin USB debug' > /tmp/disc-runtime-launch.log
DISC_ADBD_BIN=/usr/bin/adbd DISC_ADB_POLICY= exec "$seed/usb-manager.sh"
