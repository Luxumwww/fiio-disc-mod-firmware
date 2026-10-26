#!/bin/sh
# Sourced status checks. 'active' means daemon + endpoint/listener actually ready.
debug_owned_profile(){
 dbg_pid=$1;debug_profile=''
 dbg_exe=$(readlink /proc/$dbg_pid/exe 2>/dev/null)
 case "$dbg_exe" in
  "$service/profiles/usb/adbd") debug_profile=usb;return 0;;
  "$service/profiles/both/adbd") debug_profile=both;return 0;;
  "$service/profiles/wireless/adbd") debug_profile=wireless;return 0;;
  "$root"/bt-before-*/service/profiles/*/adbd|"$root"/service-before-tcpip-module-*/profiles/*/adbd) :;;
  *) return 1;;
 esac
 dbg_parent=${dbg_exe%/adbd};dbg_candidate=${dbg_parent##*/}
 case "$dbg_candidate" in usb|both|wireless) :;;*) return 1;;esac
 dbg_born=$(awk '{print $22}' /proc/$dbg_pid/stat 2>/dev/null)
 dbg_key=$dbg_pid:$dbg_born:$dbg_exe
 if [ "$dbg_key" != "$dbg_cache_key" ];then
  dbg_cache_key=$dbg_key;dbg_cache_profile=''
  dbg_hash=$(sha256sum /proc/$dbg_pid/exe 2>/dev/null | cut -d' ' -f1)
  dbg_expected=$(sha256sum "$service/profiles/$dbg_candidate/adbd" 2>/dev/null | cut -d' ' -f1)
  [ -z "$dbg_hash" ] || [ "$dbg_hash" != "$dbg_expected" ] || dbg_cache_profile=$dbg_candidate
 fi
 debug_profile=$dbg_cache_profile
 [ -n "$debug_profile" ]
}
debug_health(){
 usb_active=0;wireless_active=0;owned_adbd=''
 for q in $(pidof adbd);do
  if debug_owned_profile "$q";then owned_adbd="$owned_adbd $q";fi
 done
 if [ -n "$owned_adbd" ];then
  usb_fd=0;legacy_fd=0;tcp_fd=0
  tcp_inodes=$(awk '$2 ~ /:15B3$/ && $4=="0A" {print $10}' /proc/net/tcp)
  for q in $owned_adbd;do
   links=$(ls -l /proc/$q/fd 2>/dev/null)
   if printf '%s\n' "$links" | grep -Fq '/dev/usb-ffs/adb/ep1' &&
      printf '%s\n' "$links" | grep -Fq '/dev/usb-ffs/adb/ep2';then usb_fd=1;fi
   printf '%s\n' "$links" | grep -Fq '/dev/android_adb' && legacy_fd=1
   for inode in $tcp_inodes;do
    printf '%s\n' "$links" | grep -Fq "socket:[$inode]" && tcp_fd=1
   done
  done
  if [ "$usb_fd" = 1 ] && [ -e /dev/usb-ffs/adb/ep1 ] && [ -e /dev/usb-ffs/adb/ep2 ] &&
     [ -n "$(cat /sys/kernel/config/usb_gadget/disc_adb/UDC 2>/dev/null)" ] &&
     [ "$(cat /sys/class/usb_role/13500000.otg_new-role-switch/role 2>/dev/null)" = device ];then usb_active=1;fi
  if [ "$legacy_fd" = 1 ] && [ "$(cat /sys/class/android_usb/android0/enable 2>/dev/null)" = 1 ] &&
     [ "$(cat /sys/class/android_usb/android0/functions 2>/dev/null)" = adb ];then usb_active=1;fi
  [ "$tcp_fd" != 1 ] || wireless_active=1
 fi
}
