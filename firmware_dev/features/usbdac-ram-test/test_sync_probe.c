#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sync_probe.h"
static unsigned calls,state=3;static int failure,advance;
static int mock(int fd,unsigned req,void *p){
 assert(fd==10&&req==0xc0844123u);struct SyncProbe *s=p;
 assert(s->flags==(calls?7u:6u));
 /* GET bits must both be set; do not write stale appl_ptr or avail_min. */
 assert((s->flags&6)==6);
 assert(s->control[0]==0&&s->control[1]==0);
 calls++;if((int)calls==failure)return -1;
 s->status[0]=state;s->status[2]=1000+(calls==2?128:0);
 s->control[0]=2000+(calls==2?advance:0);s->control[1]=14112;
 return 1; /* positive successful ioctl must not be treated as failure */
}
int main(void){
 struct SyncRecord r={0};assert(sync_read_pair(10,mock,&r)==0&&calls==2);
 assert(r.before_min==14112&&r.after_min==14112&&r.before_app==r.after_app&&r.after_hw>r.before_hw);
 calls=0;state=4;assert(sync_read_pair(10,mock,&r)==2&&calls==1);
 state=3;calls=0;failure=1;assert(sync_read_pair(10,mock,&r)==1&&calls==1);
 calls=0;failure=2;assert(sync_read_pair(10,mock,&r)==3&&calls==2);
 calls=0;failure=0;advance=1;assert(sync_read_pair(10,mock,&r)==5);
 puts("PASS: SYNC_PTR132-byte ABI, GET-only control flags, positive success, non-running refusal and ioctl failure");
}
