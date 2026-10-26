#!/bin/sh
unset LD_PRELOAD
echo DISC_DIAGNOSTICS_VERSION=0.3
echo 'Crash hooks require UI1.10.29+, enabled module and a fresh UI process.'
cat /etc/product_version/version.in
echo MEMORY
head -3 /proc/meminfo
echo UI_AUDIO_PIDS
pidof mq_ui mq_player
echo CRASH_RECORDS
for f in /tmp/disc-crash-*.context; do test ! -f "$f" || cat "$f"; done
