#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#define DISC_RAM_TEST
#include "replay.c"
int cacheflush(void *p,size_t n,int f){(void)p;(void)n;(void)f;return 0;}
static unsigned regs[]={0x10000,0x10001,0x10002,0x1000c,0x5000a,0x5000b,0x40014,0x40016,0x20000,0x1000b,0x70004,0x40015,0x40017,0x40018,0x40019};
static unsigned char values[2][15],initial[2][15];
static int fail_once,mute_fail,mutes,unmutes,buswrites;
static int forward(int fd,unsigned req,void *arg){
 if(fd==10){assert(!arg);if(req==0x20004d2d){mutes++;return mute_fail?-1:1;}assert(req==0x20004d2e);unmutes++;return 1;}
 assert(fd==40&&req==0x707);struct FormatTransfer *t=arg;unsigned c=(t->messages[0].address-0x30)/2;assert(c<2);
 unsigned char *b=t->messages[0].data;unsigned r=b[0]*65536u+b[1]*256u+b[2];assert(!b[3]);unsigned i;
 for(i=0;i<15;i++)if(regs[i]==r)break;assert(i<15);
 if(t->count==2){assert(t->messages[0].length==4&&t->messages[1].flags==1);*t->messages[1].data=values[c][i];return 2;}
 assert(t->count==1&&t->messages[0].length==5);buswrites++;
 if(i==8)assert((b[4]&~0x40)==0xa2);else{assert(i>=3&&i<=7);assert(values[c][8]&0x40);}
 values[c][i]=b[4];
 if(fail_once&&r==0x5000b&&c==1){fail_once=0;return -1;} /* Latch before reported bus failure. */
 return 1;
}
static void reset(unsigned hz){
 unsigned char v[]={0x43,0x13,0x10,4,7,15,23,47,0xa2,7,0x54,0,0,12,10};
 memcpy(values[0],v,15);memcpy(values[1],v,15);values[1][4]=15;
 if(hz==176400)for(unsigned c=0;c<2;c++){values[c][9]=5;values[c][10]=0x50;}
 memcpy(initial,values,sizeof values);native_ioctl=forward;fail_once=mute_fail=mutes=unmutes=buswrites=0;
}
int main(void){
 for(unsigned j=0;j<2;j++){
  unsigned hz=j?176400:352800;reset(hz);assert(!format_transaction(40,10,hz));
  assert(mutes==1&&unmutes==1&&format_mute_result==1&&format_unmute_result==1);
  for(unsigned c=0;c<2;c++){
   assert(values[c][3]==5&&values[c][4]==(c?14:6)&&values[c][5]==14&&values[c][6]==31&&values[c][7]==63&&values[c][8]==0xa2);
   for(unsigned i=0;i<15;i++)if(i<3||i>=8)assert(values[c][i]==initial[c][i]);
  }
 }
 reset(352800);fail_once=1;assert(format_transaction(40,10,352800)==6);
 assert(!memcmp(initial,values,sizeof values)&&mutes==1&&unmutes==1&&format_compensated>0);
 reset(352800);mute_fail=1;assert(format_transaction(40,10,352800)==4&&!buswrites&&!unmutes&&!memcmp(initial,values,sizeof values));
 reset(352800);values[1][4]=7;assert(format_transaction(40,10,352800)==3&&!mutes&&!buswrites);
 reset(352800);values[0][8]|=0x10;assert(format_transaction(40,10,352800)==2&&!mutes&&!buswrites);
 reset(352800);values[0][9]=5;assert(format_transaction(40,10,352800)==3&&!mutes&&!buswrites);
 puts("PASS: DoP64/128 profile24 valid bits in32-bit slots, preserves phase/clock/HP/PLL, exact readback, positive mute success, compensates latched bus failure, rejects unexpected mapping/rate/power before writes");
}
