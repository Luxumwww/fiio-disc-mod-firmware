#!/bin/sh
unset LD_PRELOAD
set -eu
umask 077
root=/usr/data/disc-runtime
ram=/tmp/disc-runtime
release=$(pwd -P)
case "$release" in "$root/components/disc-runtime-perf/releases/"*|/tmp/sdcard/DISC_MODULES/disc-runtime-perf/*) :;;*) exit 2;;esac
test "$(cat "$root/service/API")" = 6
sha256sum -c SHA256SUMS >/dev/null
(cd "$root/service" && sha256sum -c SHA256SUMS >/dev/null)
test ! -d "$ram/personal-lock"
test ! -f "$ram/personal-command"
test ! -f "$ram/module-command"
names='runtime.sh module-state.sh wifi-catalog.sh personal-state.sh'
for name in $names;do
 read -r oldhash newhash < "$name.hashes"
 if [ -f "$root/service/$name" ];then
  current=$(sha256sum "$root/service/$name" | cut -d' ' -f1)
  test "$current" = "$oldhash" || test "$current" = "$newhash"
 else test "$oldhash" = -;fi
 test "$(sha256sum "$name" | cut -d' ' -f1)" = "$newhash"
 test ! -e "$root/service/$name.perf-next"
 cp "$name" "$root/service/$name.perf-next"
 chmod 755 "$root/service/$name.perf-next"
 case "$name" in *.sh) sh -n "$root/service/$name.perf-next";;esac
done
# Exercise only the new read-only helper before stopping the manager.
"$root/service/runtime-links" "$root/components/disc-runtime-perf/current" > "$ram/runtime-links-check"
test "$(cat "$ram/runtime-links-check")" = "$root/components/disc-runtime-perf/current|releases/1.0.5"
test ! -e "$root/service/SHA256SUMS.perf-next"
awk 'NR==FNR {hash[$2]=$1;next} {n=$2;sub(/^\.\//,"",n);if(n in hash)next;print} END {for(n in hash)print hash[n] "  " n}' service-hashes "$root/service/SHA256SUMS" > "$root/service/SHA256SUMS.perf-next"
read -r oldpid < "$ram/manager.pid"
case "$oldpid" in ''|*[!0-9]*) exit 4;;esac
cmd=$(tr '\000' ' ' < "/proc/$oldpid/cmdline")
case "$cmd" in '/bin/sh /usr/data/disc-runtime/service/runtime.sh'*) :;;*) exit 4;;esac
test -d /proc/$oldpid
stopped=0;started=0
finish(){
 if [ "$stopped" = 1 ] && [ "$started" = 0 ] && [ ! -d /proc/$oldpid ];then
  rmdir "$ram/lock" 2>/dev/null || true
  nohup setsid "$root/service/runtime.sh" > /tmp/disc-runtime-launch.log 2>&1 </dev/null &
 fi
}
trap finish EXIT
trap 'exit 130' HUP INT TERM
kill -TERM "$oldpid";stopped=1
n=0;while [ -d /proc/$oldpid ] && [ "$n" -lt 10 ];do sleep 1;n=$((n+1));done
test ! -d /proc/$oldpid
for name in $names;do mv "$root/service/$name.perf-next" "$root/service/$name";done
mv "$root/service/SHA256SUMS.perf-next" "$root/service/SHA256SUMS"
(cd "$root/service" && sha256sum -c SHA256SUMS >/dev/null)
sync
rmdir "$ram/lock" 2>/dev/null || true
nohup setsid "$root/service/runtime.sh" > /tmp/disc-runtime-launch.log 2>&1 </dev/null &
started=1
n=0;ready=0
while [ "$n" -lt 10 ];do
 sleep 1;n=$((n+1));read -r newpid < "$ram/manager.pid"
 if [ "$newpid" != "$oldpid" ] && [ -d /proc/$newpid ];then ready=1;break;fi
done
test "$ready" = 1
printf 'runtime optimization 1.0.5 applied; only manager reloaded\n'
