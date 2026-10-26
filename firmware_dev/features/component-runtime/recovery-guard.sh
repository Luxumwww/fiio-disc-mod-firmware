#!/bin/sh
sleep "${1:-60}"
if [ -f /tmp/disc-runtime/recovery-pending ];then
 /usr/data/disc-runtime/service/debugctl.sh set 1 0
fi
