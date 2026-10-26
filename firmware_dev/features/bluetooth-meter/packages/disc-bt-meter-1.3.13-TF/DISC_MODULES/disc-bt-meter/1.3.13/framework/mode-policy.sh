#!/bin/sh
# Sourced by runtime/USB manager. Does not write stock UI or mode settings.
stock_ui_sha=f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226
checked_ui_pid='';checked_ui_ok=0
read_work_mode(){
 work_mode=unknown;pause_reason=''
 ui_pid=$(pidof mq_ui)
 case "$ui_pid" in ''|*[!0-9]*) return 0;;esac
 if [ "$checked_ui_pid" != "$ui_pid" ];then
  checked_ui_pid=$ui_pid;checked_ui_ok=0
  if [ "$(readlink /proc/$ui_pid/exe)" = /usr/bin/mq_ui ] &&
     [ "$(sha256sum /proc/$ui_pid/exe | cut -d' ' -f1)" = "$stock_ui_sha" ];then checked_ui_ok=1;fi
 fi
 [ "$checked_ui_ok" = 1 ] || return 0
 work_mode=$("$service/ui-mode" "$ui_pid" 2>/dev/null) || work_mode=unknown
 case "$work_mode" in 1) pause_reason=usbdac;;2) pause_reason=usb_audio;;4) pause_reason=storage;;esac
 # BT receiver may temporarily use the physical port as a USB audio host.
 # A request belongs to this exact UI lifetime and this enabled module.
 if [ "$work_mode" = 3 ] && [ -f "$ram/bt-usb.request" ] &&
    [ "$disc_bt_ready" = 1 ] && [ -f "$root/components/disc-bt-meter/enabled" ] &&
    [ ! -f "$root/components/disc-bt-meter/disabled" ];then
  bt_usb_ui='';read -r bt_usb_ui < "$ram/bt-usb.request"
  [ "$bt_usb_ui" != "$ui_pid" ] || pause_reason=bt_usb_audio
 fi
}
rescue_active(){ [ -f /tmp/sdcard/DISC_DEBUG_USB_ENABLE ]; }
choose_mode(){
 rescue=0;requested_usb=$usb;requested_wifi=$wifi
 read_work_mode
 if rescue_active;then rescue=1;usb=1;wifi=0;pause_reason=''
 elif [ -n "$pause_reason" ];then usb=0;fi
 case "$usb:$wifi" in 1:0) desired=usb;;1:1) desired=both;;0:1) desired=wireless;;0:0) desired=off;;esac
}
