#!/bin/sh
# Sourced status checks. 'active' means daemon + endpoint/listener actually ready.
debug_owned_profile(){
 dbg_pid=$1;debug_profile=''
 # File identity tests avoid spawning readlink for unchanged owned profiles.
 # Moved releases still use the original path/hash verification below.
 for dbg_candidate in usb both wireless;do
  if [ /proc/$dbg_pid/exe -ef "$service/profiles/$dbg_candidate/adbd" ];then
   debug_profile=$dbg_candidate;return 0
  fi
 done
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
 adbd_pids=$(pidof adbd);debug_health_snapshot=1
 for q in $adbd_pids;do
  if debug_owned_profile "$q";then owned_adbd="$owned_adbd $q";fi
 done
 if [ -n "$owned_adbd" ];then
  usb_fd=0;legacy_fd=0;tcp_fd=0
  tcp_inodes=''
  while read -r slot local_address peer tcp_state queues timer retrans uid timeout inode tcp_rest;do
   case "$local_address:$tcp_state" in *:15B3:0A)
    case "$inode" in ''|*[!0-9]*) :;;*) tcp_inodes="$tcp_inodes $inode";;esac;;
   esac
  done < /proc/net/tcp
  for q in $owned_adbd;do
   if [ -x "$service/runtime-links" ];then
    links=$("$service/runtime-links" /proc/$q/fd/* 2>/dev/null) || links=$(ls -l /proc/$q/fd 2>/dev/null)
   else links=$(ls -l /proc/$q/fd 2>/dev/null);fi
   # Fixed-string shell matches retain the original readiness checks without
   # forking printf/grep pipelines on every one-second health tick.
   ep1_fd=0;ep2_fd=0
   case "$links" in *'/dev/usb-ffs/adb/ep1'*) ep1_fd=1;;esac
   case "$links" in *'/dev/usb-ffs/adb/ep2'*) ep2_fd=1;;esac
   [ "$ep1_fd:$ep2_fd" != 1:1 ] || usb_fd=1
   case "$links" in *'/dev/android_adb'*) legacy_fd=1;;esac
   for inode in $tcp_inodes;do
    case "$links" in *"socket:[$inode]"*) tcp_fd=1;;esac
   done
  done
  dbg_udc='';dbg_role='';dbg_legacy_enable='';dbg_legacy_functions=''
  if [ "$usb_fd" = 1 ];then
   [ ! -f /sys/kernel/config/usb_gadget/disc_adb/UDC ] ||
    read -r dbg_udc < /sys/kernel/config/usb_gadget/disc_adb/UDC
   [ ! -f /sys/class/usb_role/13500000.otg_new-role-switch/role ] ||
    read -r dbg_role < /sys/class/usb_role/13500000.otg_new-role-switch/role
  fi
  if [ "$legacy_fd" = 1 ];then
   [ ! -f /sys/class/android_usb/android0/enable ] ||
    read -r dbg_legacy_enable < /sys/class/android_usb/android0/enable
   [ ! -f /sys/class/android_usb/android0/functions ] ||
    read -r dbg_legacy_functions < /sys/class/android_usb/android0/functions
  fi
  if [ "$usb_fd" = 1 ] && [ -e /dev/usb-ffs/adb/ep1 ] && [ -e /dev/usb-ffs/adb/ep2 ] &&
     [ -n "$dbg_udc" ] && [ "$dbg_role" = device ];then usb_active=1;fi
  if [ "$legacy_fd" = 1 ] && [ "$dbg_legacy_enable" = 1 ] &&
     [ "$dbg_legacy_functions" = adb ];then usb_active=1;fi
  [ "$tcp_fd" != 1 ] || wireless_active=1
 fi
}
