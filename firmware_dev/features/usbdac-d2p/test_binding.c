#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#define DISC_D2P_TEST 1
static union {unsigned align;unsigned char bytes[0x900000];} arena;
#define PC_BYTE(a) arena.bytes[(unsigned)(a)]
#define PC_WORD(a) (*(unsigned *)(void *)(arena.bytes+(unsigned)(a)))
static void *native(unsigned);
#define PC_FN(a,t) ((t)native(a))
static int mode=4,route=6,pcm_opened=1,clock_sec=1;
static unsigned calls,original_outputs,writes,write_bytes,eq_calls,rate_out,open_fail;
static int pc_workmode(void){return mode;}
static unsigned pc_route(void){return route;}
static int pc_pcm_open(unsigned r){return r==6&&pcm_opened;}
static unsigned pc_seconds(void){return clock_sec;}
static int sleep_mock(unsigned x){(void)x;return 0;}
static int (*pc_real_usleep)(unsigned)=sleep_mock;
/* Production uses a zero-initialized native mutex. Host test supplies the
 * same API over host mutex storage via no-op single-thread mocks. */
#define pthread_mutex_lock(p) ((void)(p),0)
#define pthread_mutex_unlock(p) ((void)(p),0)
static int config(unsigned r,unsigned b,unsigned d){assert(b==32);calls++;rate_out=r;PC_BYTE(0x899060)=d;return 0;}
static void mute(unsigned v){assert(v<=1);}
static void close_pcm(unsigned flags){assert(!flags);pcm_opened=0;}
static int open_pcm(unsigned r,unsigned b,unsigned c,unsigned f,unsigned d){assert(b==32&&c==2&&!f&&!d);rate_out=r;pcm_opened=!open_fail;return open_fail?-1:0;}
static int original(void *p,int n,int q){(void)p;(void)n;(void)q;original_outputs++;return 0;}
static int eq(void *p,int n,int r){(void)p;assert(n>=0&&r==88200);eq_calls++;return 0;}
static int output(void *p,unsigned n){assert(p&&n%8==0&&pcm_opened);writes++;write_bytes+=n;return 0;}
static void *native(unsigned a){switch(a){case 0x4e8b94:return mute;case 0x478354:return close_pcm;case 0x47859c:return open_pcm;case 0x465718:return original;case 0x45c648:return eq;case 0x47824c:return output;case 0x4e9534:return config;default:abort();}}
#include "binding.h"
int main(void){
 unsigned p=0x810000;PC_WORD(0x83dfc4)=p;PC_BYTE(0x83a75a)=6;PC_WORD(0x839a74)=1;
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=352800;PC_WORD(p+0x10)=PC_WORD(p+0x20)=2;
 PC_WORD(p+0x14)=PC_WORD(p+0x24)=32;PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;
 d2p_enabled=1;d2p_original_configure=config;
 assert(d2p_configure(352800,32,1)==0&&d2p_ready&&d2p_active&&rate_out==88200&&!PC_BYTE(0x899060));
 assert(PC_WORD(p+0x1c)==88200&&PC_WORD(p+0xc)==352800&&PC_WORD(p+0x58)==1);
 unsigned char packet[1024];for(unsigned i=0;i<sizeof(packet);i+=8){packet[i]=packet[i+4]=0;packet[i+1]=packet[i+2]=packet[i+5]=packet[i+6]=0x69;packet[i+3]=packet[i+7]=(i/8&1)?250:5;}
 assert(!d2p_output(packet,sizeof(packet),0)&&writes==1&&write_bytes==256&&eq_calls==1&&original_outputs==0);
 packet[7]=7;assert(!d2p_output(packet,sizeof(packet),0)&&d2p_bad==1&&writes==2&&write_bytes==512);
 assert(!d2p_output(packet,sizeof(packet),1)&&d2p_silence==1&&eq_calls==1);
 mode=1;assert(!d2p_output(packet,sizeof(packet),0)&&original_outputs==1);mode=2;assert(!d2p_output(packet,sizeof(packet),0)&&original_outputs==2);
 mode=4;route=3;assert(!d2p_output(packet,sizeof(packet),0)&&original_outputs==3);route=6;
 PC_WORD(p+0x2c)=1;PC_WORD(p+0x58)=0;assert(!d2p_configure(88200,32,0)&&!d2p_active&&rate_out==352800);
 assert(!d2p_output(packet,sizeof(packet),0)&&original_outputs==4);
 PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;open_fail=1;assert(d2p_configure(352800,32,1)==-1&&!d2p_ready&&d2p_failures==1);unsigned before=writes;assert(!d2p_output(packet,sizeof(packet),0)&&writes==before);
 d2p_enabled=0;assert(!d2p_output(packet,sizeof(packet),0)&&writes==before); /* no raw DoP into configured PCM */
 puts("PASS: only USB DoP converted, native PCM config/rate restored, no raw DoP on invalid packets/open failure/disable, local/BT/external route forwarded, original input descriptor preserved");
 return 0;
}
