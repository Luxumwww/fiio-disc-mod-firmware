#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned memory[0x100000/4],pcm[52],state=3,available=0,clock_valid=1;
#define PC_WORD(a) memory[((a)-0x800000)/4]
#define DOP_PCM_WORD(p,o) (((unsigned *)(p))[(o)/4])
#define DOP_PCM_STATE(p) ((void)(p),state)
static unsigned sw_calls,mutes,unmutes,stops,prepares,writes,bus_checks;
static int fail_sw,fail_prepare,fail_bus,fail_unmute,race;
static unsigned char packet[112896],copy[112896];
static int fake_ioctl(int fd,unsigned req,void *arg){
 if(req==0xc0684113){assert(fd==27);unsigned *s=arg;sw_calls++;
  assert(s[0]==1&&s[1]==1&&s[3]==1&&s[4]==7056&&s[7]==0&&s[9]==pcm[5]);
  if(fail_sw)return -1;assert(s[5]==28224||s[5]==14112);assert(s[6]==28224||s[6]==2147483647u);return 0;
 }
 assert(fd==4&&!arg);if(req==0x20004d2d){mutes++;return 0;}
 assert(req==0x20004d2e);unmutes++;return fail_unmute?-1:0;
}
static int fake_stop(void *p){assert(p==pcm);stops++;state=1;pcm[2]=0;return 0;}
static int fake_prepare(void *p){assert(p==pcm);prepares++;if(fail_prepare)return -1;state=2;pcm[2]=2;available=28224;return 0;}
static int fake_timestamp(void *p,unsigned *a,long *ts){assert(p==pcm);*a=available;ts[0]=1;ts[1]=0;return clock_valid&&(state==3||state==5)?0:-1;}
static const unsigned *fake_config(void *p){return (unsigned *)p+38;}
static int fake_pcm_write(void *p,const void *data,unsigned n){
 assert(p==pcm&&n==sizeof packet&&!memcmp(data,copy,n));writes++;
 if(race){race=0;pcm[3]++;state=2;available=28224;}
 if(state==2){assert(available>=14112);available-=14112;if(!available)state=3;}
 return 0; /* Stock first successful write returns zero. */
}
static int (*dop_real_ioctl)(int,unsigned,void *)=fake_ioctl;
static int (*dop_real_pcm_write)(void *,const void *,unsigned)=fake_pcm_write;
static int dop_bus_check(unsigned rate,int repair){assert(rate==352800&&repair==1);bus_checks++;return !fail_bus;}
static int fake_open(const char *p,int flags,...){(void)p;(void)flags;return -1;}
static int fake_write(int f,const void *p,unsigned n){(void)f;(void)p;return n;}
static int fake_close(int f){(void)f;return 0;}
#define open fake_open
#define write fake_write
#define close fake_close
#include "dop_recover.h"
static void init(void){
 memset(memory,0,sizeof memory);memset(pcm,0,sizeof pcm);
 PC_WORD(0x83dfc4)=0x810000;PC_WORD(0x810000+0xc)=352800;
 PC_WORD(0x83dfe4)=0x820000;PC_WORD(0x820000+6*0x54+0x74)=(unsigned)(unsigned long)pcm;PC_WORD(0x83a7d0)=4;
 pcm[0]=27;pcm[2]=3;pcm[4]=28224;pcm[5]=1849688064;pcm[38]=2;pcm[39]=352800;
 pcm[40]=14112;pcm[41]=2;pcm[42]=7;pcm[43]=14112;pcm[44]=2147483647u;pcm[46]=1;
 dop_pcm_config=fake_config;dop_pcm_stop=fake_stop;dop_pcm_prepare=fake_prepare;dop_pcm_htimestamp=fake_timestamp;
 dop_recovery_forget();fail_sw=fail_prepare=fail_bus=fail_unmute=race=0;state=3;available=0;clock_valid=1;
 sw_calls=mutes=unmutes=stops=prepares=writes=bus_checks=0;
 dop_recovery_thresholds=dop_recovery_xruns=dop_recovery_restarts=dop_recovery_failures=0;
 memcpy(copy,packet,sizeof packet);
}
static int run(int guard){return dop_recovery_write(pcm,packet,sizeof packet,guard);}
int main(void){
 init();assert(!run(1));assert(sw_calls==1&&pcm[43]==28224&&pcm[44]==28224&&!mutes&&writes==1);
 for(unsigned i=0;i<20;i++)assert(!run(1));assert(sw_calls==1&&!stops&&!bus_checks);
 /* Same-rate native reopen reuses its heap address AND fd. */
 pcm[43]=14112;pcm[44]=2147483647u;assert(!run(1)&&sw_calls==2&&pcm[44]==28224&&!mutes);
 /* Measured lost data with RUNNING and no native underrun counter. */
 available=28225;assert(!run(1));assert(mutes==1&&stops==1&&prepares==1&&bus_checks==1&&!unmutes&&state==2&&dop_recovery_muted);
 assert(!run(1));assert(unmutes==1&&state==3&&!dop_recovery_muted&&dop_recovery_restarts==1);
 assert(!run(1));assert(stops==1); /* No reset of healthy playback. */
 init();state=4;assert(!run(1));assert(stops==1);assert(!run(1)&&unmutes==1);
 init();clock_valid=0;available=99999;assert(!run(1));assert(!mutes); /* A failed timestamp is not an XRUN. */
 init();assert(!run(1));race=1;unsigned before=writes;assert(!run(1));assert(writes==before+2&&stops==1&&!unmutes);assert(!run(1)&&unmutes==1);
 /* Disabled and PCM restore only this still-identical managed handle. */
 assert(!run(0)&&!dop_recovery_pcm&&pcm[43]==14112&&pcm[44]==2147483647u);
 init();assert(!run(0));assert(!sw_calls&&!mutes);
 init();pcm[42]=6;assert(!run(1)&&!sw_calls); /* Foreign format ABI ignored. */
 init();PC_WORD(0x820000+6*0x54+0x74)=0;assert(!run(1)&&!sw_calls);
 init();fail_sw=1;assert(run(1)<0&&!writes&&dop_recovery_failures==1);assert(run(1)<0&&sw_calls==1);
 init();state=4;fail_prepare=1;assert(run(1)<0&&!writes&&!unmutes&&dop_recovery_muted);assert(run(1)<0&&stops==1);
 init();state=4;fail_bus=1;assert(run(1)<0&&!writes&&!unmutes);
 init();state=4;assert(!run(1));fail_unmute=1;assert(run(1)<0&&dop_recovery_failed&&dop_recovery_muted);assert(run(1)<0&&unmutes==1);
 puts("PASS: DoP-only finite stop/full prefill, RUNNING starvation and XRUN recovery, hidden EPIPE race, unchanged packets, healthy zero resets, disabled/PCM restore, foreign ABI and bounded failure paths");
}
