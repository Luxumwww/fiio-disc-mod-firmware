
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
cd /usr/data/disc-runtime
test -d /usr/data/disc-runtime/service-before-tcpip-20261005-173509
test ! -e service-failed-tcpip-20261005-173509
mv service service-failed-tcpip-20261005-173509
cp -a /usr/data/disc-runtime/service-before-tcpip-20261005-173509 service
cp /usr/data/disc-runtime/service-before-tcpip-20261005-173509/DEBUG_BEFORE debug.conf
cp /usr/data/disc-runtime/service-before-tcpip-20261005-173509/HOST_BEFORE allowed-host-ipv4
rm -f service/DEBUG_BEFORE service/HOST_BEFORE /tmp/disc-runtime/tcpip.request /tmp/disc-runtime/tcpip-20261005-173509.pending
sync
start_owned
