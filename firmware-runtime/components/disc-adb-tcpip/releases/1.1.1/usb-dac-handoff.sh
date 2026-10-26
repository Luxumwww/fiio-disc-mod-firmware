#!/bin/sh
# Native UAC2 creation may race owned ADB teardown. Bind only a complete,
# currently unbound stock gadget after ADB has fully relinquished the port.
disc_uac_gadget=/sys/kernel/config/usb_gadget/uac_demo
disc_uac_gadgets=/sys/kernel/config/usb_gadget
disc_uac_role=/sys/class/usb_role/13500000.otg_new-role-switch/role
disc_uac_lock=/tmp/disc-adb/lock
disc_uac_player=/usr/bin/mq_player
disc_uac_session='';disc_uac_tries=0;disc_uac_empty_since='';disc_uac_next=0;disc_uac_deadline=0
disc_uac_handoff(){
 disc_tcpip_enabled || return 0
 read_work_mode
 if [ "$work_mode" != 1 ] || [ "$pause_reason" != usbdac ] || rescue_active;then
  disc_uac_session='';disc_uac_empty_since='';disc_uac_tries=0;return 0
 fi
 disc_uac_tick=${t:-0}
 if [ "$disc_uac_session" != "$ui_pid" ];then
  disc_uac_session=$ui_pid;disc_uac_tries=0;disc_uac_empty_since='';disc_uac_next=0;disc_uac_deadline=$((disc_uac_tick+20))
 fi
 [ "$disc_uac_tick" -le "$disc_uac_deadline" ] && [ "$disc_uac_tries" -lt 3 ] || return 0
 [ ! -d "$disc_uac_lock" ] || return 0
 [ -f "$disc_uac_gadget/UDC" ] || return 0
 [ -z "$(cat "$disc_uac_gadget/UDC")" ] || return 0
 [ "$(cat "$disc_uac_role" 2>/dev/null)" = device ] || return 0
 # Never unbind or steal another device function, including still-live ADB.
 for disc_uac_other in "$disc_uac_gadgets"/*;do
  [ -f "$disc_uac_other/UDC" ] || continue
  [ -z "$(cat "$disc_uac_other/UDC")" ] || return 0
 done
 [ "$(cat "$disc_uac_gadget/idVendor")" = 0x2972 ] &&
 [ "$(cat "$disc_uac_gadget/idProduct")" = 0x0158 ] &&
 [ "$(cat "$disc_uac_gadget/bcdUSB")" = 0x0200 ] &&
 [ "$(readlink "$disc_uac_gadget/configs/c.1/uac2.a")" = ../../../../usb_gadget/uac_demo/functions/uac2.a ] &&
 [ "$(cat "$disc_uac_gadget/functions/uac2.a/c_chmask")" = 3 ] &&
 [ "$(cat "$disc_uac_gadget/functions/uac2.a/c_ssize")" = 4 ] || return 0
 if [ -z "$disc_uac_empty_since" ];then disc_uac_empty_since=$disc_uac_tick;return 0;fi
 [ $((disc_uac_tick-disc_uac_empty_since)) -ge 2 ] && [ "$disc_uac_tick" -ge "$disc_uac_next" ] || return 0
 [ "$(sha256sum "$disc_uac_player" | cut -d' ' -f1)" = a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc ] || return 0
 disc_uac_tries=$((disc_uac_tries+1));disc_uac_next=$((disc_uac_tick+2))
 if printf '%s' 13500000.otg_new > "$disc_uac_gadget/UDC";then
  log "native UAC2 handoff completed attempt=$disc_uac_tries"
  printf 'result=bound\nattempts=%s\nui_pid=%s\n' "$disc_uac_tries" "$ui_pid" > "$ram/usb-dac-handoff.state"
 else
  log "native UAC2 handoff deferred attempt=$disc_uac_tries"
 fi
}
