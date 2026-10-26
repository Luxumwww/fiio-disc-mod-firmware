
stop_owned(){
 for pf in /tmp/disc-runtime/manager.pid /tmp/disc-adb/manager.pid;do
  p=$(cat "$pf" 2>/dev/null)
  case "$p" in ''|*[!0-9]*) continue;;esac
  cmd=$(tr '\000' ' ' < /proc/$p/cmdline 2>/dev/null)
  case "$cmd" in *'/usr/data/disc-runtime/service/runtime.sh'*|*'/usr/data/disc-runtime/service/usb-manager.sh'*) kill -TERM "$p" 2>/dev/null;;*) continue;;esac
  n=0;while [ -d /proc/$p ] && [ "$n" -lt 12 ];do sleep 1;n=$((n+1));done
  [ ! -d /proc/$p ] || return 1
 done
 for p in $(pidof adbd);do
  exe=$(readlink /proc/$p/exe)
  case "$exe" in /usr/data/disc-runtime/service/profiles/*/adbd) kill "$p";sleep 1;;*) return 1;;esac
 done
 rmdir /tmp/disc-runtime/lock /tmp/disc-adb/lock 2>/dev/null || true
}
start_owned(){
 nohup setsid /usr/data/disc-runtime/service/runtime.sh >/tmp/disc-runtime-launch.log 2>&1 </dev/null &
}

set -e
stop_owned
chmod 755 /usr/data/disc-runtime/incoming/tcpip-20261005-173509/runtime.sh /usr/data/disc-runtime/incoming/tcpip-20261005-173509/tcpip-control.sh
chmod 644 /usr/data/disc-runtime/incoming/tcpip-20261005-173509/libdisc-adb-tcpip.so
mv /usr/data/disc-runtime/incoming/tcpip-20261005-173509/runtime.sh /usr/data/disc-runtime/service/runtime.sh
mv /usr/data/disc-runtime/incoming/tcpip-20261005-173509/tcpip-control.sh /usr/data/disc-runtime/service/tcpip-control.sh
mv /usr/data/disc-runtime/incoming/tcpip-20261005-173509/libdisc-adb-tcpip.so /usr/data/disc-runtime/service/libdisc-adb-tcpip.so

cd /usr/data/disc-runtime/service
find . -type f ! -name SHA256SUMS ! -name SHA256SUMS.new | sort | while read f;do sha256sum "$f";done > SHA256SUMS.new
mv SHA256SUMS.new SHA256SUMS
sync
start_owned
