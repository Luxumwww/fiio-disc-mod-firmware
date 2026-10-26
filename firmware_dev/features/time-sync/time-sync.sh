#!/bin/sh
# Clock rollback recovery. No UI, audio, system resolver, native daemon or DB writes.
unset LD_PRELOAD
umask 077
data=${DISC_TIME_DATA:-/usr/data/disc-runtime/module-data/disc-time-sync}
ram=${DISC_TIME_RAM:-/tmp/disc-runtime}
module=${DISC_TIME_MODULE:-/usr/data/disc-runtime/components/disc-time-sync}
db=/usr/data/fiio/db/sysconfig.db
MIN_EPOCH=1767225600
MAX_EPOCH=2145916800
last_good=0
child=''
source=unknown
result=starting
failures=0
next_try=0
checkpoint_at=0
last_ntp=0
auto=unknown
ntp_server=''
ntp_address=''
ntp_resolver=''
clock_now(){ date +%s; }
uptime_now(){ read -r tick idle < /proc/uptime; printf '%s\n' "${tick%%.*}"; }
valid_epoch(){
 case "$1" in ''|*[!0-9]*) return 1;;esac
 [ "${#1}" -eq 10 ] && [ "$1" -ge "$MIN_EPOCH" ] && [ "$1" -lt "$MAX_EPOCH" ]
}
read_saved(){
 last_good=0
 if [ -f "$data/last-good" ];then
  IFS= read -r saved < "$data/last-good"
  if valid_epoch "$saved";then last_good=$saved;fi
 fi
}
store_saved(){
 valid_epoch "$1" || return 1
 # Clock recovery data is separate from the immutable component release.
 printf '%s\n' "$1" > "$data/last-good.new" &&
  mv "$data/last-good.new" "$data/last-good" || return 1
 last_good=$1
}
native_auto(){ sqlite3 -readonly "$db" 'SELECT AUTO_TIME FROM SYSCONFIG ORDER BY ID LIMIT 1;' 2>/dev/null; }
set_clock(){
 valid_epoch "$1" || return 1
 stamp=$(date -u -d "@$1" '+%Y-%m-%d %H:%M:%S') || return 1
 date -u -s "$stamp" >/dev/null
}
write_rtc(){ hwclock -w -l >/dev/null 2>&1; }
recover_clock(){
 current=$(clock_now)
 if valid_epoch "$last_good" && { ! valid_epoch "$current" || [ "$current" -lt "$((last_good-120))" ]; };then
  if set_clock "$last_good";then
   [ "$source" != ntp ] || next_try=0
   source=saved_approximate
   result=restored_waiting_network
   write_rtc || result=restored_rtc_write_failed
  else result=restore_failed;fi
 fi
}
network_ready(){ awk '$2=="00000000" {ok=1} END {exit !ok}' /proc/net/route; }
bounded(){
 limit=$1;shift
 "$@" & child=$!
 began=$(uptime_now)
 while kill -0 "$child" 2>/dev/null;do
  current_tick=$(uptime_now)
  if [ "$((current_tick-began))" -ge "$limit" ];then
   kill -TERM "$child" 2>/dev/null || true
   sleep 1
   kill -KILL "$child" 2>/dev/null || true
   wait "$child" 2>/dev/null || true
   child='';return 124
  fi
  sleep 1
 done
 wait "$child"; child_rc=$?;child='';return "$child_rc"
}
valid_ipv4(){
 case "$1" in ''|*[!0-9.]*|.*|*.) return 1;;esac
 (IFS=.;set -- $1;[ "$#" = 4 ] || exit 1
  for part do case "$part" in ''|0[0-9]*) exit 1;;esac;[ "$part" -le 255 ] || exit 1;done)
}
resolve_server(){
 resolved='';resolved_by=''
 # Explicit resolver is local to this query; never rewrite resolv.conf.
 for dns in 223.5.5.5 223.6.6.6;do
  if bounded 8 busybox nslookup "$1" "$dns" > "$ram/time-sync.dns" 2>/dev/null;then
   candidates=$(awk '/^Name:/ {answer=1;next} answer && /^Address: / {print $2}' "$ram/time-sync.dns")
   for candidate in $candidates;do
    if valid_ipv4 "$candidate";then resolved=$candidate;resolved_by=$dns;return 0;fi
   done
  fi
 done
 return 0
}
ntp_apply(){ bounded 8 ntpdate -b -u -p 1 -t 2 "$1" > "$ram/time-sync.ntp" 2>&1; }
sync_host(){
 host=$1;shift
 [ "$(native_auto)" = 1 ] || return 1
 resolve_server "$host"
 # Direct addresses also permit recovery if both explicit DNS queries fail.
 for address in "$resolved" "$@";do
  valid_ipv4 "$address" || continue
  case " ${tried:-} " in *" $address "*) continue;;esac
  tried="${tried:-} $address"
  [ "$(native_auto)" = 1 ] || return 1
  if ntp_apply "$address";then
   current=$(clock_now)
   if valid_epoch "$current";then
    source=ntp;result=synchronized;failures=0
    ntp_server=$host;ntp_address=$address;ntp_resolver=direct
    [ "$address" != "$resolved" ] || ntp_resolver=$resolved_by
    last_ntp=$(uptime_now)
    store_saved "$current" || result=sync_save_failed
    write_rtc || result=sync_rtc_write_failed
    next_try=$((last_ntp+21600))
    checkpoint_at=$((last_ntp+3600))
    return 0
   fi
  fi
 done
 return 1
}
sync_network(){
 tried=''
 # Cloudflare documents both anycast addresses. Tencent/Ali remain fallbacks.
 if sync_host time.cloudflare.com 162.159.200.1 162.159.200.123 ||
    sync_host ntp.tencent.com 106.55.184.199 ||
    sync_host ntp.aliyun.com 203.107.6.88;then tried='';return 0;fi
 tried='';failures=$((failures+1));result=network_sync_failed
 tick=$(uptime_now)
 if [ "$failures" -le 3 ];then next_try=$((tick+60));else next_try=$((tick+300));fi
 return 1
}
publish(){
 current=$(clock_now);tick=$(uptime_now)
 {
  printf 'version=0.1.2\npid=%s\nauto=%s\nsource=%s\nresult=%s\n' "$$" "$auto" "$source" "$result"
  printf 'ntp_server=%s\nntp_address=%s\nntp_resolver=%s\n' "$ntp_server" "$ntp_address" "$ntp_resolver"
  printf 'epoch=%s\nlast_good=%s\nupdated_uptime=%s\nlast_ntp_uptime=%s\nnext_try=%s\nfailures=%s\n' "$current" "$last_good" "$tick" "$last_ntp" "$next_try" "$failures"
 } > "$ram/time-sync.state.new"
 mv "$ram/time-sync.state.new" "$ram/time-sync.state"
}
step(){
 auto=$(native_auto)
 if [ "$auto" != 1 ];then
  # Unknown/read failure is also fail-closed. A manual clock is never restored.
  result=manual_or_unavailable;next_try=0;publish;return
 fi
 recover_clock
 tick=$(uptime_now)
 if [ "$tick" -ge "$next_try" ];then
  if network_ready;then sync_network || true
  else result=waiting_network;next_try=$((tick+30));fi
 fi
 tick=$(uptime_now);current=$(clock_now)
 if [ "$tick" -ge "$checkpoint_at" ];then
  if valid_epoch "$last_good" && valid_epoch "$current" && [ "$current" -gt "$last_good" ];then
   store_saved "$current" || result=checkpoint_failed
  fi
  checkpoint_at=$((tick+3600))
 fi
 publish
}
cleanup(){
 [ -z "$child" ] || { kill -TERM "$child" 2>/dev/null || true;wait "$child" 2>/dev/null || true; }
 rmdir "$ram/time-sync.lock" 2>/dev/null || true
}
main(){
 mkdir -p "$data" "$ram" || exit 1
 mkdir "$ram/time-sync.lock" 2>/dev/null || exit 0
 trap cleanup EXIT
 trap 'exit 0' TERM INT
 read_saved
 while [ -f "$module/enabled" ] && [ ! -f "$module/disabled" ];do
  step
  sleep 30 & child=$!;wait "$child";child=''
 done
}
# Unit fixtures source functions with replacement clocks/commands only.
[ "${DISC_TIME_TEST:-0}" = 1 ] || main
