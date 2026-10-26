sleep 90
if [ -f /tmp/disc-runtime/tcpip-test.pending ];then /usr/data/disc-runtime/service/debugctl.sh set 1 0;rm -f /tmp/disc-runtime/tcpip-test.pending;fi
