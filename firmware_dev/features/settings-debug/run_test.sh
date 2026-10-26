#!/bin/sh
set -eu
W=/root/disc-settings-test
R="$W/root"
mount --make-rprivate /
mount -t proc proc /proc
mount -t proc proc "$R/proc"
mount -t mqueue mqueue "$R/dev/mqueue"
printf 64 > /proc/sys/fs/mqueue/msg_max
printf 8192 > /proc/sys/fs/mqueue/msgsize_max
ulimit -n 8192
ulimit -c 0
guest(){ "$W/emu-confine" "$R" /emu/qemu-mipsel -cpu P5600 -E LD_LIBRARY_PATH=/usr/lib:/usr/lib/pulseaudio:/usr/data/lib:/usr/data/lib/ffmpeg "$@"; }
guest /usr/bin/mq_ui > "$W/ui.log" 2>&1 &
u=$!
sleep 3
guest /usr/bin/mq_player > "$W/player.log" 2>&1 &
p=$!
trap 'kill "$u" "$p" 2>/dev/null || true' EXIT INT TERM
sleep 1800 &
wait $!
