#!/bin/sh
set -eu
here=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
fixture=$(mktemp -d)
trap 'rm -rf "$fixture"' EXIT
export DISC_TIME_TEST=1 DISC_TIME_DATA="$fixture/data" DISC_TIME_RAM="$fixture/ram" DISC_TIME_MODULE="$fixture/module"
mkdir -p "$DISC_TIME_DATA" "$DISC_TIME_RAM" "$DISC_TIME_MODULE"
. "$here/time-sync.sh"
fake=1583049600
calls=0;rtc_calls=0;auto_value=1;online=0;ntp_calls=0;ntp_success=0
clock_now(){ printf '%s\n' "$fake"; }
set_clock(){ calls=$((calls+1));fake=$1; }
write_rtc(){ rtc_calls=$((rtc_calls+1)); }
native_auto(){ printf '%s\n' "$auto_value"; }
network_ready(){ [ "$online" = 1 ]; }
resolve_server(){ resolved=203.107.6.88;resolved_by=223.6.6.6; }
ntp_apply(){
 ntp_calls=$((ntp_calls+1))
 if [ "$ntp_success" = 1 ] && [ "$1" = 106.55.184.199 ];then fake=1792109000;return 0;fi
 return 1
}
! valid_epoch '1583049600'
! valid_epoch '1792108800;touch bad'
! valid_epoch '99999999999999999999999'
! valid_epoch '2145916800'
valid_epoch 1792108800
valid_ipv4 106.55.184.199
! valid_ipv4 '106.55.184.999'
! valid_ipv4 '106.55.184.199;reboot'
! valid_ipv4 '106..184.199'
printf '%s\n' corrupt > "$data/last-good";read_saved;[ "$last_good" = 0 ]
store_saved 1792108800;read_saved;[ "$last_good" = 1792108800 ]
auto_value=0;step
[ "$fake:$calls:$ntp_calls" = 1583049600:0:0 ]
auto_value='';step;[ "$calls" = 0 ]
auto_value=1;step
[ "$fake:$calls:$rtc_calls:$ntp_calls" = 1792108800:1:1:0 ]
[ "$source" = saved_approximate ]
step;[ "$calls" = 1 ]
fake=$((fake+3600));checkpoint_at=0;step;read_saved;[ "$last_good" = "$fake" ]
online=1;next_try=0;ntp_success=0
step;[ "$result" = network_sync_failed ];[ "$failures" = 1 ];[ "$ntp_calls" = 4 ]
before=$ntp_calls;step;[ "$ntp_calls" = "$before" ]
next_try=0;ntp_success=1;step
[ "$source:$result:$last_good" = ntp:synchronized:1792109000 ]
[ "$ntp_calls" = 8 ];[ "$failures" = 0 ]
[ "$ntp_server:$ntp_address:$ntp_resolver" = ntp.tencent.com:106.55.184.199:direct ]
before=$ntp_calls;step;[ "$ntp_calls" = "$before" ]
# A rollback after successful NTP must bypass the six-hour sync interval.
fake=1583049600;step
[ "$source:$result:$last_good" = ntp:synchronized:1792109000 ]
[ "$calls" = 2 ];[ "$ntp_calls" = 12 ]
# A user selecting manual time must stop both recovery and network attempts.
auto_value=0;fake=1583049600;next_try=0;step
[ "$fake:$ntp_calls" = 1583049600:12 ]
# Production deadline mechanism must terminate its owned child without relying
# on the unavailable BusyBox timeout applet.
if bounded 1 sleep 5;then exit 1;else timeout_rc=$?;fi
[ "$timeout_rc" = 124 ]
[ -z "$child" ]
printf 'PASS corrupt checkpoint, manual/unknown fail-closed, offline RTC recovery, hourly persistence, NTP fallback/backoff, no duplicate requests, bounded child termination\n'
