#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#define DISC_DSD_ENGINE_TEST 1
static union {unsigned align;unsigned char b[0x900000];} arena;
#define PC_WORD(a) (*(unsigned *)(void *)(arena.b+(unsigned)(a)))
#define PC_BYTE(a) arena.b[(unsigned)(a)]
static void *native(unsigned);
#define PC_FN(a,t) ((t)native(a))
static unsigned pcm[52],kernel_state=3,available=0,prefill;
#define DSDE_PCM_WORD(p,o) (((unsigned *)(p))[(o)/4])
#define DSDE_PCM_STATE(p) ((void)(p),kernel_state)
static int mode=4,route=6,clock_sec=1,opened=1;
static unsigned writes,originals,mutes,unmutes,bus_calls,prepares,stops,sw_calls,sleeps,emitted_size,output_phase=5;
static int fail_sw,fail_prepare,fail_bus,fail_unmute,fail_write,hidden_xrun,engine_disabled,old_enabled;
static unsigned char emitted[4000000],source[0x12c000];
static int pc_workmode(void){return mode;}
static unsigned pc_route(void){return route;}
static int pc_pcm_open(unsigned r){return r==6&&opened;}
static unsigned pc_seconds(void){return clock_sec;}
static int sleep_mock(unsigned n){assert(n>=1000);sleeps++;return 0;}
static int (*pc_real_usleep)(unsigned)=sleep_mock;
static int mutex_mock(void *p){assert(p);return 0;}
#define pthread_mutex_lock mutex_mock
#define pthread_mutex_unlock mutex_mock
static int fake_ioctl(int fd,unsigned req,void *arg){
 if(req==0xc0684113){assert(fd==27);unsigned *s=arg;sw_calls++;assert(s[4]==pcm[40]/2&&s[7]==0&&s[9]==pcm[5]);return fail_sw?-1:0;}
 assert(fd==8||fd==9);if(req==0x20004d2d){mutes++;return 1;}if(req==0x20004d2e){unmutes++;return fail_unmute?-1:1;}return 1;
}
static int (*dop_real_ioctl)(int,unsigned,void *)=fake_ioctl;
static int dop_bus_check(unsigned r,int change){assert((r==176400||r==352800)&&(change==0||change==1));bus_calls++;return !fail_bus;}
static const unsigned *fake_config(void *p){assert(p==pcm);return pcm+38;}
static int fake_stop(void *p){assert(p==pcm);stops++;kernel_state=1;return 0;}
static int fake_prepare(void *p){assert(p==pcm);prepares++;kernel_state=2;prefill=0;output_phase=5;available=pcm[4];return fail_prepare?-1:0;}
static int fake_stamp(void *p,unsigned *a,long *t){assert(p==pcm);*a=available;t[0]=1;t[1]=0;return kernel_state==3?0:-1;}
static int fake_write(void *p,const void *data,unsigned n){
 assert(p==pcm&&n&&n<=65536&&!(n&7));writes++;if(fail_write)return -1;
 const unsigned char *b=data;
 for(unsigned i=0;i<n;i+=8){assert(!b[i]&&!b[i+4]&&b[i+3]==output_phase&&b[i+7]==output_phase);output_phase^=255;}
 assert(emitted_size+n<=sizeof emitted);memcpy(emitted+emitted_size,b,n);emitted_size+=n;
 if(kernel_state==2){prefill+=n/8;available=pcm[4]-prefill;if(prefill==pcm[4])kernel_state=3;}
 else if(hidden_xrun){hidden_xrun=0;pcm[3]++;kernel_state=2;prefill=0;available=pcm[4];}
 return n/8; /* Positive success is valid, as are native first-write0 returns. */
}
static void dac(unsigned r,unsigned bits,unsigned dsd){assert(bits==32&&(r==176400||r==352800));PC_BYTE(0x899060)=dsd;PC_WORD(0x899084)=r;}
static int original_cfg(unsigned r,unsigned b,unsigned d){(void)r;(void)b;PC_BYTE(0x899060)=d;return 0;}
static int original_output(void *p,int n,int q){(void)p;(void)n;(void)q;originals++;return 0;}
static void *native(unsigned a){switch(a){case 0x4e8858:return dac;case 0x4e9534:return original_cfg;case 0x465718:return original_output;default:abort();}}
static const char *reading;
static int fake_open(const char *p,int f,...){(void)f;reading=p;return strstr(p,"FEATURES")?30:strstr(p,"/proc/version")?31:-1;}
static int fake_read(int f,void *b,unsigned n){const char *s=f==30?"usbdac-dsd-engine-abi1\n":"Linux version 4.4.94 #236 SMP PREEMPT";unsigned len=strlen(s);assert(len<=n);memcpy(b,s,len);return len;}
static int fake_access(const char *p,int n){(void)n;if(strstr(p,"SAFE_MODE"))return -1;if(strstr(p,"dsd-engine/disabled"))return engine_disabled?0:-1;return old_enabled?-1:0;}
static int fake_close(int f){(void)f;return 0;}
static int fake_write_file(int f,const void *p,unsigned n){(void)f;(void)p;return n;}
static void *fake_dlsym(void *h,const char *name){(void)h;if(!strcmp(name,"ioctl"))return fake_ioctl;if(!strcmp(name,"pcm_write"))return fake_write;if(!strcmp(name,"pcm_get_config"))return fake_config;if(!strcmp(name,"pcm_get_htimestamp"))return fake_stamp;if(!strcmp(name,"pcm_stop"))return fake_stop;if(!strcmp(name,"pcm_prepare"))return fake_prepare;return 0;}
#define open fake_open
#define read fake_read
#define access fake_access
#define close fake_close
#define write fake_write_file
#define dlsym fake_dlsym
#define getpid() 42
#define PLAYER_IOCTL_GOT 0x83820c
#include "binding.h"
static void init(unsigned rate){
 memset(&arena,0,sizeof arena);memset(pcm,0,sizeof pcm);mode=4;route=6;opened=1;
 unsigned p=0x810000,sink=0x820000;PC_WORD(0x83dfc4)=p;PC_WORD(0x83dfe4)=sink;PC_BYTE(0x83a75a)=6;PC_WORD(0x839a74)=1;
 PC_WORD(p+0xc)=PC_WORD(p+0x1c)=rate;PC_WORD(p+0x10)=PC_WORD(p+0x20)=2;PC_WORD(p+0x14)=PC_WORD(p+0x24)=32;PC_WORD(p+0x2c)=2;PC_WORD(p+0x58)=1;PC_WORD(p+0x98)=2;
 PC_WORD(sink+6*0x54+0x74)=(unsigned)(uintptr_t)pcm;PC_WORD(0x83a7d0)=8;PC_WORD(0x83a7cc)=9;
 pcm[0]=27;pcm[4]=rate/25*2;pcm[5]=1849688064;pcm[38]=2;pcm[39]=rate;pcm[40]=rate/25;pcm[41]=2;pcm[42]=7;pcm[43]=rate/25;pcm[44]=2147483647;pcm[46]=1;
 dsde_enabled=1;dsde_active=dsde_ready=dsde_primed=dsde_failed=0;dsde_pcm=0;
 dsde_real_write=fake_write;dsde_config=fake_config;dsde_stamp=fake_stamp;dsde_stop=fake_stop;dsde_prepare=fake_prepare;dsde_original_configure=original_cfg;
 dsde_errors=dsde_recoveries=dsde_payload_frames=dsde_silent_frames=dsde_invalid=0;
 writes=originals=mutes=unmutes=bus_calls=prepares=stops=sw_calls=sleeps=emitted_size=0;output_phase=5;available=0;
 fail_sw=fail_prepare=fail_bus=fail_unmute=fail_write=hidden_xrun=engine_disabled=old_enabled=0;
 for(unsigned i=0;i<sizeof source;i+=8){source[i]=source[i+4]=0;source[i+1]=(i>>4)|1;source[i+2]=(i>>7)|3;source[i+5]=(i>>6)|7;source[i+6]=(i>>8)|15;source[i+3]=source[i+7]=(i&8)?250:5;}
}
int main(void){
 for(unsigned rate=176400;rate<=352800;rate*=2){
  init(rate);assert(!dsde_configure(rate,32,1)&&dsde_ready&&!dsde_primed);
  assert(!dsde_output(source,32768,1)&&!writes);assert(!dsde_output(source,8,0)&&!writes);
  assert(!dsde_output(source,sizeof source,0)&&dsde_primed&&!dsde_failed);
  assert(dsde_payload_frames==sizeof(source)/8&&pcm[43]==pcm[4]&&pcm[44]==pcm[4]&&unmutes==1);
  unsigned offset=pcm[4]*8;assert(emitted_size==offset+sizeof source);
  for(unsigned i=0;i<sizeof source;i+=8)for(unsigned j=1;j<=6;j++)if(j!=3&&j!=4)assert(emitted[offset+i+j]==source[i+j]);
  unsigned before=writes;assert(!dsde_output(source,32768,1)&&writes==before);available=pcm[40];
  assert(!dsde_output(source,32768,1)&&writes==before+1);available=0;
  source[7]=7;assert(!dsde_output(source,1024,0)&&dsde_invalid==1);source[7]=5;
  for(unsigned i=emitted_size-1024;i<emitted_size;i+=8)assert(emitted[i+1]==0x69&&emitted[i+2]==0x69&&emitted[i+5]==0x69&&emitted[i+6]==0x69);
  dsde_enabled=0;assert(!dsde_output(source,1024,0)&&!originals);dsde_enabled=1;
  hidden_xrun=1;assert(!dsde_output(source,1024,0)&&dsde_recoveries==1&&dsde_primed&&!dsde_failed);
  assert(!dsde_configure(rate,32,1)&&!dsde_primed);assert(!dsde_output(source,1024,0)&&dsde_primed);
  assert(!dsde_configure(rate,32,0)&&!dsde_active&&pcm[43]==rate/25&&pcm[44]==2147483647u);
  PC_WORD(0x810000+0x2c)=1;PC_WORD(0x810000+0x58)=0;
  assert(!dsde_output(source,1024,0)&&originals==1);
 }
 init(352800);assert(!dsde_configure(352800,32,1));fail_prepare=1;assert(!dsde_output(source,1024,0)&&dsde_failed&&!writes&&!unmutes);
 init(352800);assert(!dsde_configure(352800,32,1));fail_unmute=1;assert(!dsde_output(source,1024,0)&&dsde_failed&&!dsde_primed);unsigned before=writes;assert(!dsde_output(source,1024,0)&&writes==before);
 init(352800);assert(!dsde_configure(352800,32,1));fail_bus=1;assert(!dsde_output(source,1024,0)&&dsde_failed&&!writes);
 init(352800);assert(!dsde_configure(352800,32,1));pcm[42]=6;assert(!dsde_output(source,1024,0)&&!writes);pcm[42]=7;
 mode=1;assert(!dsde_output(source,1024,0)&&originals==1);mode=2;assert(!dsde_output(source,1024,0)&&originals==2);mode=4;route=3;assert(!dsde_output(source,1024,0)&&originals==3);
 clock_sec++;dsde_gate();assert(dsde_enabled);old_enabled=1;clock_sec++;dsde_gate();assert(!dsde_enabled);old_enabled=0;engine_disabled=1;clock_sec++;dsde_gate();assert(!dsde_enabled);
 puts("PASS: DSD64/128 full bursts, no truncation/DSP, valid muted full-buffer prefill, native positive control/write success, quiet competition paced, malformed-to-DSD-silence, hidden XRUN bounded retry, independent disable transition, PCM thresholds restored, unrelated routes/unknown layout untouched, profile exclusion/kernel gate");
}
