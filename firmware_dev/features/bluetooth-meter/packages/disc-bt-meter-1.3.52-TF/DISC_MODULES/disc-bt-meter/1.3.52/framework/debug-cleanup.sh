#!/bin/sh
# Sourced by the manager; bounded cleanup of abandoned, owned ADB objects only.
disc_debug_cleanup(){
 [ -e /dev/usb-ffs/adb/ep0 ] || [ -d /sys/kernel/config/usb_gadget/disc_adb ] || [ -d /tmp/disc-adb/lock ] || return 0
 ! pidof adbd >/dev/null 2>&1 || return 1
 p=$(cat /tmp/disc-adb/manager.pid 2>/dev/null)
 case "$p" in ''|*[!0-9]*) :;;*) [ ! -d /proc/$p ] || return 1;;esac
 gadget=/sys/kernel/config/usb_gadget/disc_adb
 if [ -d "$gadget" ];then
  [ "$(cat "$gadget/idVendor")" = 0x18d1 ] && [ "$(cat "$gadget/idProduct")" = 0x4ee7 ] || return 1
  [ -d "$gadget/functions/ffs.adb" ] || return 1
  link=$(readlink "$gadget/configs/c.1/adb" 2>/dev/null)
  case "$link" in ''|../../../../usb_gadget/disc_adb/functions/ffs.adb|"$gadget/functions/ffs.adb") :;;*) return 1;;esac
  [ -z "$(cat "$gadget/UDC")" ] || printf '\n' > "$gadget/UDC" || return 1
 fi
 if awk '$2=="/dev/usb-ffs/adb" {found=1;if($1!="adb"||$3!="functionfs")bad=1} END {exit !found||bad}' /proc/mounts;then
  tries=0
  while ! umount /dev/usb-ffs/adb;do tries=$((tries+1));[ "$tries" -lt 3 ] || return 1;sleep 1;done
 elif [ -e /dev/usb-ffs/adb/ep0 ];then return 1;fi
 if [ -d "$gadget" ];then
  [ ! -L "$gadget/configs/c.1/adb" ] || rm "$gadget/configs/c.1/adb"
  for d in functions/ffs.adb configs/c.1/strings/0x409 configs/c.1 strings/0x409;do
   [ ! -d "$gadget/$d" ] || rmdir "$gadget/$d" || return 1
  done
  rmdir "$gadget" || return 1
 fi
 [ ! -d /tmp/disc-adb/lock ] || rmdir /tmp/disc-adb/lock || return 1
 [ ! -e /dev/usb-ffs/adb/ep0 ]
}
