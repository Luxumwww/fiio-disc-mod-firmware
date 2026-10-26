
set -e
root=/usr/data/disc-runtime
stage=$root/incoming/service-modules-v1.3-1791184570
backup=$root/service-before-modules-v1.3-1791184570
cd "$stage"
sha256sum -c SHA256SUMS
chmod 755 *.sh atomic-rename ui-mode profiles/*/adbd wifi-tools/wpa_passphrase
chmod 644 libdisc-adb-policy.so SHA256SUMS
test "$(./ui-mode $(pidof mq_ui))" != ''
old=$(cat /tmp/disc-runtime/manager.pid)
case "$old" in ''|*[!0-9]*) exit 8;;esac
cmd=$(tr '\000' ' ' < /proc/$old/cmdline)
case "$cmd" in *'/usr/data/disc-runtime/service/runtime.sh'*) :;;*) exit 8;;esac
kill -TERM "$old"
n=0;while [ -d /proc/$old ] && [ "$n" -lt 10 ];do sleep 1;n=$((n+1));done
test ! -d /proc/$old
cd "$root"
mv service "$backup"
if ! mv "$stage" service;then mv "$backup" service;exit 9;fi
sync
rmdir /tmp/disc-runtime/lock 2>/dev/null || true
nohup setsid "$root/service/runtime.sh" >/tmp/disc-runtime-launch.log 2>&1 </dev/null &
sleep 2
[ -f /tmp/disc-runtime/manager.pid ] && kill -0 "$(cat /tmp/disc-runtime/manager.pid)"
echo 'service update started; previous service preserved'
