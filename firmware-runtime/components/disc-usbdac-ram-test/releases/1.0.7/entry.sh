#!/bin/sh
unset LD_PRELOAD
set -eu
test "$(cat /usr/data/disc-runtime/service/API)" = 6
echo 'DoP realtime 512-frame split experiment selected; RAM replay and audio capture remain idle until explicitly armed'
