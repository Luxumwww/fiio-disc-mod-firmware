#include <assert.h>
#include <stdio.h>
#include "bt_wifi_policy.h"
int main(void){
 DiscBtWifiPolicy p={0};
 assert(disc_bt_wifi_step(&p,0,1,0,1)==-1);
 assert(disc_bt_wifi_step(&p,3,1,1,2)==-1&&p.restore==1);
 assert(disc_bt_wifi_step(&p,3,1,0,3)==0);
 assert(disc_bt_wifi_step(&p,3,0,0,4)==-1&&p.pending==0);
 assert(disc_bt_wifi_step(&p,3,0,0,5)==-1&&p.restore==1);
 assert(disc_bt_wifi_step(&p,0,0,1,6)==-1&&p.pending==2);
 assert(disc_bt_wifi_step(&p,0,0,0,7)==1);
 assert(disc_bt_wifi_step(&p,0,1,0,8)==-1&&p.pending==0);
 p=(DiscBtWifiPolicy){0};
 assert(disc_bt_wifi_step(&p,3,0,0,1)==-1&&!p.restore);
 assert(disc_bt_wifi_step(&p,0,0,0,2)==-1);
 p=(DiscBtWifiPolicy){1,1,0,0,0,0}; /* UI reload / next boot retains restore intent. */
 assert(disc_bt_wifi_step(&p,3,0,0,10)==-1&&p.restore);
 assert(disc_bt_wifi_step(&p,3,1,0,11)==-1&&p.manual&&!p.restore);
 assert(disc_bt_wifi_step(&p,3,0,0,12)==-1);
 assert(disc_bt_wifi_step(&p,0,0,0,13)==-1);
 p=(DiscBtWifiPolicy){1,1,0,0,0,0};
 assert(disc_bt_wifi_step(&p,0,0,1,1)==-1&&p.pending==2);
 assert(disc_bt_wifi_step(&p,3,0,0,2)==-1&&p.restore&&!p.pending);
 assert(disc_bt_wifi_step(&p,0,0,0,3)==1);
 p=(DiscBtWifiPolicy){0};
 assert(disc_bt_wifi_step(&p,3,1,0,1)==0);
 assert(disc_bt_wifi_step(&p,3,1,0,2)==-1);
 assert(disc_bt_wifi_step(&p,3,1,0,2001)==0);
 assert(disc_bt_wifi_step(&p,3,1,0,4001)==0);
 assert(disc_bt_wifi_step(&p,3,1,0,9000)==-1&&p.attempts==3);
 disc_bt_wifi_manual(&p);
 assert(disc_bt_wifi_step(&p,3,1,0,9999)==-1&&!p.restore);
 DiscBtWifiPolicy old=p;
 assert(disc_bt_wifi_step(&p,99,1,0,10000)==-1&&p.active==old.active);
 assert(disc_bt_wifi_valid(&p));p.pending=2;assert(!disc_bt_wifi_valid(&p));
 puts("PASS entry/exit, initially off, busy defer, UI reload, manual override, rapid reentry, bounded retries, invalid mode/state");
}
