#!/bin/sh
set -eu
script=$(realpath "$(dirname "$0")/tcpip-module-source/usb-dac-handoff.sh")
fixture=$(mktemp -d)
trap 'rm -rf "$fixture"' EXIT
. "$script"
disc_uac_gadgets=$fixture/gadgets;disc_uac_gadget=$disc_uac_gadgets/uac_demo
disc_uac_role=$fixture/role;disc_uac_lock=$fixture/lock;disc_uac_player=$fixture/player
ram=$fixture/ram;mkdir -p "$ram" "$disc_uac_gadget/functions/uac2.a" "$disc_uac_gadget/configs/c.1"
printf 0x2972 > "$disc_uac_gadget/idVendor";printf 0x0158 > "$disc_uac_gadget/idProduct"
printf 0x0200 > "$disc_uac_gadget/bcdUSB";printf 3 > "$disc_uac_gadget/functions/uac2.a/c_chmask"
printf 4 > "$disc_uac_gadget/functions/uac2.a/c_ssize";printf device > "$disc_uac_role"
ln -s ../../../../usb_gadget/uac_demo/functions/uac2.a "$disc_uac_gadget/configs/c.1/uac2.a"
: > "$disc_uac_gadget/UDC"
enabled=1;safe=0;work_mode=1;pause_reason=usbdac;ui_pid=123;t=10
disc_tcpip_enabled(){ [ "$enabled" = 1 ]; }
read_work_mode(){ :; }
rescue_active(){ [ "$safe" = 1 ]; }
log(){ printf '%s\n' "$*" >> "$ram/log"; }
sha256sum(){ printf 'a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc  %s\n' "$1"; }
empty(){ [ -z "$(cat "$disc_uac_gadget/UDC")" ]; }
reset(){ : > "$disc_uac_gadget/UDC";disc_uac_session='';disc_uac_empty_since='';disc_uac_tries=0;t=10; }
pair(){ disc_uac_handoff;t=12;disc_uac_handoff; }
pair;[ "$(cat "$disc_uac_gadget/UDC")" = 13500000.otg_new ];[ "$disc_uac_tries" = 1 ]
t=13;disc_uac_handoff;[ "$disc_uac_tries" = 1 ] # no rebind while already bound
reset;enabled=0;pair;empty;enabled=1
reset;safe=1;pair;empty;safe=0
reset;work_mode=0;pair;empty;work_mode=1
reset;pause_reason=usb_audio;pair;empty;pause_reason=usbdac
reset;mkdir "$disc_uac_lock";pair;empty;rmdir "$disc_uac_lock"
reset;printf host > "$disc_uac_role";pair;empty;printf device > "$disc_uac_role"
reset;mkdir "$disc_uac_gadgets/foreign";printf foreign-controller > "$disc_uac_gadgets/foreign/UDC";pair;empty
[ "$(cat "$disc_uac_gadgets/foreign/UDC")" = foreign-controller ];rm "$disc_uac_gadgets/foreign/UDC";rmdir "$disc_uac_gadgets/foreign"
reset;printf 0x18d1 > "$disc_uac_gadget/idVendor";pair;empty;printf 0x2972 > "$disc_uac_gadget/idVendor"
reset;printf 0x0157 > "$disc_uac_gadget/idProduct";pair;empty;printf 0x0158 > "$disc_uac_gadget/idProduct"
reset;printf 2 > "$disc_uac_gadget/functions/uac2.a/c_ssize";pair;empty;printf 4 > "$disc_uac_gadget/functions/uac2.a/c_ssize"
reset;disc_uac_handoff;t=40;disc_uac_handoff;empty # bounded20s window
reset;disc_uac_handoff;disc_uac_tries=3;t=12;disc_uac_handoff;empty
reset;disc_uac_handoff;t=11;disc_uac_handoff;empty # settle without blocking runtime
t=12;disc_uac_handoff;[ "$(cat "$disc_uac_gadget/UDC")" = 13500000.otg_new ]
printf '%s\n' 'PASS: native blank-UAC recovery, no rebind/no stealing, module/safe/mode/role/lock/vendor/product/format guards,2s settle,20s deadline,3-attempt cap'
