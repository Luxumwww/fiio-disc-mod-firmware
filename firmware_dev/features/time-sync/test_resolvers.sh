#!/bin/sh
set -eu
here=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
fixture=$(mktemp -d)
trap 'rm -rf "$fixture"' EXIT
export DISC_TIME_TEST=1 DISC_TIME_RAM="$fixture"
. "$here/time-sync.sh"
dns_calls=0;dns_success=223.6.6.6
bounded(){
 dns_calls=$((dns_calls+1))
 [ "$1:$2:$3:$4" = 8:busybox:nslookup:time.cloudflare.com ]
 [ "$5" = "$dns_success" ] || return 1
 printf 'Server: %s\nAddress: %s:53\nName: time.cloudflare.com\nAddress: 2606:4700:f1::1\nAddress: 162.159.200.1\n' "$5" "$5"
}
resolve_server time.cloudflare.com
[ "$dns_calls:$resolved:$resolved_by" = 2:162.159.200.1:223.6.6.6 ]
dns_calls=0;dns_success=none
resolve_server time.cloudflare.com
[ "$dns_calls:$resolved:$resolved_by" = 2:: ]
native_auto(){ printf '1\n'; }
ntp_apply(){ [ "$1" = 162.159.200.1 ]; }
clock_now(){ printf '1792109000\n'; }
store_saved(){ :; }
write_rtc(){ :; }
sync_host time.cloudflare.com 162.159.200.1 162.159.200.123
[ "$ntp_server:$ntp_address:$ntp_resolver" = time.cloudflare.com:162.159.200.1:direct ]
printf 'PASS alternate DNS, IPv6 skipped, empty DNS result cleared, documented direct NTP fallback\n'
