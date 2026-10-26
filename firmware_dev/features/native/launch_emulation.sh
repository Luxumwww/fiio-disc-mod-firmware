#!/bin/sh
set -eu
EMU_WORK=/root/disc-firmware-dev-v257/emulation
unshare -m -p -f -i -n -u --kill-child -- sh "$EMU_WORK/run_emulation.sh" &
disc_emu_ns=$!
printf '%s\n' "$disc_emu_ns" > "$EMU_WORK/namespace.pid"
wait "$disc_emu_ns"
