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
 runtime_bt_refresh
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
disc_bt_key='';disc_bt_release='';disc_bt_pid='';disc_bt_ready=0;disc_bt_decoder_pid='';disc_bt_decoder_ok=0
runtime_bt_refresh(){
 dir=$root/components/disc-bt-meter
 key=$(readlink "$dir/current" 2>/dev/null)
 if [ "$key" != "$disc_bt_key" ];then
  disc_bt_release='';disc_bt_ready=0
  release=$(readlink -f "$dir/current" 2>/dev/null)
  case "$release" in "$root/components/disc-bt-meter/releases/"*|/tmp/sdcard/DISC_MODULES/disc-bt-meter/*)
   if [ "$(cat "$release/INTEGRATION" 2>/dev/null)" = ui ] &&
      (cd "$release" && sha256sum -c SHA256SUMS >/dev/null 2>&1);then
    disc_bt_release=$release
    if [ -f "$release/bridge/librt-ui-loader.so.1" ] && [ -f /usr/data/disc-settings/module-ui.so ] &&
       [ "$(sha256sum /usr/data/lib/librt.so.1 | cut -d' ' -f1)" = "$(sha256sum "$release/bridge/librt-ui-loader.so.1" | cut -d' ' -f1)" ] &&
       [ "$(sha256sum /usr/data/disc-settings/module-ui.so | cut -d' ' -f1)" = "$(sha256sum "$release/bridge/disc-settings.so" | cut -d' ' -f1)" ];then disc_bt_ready=1;fi
   fi;;
  esac
  disc_bt_key=$key
 fi
 if [ "$disc_bt_ready" = 1 ] && [ -n "$disc_bt_release" ] && [ -f "$dir/enabled" ] && [ ! -f "$dir/disabled" ] && [ ! -f /tmp/sdcard/DISC_MODULES_SAFE_MODE ];then
  path=$disc_bt_release/libdisc-bt-meter.so
  applied='';[ ! -f "$ram/bt-module.path" ] || read -r applied < "$ram/bt-module.path"
  if [ "$applied" != "$path" ];then printf '%s\n' "$path" > "$ram/bt-module.path.new";mv "$ram/bt-module.path.new" "$ram/bt-module.path";fi
  p=$(pidof mq_player 2>/dev/null)
  if [ "$p" != "$disc_bt_pid" ];then
   rm -f "$ram/bt-player";disc_bt_pid=$p
   if [ -n "$p" ] && [ "$(readlink /proc/$p/exe)" = /usr/bin/mq_player ] &&
      [ "$(sha256sum /usr/bin/mq_player | cut -d' ' -f1)" = a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc ];then
    printf '%s\n' "$p" > "$ram/bt-player.new";mv "$ram/bt-player.new" "$ram/bt-player"
   fi
  fi
  # Metadata only: identify the existing decoder, never open an audio stream.
  bp=$(pidof bluealsa 2>/dev/null)
  if [ "$bp" != "$disc_bt_decoder_pid" ];then
   disc_bt_decoder_pid=$bp;disc_bt_decoder_ok=0
   if [ -n "$bp" ] && [ "$(readlink /proc/$bp/exe)" = /usr/bin/bluealsa ] &&
      [ "$(sha256sum /usr/bin/bluealsa | cut -d' ' -f1)" = 773320d3c61f35ece83becdada68e7417068e81dbd48b22bedc63a985404dd9d ] &&
      [ "$(sha256sum /usr/lib/libldacBT_enc.so.2.0.2.3 | cut -d' ' -f1)" = 9b86b29fca2b2ab5dde45386ad4b190768109c05fa19be6072c538be1727f66d ];then disc_bt_decoder_ok=1;fi
  fi
  if [ "$disc_bt_decoder_ok" = 1 ];then
   printf '%s\n' "$bp" > "$ram/bt-decoder.new"
   for task in /proc/$bp/task/*;do
    name='';read -r name < "$task/comm" || continue
    case "$name" in ba-a2dp*) printf '%s\n' "${task##*/}" >> "$ram/bt-decoder.new";;esac
   done
   mv "$ram/bt-decoder.new" "$ram/bt-decoder"
  else rm -f "$ram/bt-decoder";fi
 else
  [ ! -f "$ram/bt-module.path" ] || rm -f "$ram/bt-module.path"
  [ ! -f "$ram/integrated-disc-bt-meter" ] || rm -f "$ram/integrated-disc-bt-meter"
  disc_bt_pid=''
  rm -f "$ram/bt-decoder"
 fi
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
