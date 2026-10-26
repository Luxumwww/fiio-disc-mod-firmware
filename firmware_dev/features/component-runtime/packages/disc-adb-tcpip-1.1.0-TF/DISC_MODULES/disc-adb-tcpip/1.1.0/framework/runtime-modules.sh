#!/bin/sh
# Integrated modules reuse the existing runtime/adbd. No module daemon is spawned.
disc_tcpip_key='';disc_tcpip_release='';disc_tcpip_library='';disc_tcpip_loaded=''
disc_tcpip_enabled(){
 [ -f "$root/components/disc-adb-tcpip/enabled" ] &&
 [ ! -f "$root/components/disc-adb-tcpip/disabled" ] &&
 [ ! -f /tmp/sdcard/DISC_MODULES_SAFE_MODE ] &&
 [ -n "$disc_tcpip_library" ]
}
runtime_modules_refresh(){
 tcpip_module_changed=0
 dir=$root/components/disc-adb-tcpip
 link=$(readlink "$dir/current" 2>/dev/null)
 key=''
 if [ -f "$dir/current/INTEGRATION" ];then
  read -r integration < "$dir/current/INTEGRATION"
  if [ "$integration" = runtime ] && [ -f "$dir/current/libdisc-adb-tcpip.so" ] &&
     [ -f "$dir/current/tcpip-control.sh" ];then key=$link;fi
 fi
 [ "$key" != "$disc_tcpip_key" ] || return 0
 disc_tcpip_release='';disc_tcpip_library=''
 if [ -n "$key" ];then
  release=$(readlink -f "$dir/current")
  case "$release" in
   "$root/components/disc-adb-tcpip/releases/"*|/tmp/sdcard/DISC_MODULES/disc-adb-tcpip/*)
    if (cd "$release" && sha256sum -c SHA256SUMS >/dev/null 2>&1);then
     disc_tcpip_release=$release
     disc_tcpip_library=$release/libdisc-adb-tcpip.so
     . "$release/tcpip-control.sh"
    else key='';log 'tcpip module rejected: package integrity failed';fi;;
   *) key='';log 'tcpip module rejected: unowned path';;
  esac
 fi
 disc_tcpip_key=$key;tcpip_module_changed=1
 log "integrated tcpip module selected: ${key:-absent}"
}
runtime_modules_publish(){
 marker=$ram/integrated-disc-adb-tcpip
 if disc_tcpip_enabled && [ "$disc_tcpip_loaded" = "$disc_tcpip_key" ] &&
    { [ "$usb_active" = 1 ] || [ "$wireless_active" = 1 ]; };then
  # Marker is RAM only and represents this daemon's actually applied release.
  applied='';[ ! -f "$marker" ] || read -r applied < "$marker"
  [ "$applied" = "$disc_tcpip_key" ] || printf '%s\n' "$disc_tcpip_key" > "$marker"
 else [ ! -f "$marker" ] || rm -f "$marker";fi
}
