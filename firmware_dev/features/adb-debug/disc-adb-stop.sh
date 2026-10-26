#!/bin/sh
# Disable for future boots and request restoration from the running manager.
umask 077
: > /usr/data/DISC_ADB_DISABLE || exit 1
disc_pid=$(cat /tmp/disc-adb/manager.pid 2>/dev/null)
case "$disc_pid" in ''|*[!0-9]*) exit 0;; esac
case "$(cat /proc/$disc_pid/cmdline 2>/dev/null | tr '\000' ' ')" in
 *disc-adb.sh*) kill -TERM "$disc_pid";;
esac
echo 'ADB disabled. Remove /usr/data/DISC_ADB_DISABLE and reboot to enable again.'
