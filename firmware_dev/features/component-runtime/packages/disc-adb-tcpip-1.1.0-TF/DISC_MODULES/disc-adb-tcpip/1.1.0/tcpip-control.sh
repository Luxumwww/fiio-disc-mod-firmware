#!/bin/sh
# Sourced by runtime: device-side command queue; no UI reload or shell wrapper.
tcpip_requests(){
 tcpip_restart=0
 [ -f "$ram/tcpip.request" ] || return 0
 mv "$ram/tcpip.request" "$ram/tcpip.processing" || return 0
 port=$(cat "$ram/tcpip.processing");rm -f "$ram/tcpip.processing"
 disc_tcpip_enabled || { log 'tcpip ignored: module disabled or missing';return 0; }
 [ "$port" = 5555 ] || { log 'tcpip rejected invalid request';return 0; }
 # Native adbd must finish writing and closing its reply before teardown.
 sleep 3
 disc_tcpip_enabled || { log 'tcpip ignored: module disabled while queued';return 0; }
 if rescue_active;then log 'tcpip deferred: TF USB rescue is active';return 0;fi
 tcpip_usb=1
 v=$(sed -n 's/^usb=\([01]\)$/\1/p' "$root/debug.conf" 2>/dev/null)
 [ "$v" != 0 ] || tcpip_usb=0
 : > /usr/data/disc-settings.wifi-enable
 printf 'usb=%s\nwireless=1\n' "$tcpip_usb" > "$root/debug.tcpip.new" &&
  mv "$root/debug.tcpip.new" "$root/debug.conf" || { log 'tcpip config save failed';return 1; }
 sync
 tcpip_restart=1
 log "native tcpip 5555 accepted; USB request preserved=$tcpip_usb"
}
