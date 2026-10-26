#!/bin/sh
# Persistent monitor: idle/mode changes are not proof of a UI crash.
# Only explicit uninstall disables the extension; monitoring is read-only.
unset LD_PRELOAD LD_LIBRARY_PATH
root=/usr/data/disc-settings
bridge=/usr/data/lib/librt.so.1
activation=/usr/data/disc-settings.enabled
heartbeat=/tmp/disc-settings.heartbeat
last='';window=0;changes=0;started=0;state=''
now(){ cut -d. -f1 /proc/uptime; }
log(){
 [ ! -f "$root/guard.log" ] || [ "$(wc -c < "$root/guard.log")" -lt 262144 ] || return 0
 printf '%s monitor: %s\n' "$(now)" "$1" >> "$root/guard.log"
}
check(){
 if [ -f "$activation" ] && [ -f "$bridge" ];then
  t=$(now);p=$(pidof mq_ui)
  if [ "$started" = 0 ];then started=$t;window=$t;last=$p;fi
  if [ "$p" != "$last" ];then
   if [ $((t-window)) -gt 60 ];then window=$t;changes=0;fi
   changes=$((changes+1));last=$p
   if [ "$changes" = 3 ];then log 'UI process changed repeatedly; extension retained';fi
  fi
   hbpid='';hbtime=0
   [ ! -f "$heartbeat" ] || read -r hbpid hbtime < "$heartbeat"
   case "$hbtime" in ''|*[!0-9]*) hbtime=0;;esac
   if [ $((t-started)) -gt 55 ] && { [ -z "$p" ] || [ "$hbpid" != "$p" ] || [ $((t-hbtime)) -gt 45 ]; };then
    if [ "$state" != stale ];then log 'UI heartbeat missing; extension retained';state=stale;fi
   elif [ -n "$p" ] && [ "$hbpid" = "$p" ] && [ $((t-hbtime)) -le 45 ];then
    if [ "$state" = stale ];then log 'UI heartbeat resumed';fi
    state=healthy
   fi
 else last='';changes=0;started=0;state='';fi
}
# MAIN: tests source only the function definitions above this line.
mkdir -p "$root"
log 'v1.1.1 persistent monitor started (manual uninstall only)'
while :;do check;sleep 5;done
