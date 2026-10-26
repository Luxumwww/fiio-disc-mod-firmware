#!/bin/sh
set -eu
EMU_WORK=/root/disc-firmware-dev-v257/emulation
EMU_ROOT="$EMU_WORK/usbdac-root"
case "$EMU_ROOT" in /root/disc-firmware-dev-v257/emulation/usbdac-root) ;; *) exit 2;; esac
mount --make-rprivate /
mount -t proc proc /proc
mount -t proc proc "$EMU_ROOT/proc"
mount -t mqueue mqueue "$EMU_ROOT/dev/mqueue"
printf 64 > /proc/sys/fs/mqueue/msg_max
printf 8192 > /proc/sys/fs/mqueue/msgsize_max
ulimit -n 8192
ulimit -c 0
# No firmware init is executed; native helpers run only inside this root and
# cannot load modules, reboot, alter the clock/network, or perform raw I/O.
run_guest(){
 "$EMU_WORK/emu-confine" "$EMU_ROOT" /emu/qemu-mipsel -cpu P5600 "$@"
}
if [ -f "$EMU_ROOT/usr/data/fiio/disc-spectrum.activate" ]; then
 run_guest -E LD_PRELOAD=/usr/lib/libdisc-spectrum-ui.so /usr/bin/mq_ui > "$EMU_WORK/ui.log" 2>&1 &
else
 run_guest /usr/bin/mq_ui > "$EMU_WORK/ui.log" 2>&1 &
fi
emu_ui_pid=$!
sleep 3
if [ -f "$EMU_ROOT/usr/data/fiio/disc-spectrum.activate" ]; then
 run_guest -E LD_PRELOAD=/usr/lib/libdisc-pcm-tap.so /usr/bin/mq_player > "$EMU_WORK/player.log" 2>&1 &
else
 run_guest /usr/bin/mq_player > "$EMU_WORK/player.log" 2>&1 &
fi
emu_player_pid=$!
printf 'Guest UI/backend started in private PID/IPC/net/mount namespaces.\n'
trap 'kill "$emu_ui_pid" "$emu_player_pid" 2>/dev/null || true' EXIT INT TERM
sleep "${DISC_EMU_TTL:-1800}" &
wait $!
