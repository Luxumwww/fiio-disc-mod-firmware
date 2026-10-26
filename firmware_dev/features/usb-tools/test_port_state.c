#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "thermal_port_state.h"
int main(void){
 assert(!thermal_port_state_ready("mode=both\nusb_active=0\nwireless_active=0\ntransition=debug\n"));
 assert(!thermal_port_state_ready("mode=wireless\nusb_active=0\nwireless_active=0\ntransition=\n"));
 assert(!thermal_port_state_ready("mode=wireless\nusb_active=0\nwireless_active=1\ntransition=debug\n"));
 assert(thermal_port_state_ready("mode=wireless\nusb_active=0\nwireless_active=1\ntransition=\n"));
 assert(thermal_port_state_ready("mode=off\nusb_active=0\nwireless_active=0\ntransition=\n"));
 puts("PASS incomplete USB teardown rejected; only completed off/wireless handoff accepted");
}
