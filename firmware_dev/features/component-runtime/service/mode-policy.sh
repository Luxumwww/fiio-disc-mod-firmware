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
 # A pure/BT USB-output request must release our device gadget before native
 # output selection and AW35615 host power. Only the current enabled UI owns it.
 if { [ "$work_mode" = 0 ] || [ "$work_mode" = 3 ]; } && [ -f "$ram/bt-usb.request" ] &&
    [ "${disc_bt_ready:-0}" = 1 ] && [ -f "$root/components/disc-bt-meter/enabled" ] &&
    [ ! -f "$root/components/disc-bt-meter/disabled" ];then
  bt_usb_ui='';read -r bt_usb_ui < "$ram/bt-usb.request"
  if [ "$bt_usb_ui" = "$ui_pid" ];then
   case "$work_mode" in 0) pause_reason=pure_usb_audio;;3) pause_reason=bt_usb_audio;;esac
  fi
 fi
 # An on-demand USB tool holds the physical port through its cleanup. The
 # worker retains this reservation until source power and host role are off.
 if [ "$work_mode" = 0 ] && [ -f "$ram/usb-tool.request" ];then
  tool_ui='';tool_token='';tool_worker='';tool_extra=''
  read -r tool_ui tool_token tool_worker tool_extra < "$ram/usb-tool.request"
  case "$tool_ui:$tool_token:$tool_worker" in ''|*[!0-9:]*|:*|*::*) :;;*)
   if [ -z "$tool_extra" ] && [ -n "$tool_worker" ] && [ "$tool_worker" != 0 ] && kill -0 "$tool_worker" 2>/dev/null &&
      grep -q '/components/disc-usb-tools/releases/[^/]*/libthermal-worker.so' "/proc/$tool_worker/maps" 2>/dev/null;then
    pause_reason=usb_tool
   fi;;esac
 fi
 if debug_usb_priority && [ "$pause_reason" != usb_tool ];then pause_reason='';fi
}
debug_usb_priority(){
 debug_priority_key='';debug_priority_state='';debug_priority_error=''
 priority_enabled=${requested_usb:-}
 if [ -z "$priority_enabled" ];then
  priority_enabled=0;while IFS= read -r priority_config;do case "$priority_config" in usb=1) priority_enabled=1;;usb=0) priority_enabled=0;;esac;done < "$root/debug.conf"
 fi
 [ "$priority_enabled" = 1 ] && [ -f "$ram/debug-usb-priority" ] || return 1
 priority_pid='';priority_mode='';priority_at='';read -r priority_pid priority_mode priority_at < "$ram/debug-usb-priority"
 case "$priority_pid:$priority_mode:$priority_at" in *[!0-9:]*|::*|*:) return 1;;esac
 [ "$priority_pid" = "$ui_pid" ] && [ "$priority_mode" = "$work_mode" ] || return 1
 priority_ack='';[ ! -f "$ram/debug-usb-priority.ack" ] || read -r priority_ack < "$ram/debug-usb-priority.ack"
 debug_priority_key="$priority_pid:$priority_mode:$priority_at"
 if [ "$priority_ack" = "$priority_pid $priority_mode $priority_at" ];then debug_priority_state=active;return 0;fi
 debug_priority_state=pending
 read -r priority_now priority_idle < /proc/uptime;priority_now=${priority_now%%.*}
 [ "$priority_now" -lt "$((priority_at+15))" ] || debug_priority_error=audio_release_timeout
 return 1
}
rescue_active(){ [ -f /tmp/sdcard/DISC_DEBUG_USB_ENABLE ]; }
choose_mode(){
 rescue=0;requested_usb=$usb;requested_wifi=$wifi
 read_work_mode
 if rescue_active;then rescue=1;usb=1;wifi=0;pause_reason=''
 elif [ "$pause_reason" = usb_tool ];then usb=0
 elif debug_usb_priority;then usb=1;pause_reason=''
 elif [ -n "$pause_reason" ];then usb=0;fi
 case "$usb:$wifi" in 1:0) desired=usb;;1:1) desired=both;;0:1) desired=wireless;;0:0) desired=off;;esac
}
