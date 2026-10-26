sleep 90
if [ "$(cat /tmp/disc-runtime/tcpip-test.pending 2>/dev/null)" = 31280812000000 ];then /usr/data/disc-runtime/service/debugctl.sh set 1 0;rm -f /tmp/disc-runtime/tcpip-test.pending;fi
