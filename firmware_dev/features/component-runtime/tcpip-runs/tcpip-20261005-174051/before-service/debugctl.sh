#!/bin/sh
unset LD_PRELOAD
umask 077
root=/usr/data/disc-runtime
conf=$root/debug.conf
read_conf() {
 usb=1;wireless=0
 [ ! -f "$conf" ] || {
  v=$(sed -n 's/^usb=\([01]\)$/\1/p' "$conf");[ "$v" != 0 ] || usb=0
  v=$(sed -n 's/^wireless=\([01]\)$/\1/p' "$conf");[ "$v" != 1 ] || wireless=1
 }
}
read_conf
case "$1" in
 status)
  printf 'usb=%s\nwireless=%s\n' "$usb" "$wireless"
  ip -4 addr show wlan0 | awk '/inet / {split($2,a,"/");print "ip="a[1]}'
  printf 'allowed_host=';cat "$root/allowed-host-ipv4" 2>/dev/null
  [ ! -f /tmp/disc-runtime/state ] || cat /tmp/disc-runtime/state
  exit 0;;
 set)
  case "$2:$3" in 0:0|0:1|1:0|1:1) :;; *) echo 'invalid switch state' >&2;exit 2;;esac
  if [ "$3" = 1 ]; then
   printf '%s\n' "$4" | awk -F. 'NF!=4 {exit 1} {for(i=1;i<=4;i++)if($i!~/^[0-9]+$/||$i>255||length($i)>3)exit 1}' || exit 2
   printf '%s\n' "$4" > "$root/allowed-host-ipv4.new"
   mv "$root/allowed-host-ipv4.new" "$root/allowed-host-ipv4" || exit 1
  fi
  printf 'usb=%s\nwireless=%s\n' "$2" "$3" > "$conf.new" && mv "$conf.new" "$conf" && sync
  echo 'saved; runtime applies in about 3 seconds';;
 *) echo 'usage: debugctl.sh status | set USB(0/1) WIRELESS(0/1) [computer_ipv4]' >&2;exit 2;;
esac
