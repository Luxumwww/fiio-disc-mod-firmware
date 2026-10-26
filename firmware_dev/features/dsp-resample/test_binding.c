#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
static unsigned header[64],mode=4,route=6,time_ms=10000,word_stub;
static unsigned char byte_stub;
static char profile[8192],config[128];static const char *read_text;static int read_done,force_busy;
static unsigned *pc_word(unsigned a){if(a==0x83dfc4){static unsigned p=0x10000000;return &p;}if(a>=0x10000000&&a<0x10000100)return header+(a-0x10000000)/4;return &word_stub;}
#define PC_WORD(a) (*pc_word(a))
#define PC_BYTE(a) byte_stub
static int native_fallback(void *p,int bytes){(void)p;return bytes;}
#define PC_FN(a,t) ((t)native_fallback)
static int pc_workmode(void){return mode;}
static unsigned pc_route(void){return route;}
static unsigned pc_seconds(void){return time_ms/1000;}
static int fake_open(const char *path,int flags,...){(void)flags;read_done=0;if(strstr(path,"/FEATURES"))read_text="audio-dsp-abi3\n";else if(strstr(path,"shared-profile.conf"))read_text=profile;else if(strstr(path,"resample.conf")){if(!config[0])return -1;read_text=config;}else return -1;return 50;}
static int fake_read(int fd,void *b,size_t size){(void)fd;if(read_done)return 0;unsigned n=(unsigned)strlen(read_text);if(n>size)n=(unsigned)size;memcpy(b,read_text,n);read_done=1;return (int)n;}
static int fake_close(int fd){(void)fd;return 0;}
static int fake_access(const char *path,int flags){(void)flags;if(strstr(path,"resample.conf"))return config[0]?0:-1;if(strstr(path,"/enabled"))return 0;return -1;}
static int fake_trylock(void *m){(void)m;return force_busy;}
static int fake_unlock(void *m){(void)m;return 0;}
static int fake_clock(int id,void *p){(void)id;long *v=p;v[0]=time_ms/1000;v[1]=(long)(time_ms%1000)*1000000;return 0;}
static int module_cache_sync(const char *p){(void)p;return 1;}
static int dop_eq_ready=1;static int dop_eq_install(void){return 1;}
static int native_eq_install(void){return 1;}
#define DISC_NATIVE_EQ_TEST 1
#define open fake_open
#define read fake_read
#define close fake_close
#define access fake_access
#define pthread_mutex_trylock fake_trylock
#define pthread_mutex_unlock fake_unlock
#define clock_gettime fake_clock
#include "../audio-dsp/binding.h"
#undef open
#undef read
#undef close
#undef access
#undef clock_gettime
static int pcm[1024];
static void blocks(unsigned n){for(unsigned k=0;k<n;k++){for(unsigned i=0;i<512;i++){pcm[i*2]=(int)(sin((k*512.+i)*.007)*200000000.);pcm[i*2+1]=pcm[i*2];}dsp_process_buffer(pcm,sizeof(pcm),(int)header[7]);}}
static void *fail_create(unsigned a,unsigned b){(void)a;(void)b;return 0;}
int main(int argc,char **argv){
 assert(argc==2);void *lib=dlopen(argv[1],2);if(!lib){puts(dlerror());return 2;}
 const DiscDspApi *(*get)(void)=dlsym(lib,"disc_dsp_get_api");const DiscResampleApi *(*src)(void)=dlsym(lib,"disc_resample_get_api");assert(get&&src);dsp_api=get();dsp_src_api=src();dsp_chain_create=dlsym(lib,"disc_dsp_create_chain");dsp_chain_last_error=dlsym(lib,"disc_dsp_chain_error");dsp_float_process=(DiscResampleProcessFloat)dlsym(lib,"disc_dsp_process_float");assert(dsp_chain_create&&dsp_float_process);native_eq_ready=1;
 DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=16;c.nodes[0].enabled=1;memcpy(c.nodes[0].params,disc_warmgloss_default,sizeof(c.nodes[0].params));assert(disc_shared_format(&c,1,profile,sizeof(profile)));
 header[7]=192000;header[8]=2;header[9]=32;header[18]=1;dsp_tick();assert(dsp_context&&dsp_rate==192000&&dsp_processing_rate==192000&&!dsp_src_context);blocks(20);
 /* Cancelling before publication must restore the old context's gain. */
 strcpy(config,"1 1 1 1 1 1 1 1\n");time_ms=11000;dsp_tick();assert(dsp_src_fade_mode==1);blocks(20);assert(dsp_src_fade_mode==2);strcpy(config,"1 0 1 1 1 1 1 1\n");dsp_src_poll();dsp_tick();blocks(20);assert(!dsp_src_fade_mode&&!dsp_param_fading&&!dsp_src_context);
 strcpy(config,"1 1 1 1 1 1 1 1\n");dsp_src_poll();dsp_tick();blocks(20);assert(dsp_src_fade_mode==2);time_ms+=200;dsp_tick();assert(dsp_processing_rate==96000&&dsp_rate==192000&&dsp_src_context&&dsp_src_fade_mode==3);blocks(80);assert(!dsp_src_fade_mode);DiscSrcSnapshot state;assert(dsp_src_read_stats(&state)&&!state.src_errors&&!state.src_underflows&&!state.invalid);
 /* A stale audio block after publication stays muted and cannot consume the
  * new engine's wait period. This protects the context-swap boundary. */
 dsp_src_fade_gain=0;dsp_src_fade_up(dsp_generation,99);for(unsigned i=0;i<1024;i++)pcm[i]=12345678;dsp_src_fade_apply(pcm,sizeof(pcm),192000,dsp_generation-1);assert(dsp_src_wait==99);for(unsigned i=0;i<1024;i++)assert(!pcm[i]);blocks(20);assert(!dsp_src_fade_mode);
 strcpy(config,"1 0 1 1 1 1 1 1\n");time_ms=12000;dsp_tick();blocks(20);assert(dsp_src_fade_mode==2);time_ms+=200;dsp_tick();assert(!dsp_src_context&&dsp_processing_rate==192000);blocks(20);assert(!dsp_src_fade_mode);
 /* Allocation failure retains the old context, fades back up, and backs off. */
 const DiscResampleApi *real=dsp_src_api;DiscResampleApi fail=*real;fail.create=fail_create;dsp_src_api=&fail;strcpy(config,"1 1 1 1 1 1 1 1\n");time_ms=13000;dsp_tick();blocks(20);time_ms+=200;void *old=dsp_context;dsp_tick();assert(dsp_context==old&&!dsp_src_context&&dsp_chain_error);blocks(20);unsigned failures=dsp_failed;time_ms+=200;dsp_tick();assert(dsp_failed==failures&&!dsp_src_fade_mode);dsp_src_api=real;
 /* Packed/mono format and DoP never enter the PCM DSP/SRC path. */
 header[9]=16;dsp_process_buffer(pcm,sizeof(pcm),192000);assert(dsp_reset_needed);header[9]=32;dsp_reset_needed=0;byte_stub=1;int before[1024];memcpy(before,pcm,sizeof(pcm));dsp_process_buffer(pcm,sizeof(pcm),192000);assert(!memcmp(before,pcm,sizeof(pcm)));byte_stub=0;
 dsp_api->destroy(dsp_context);dsp_context=0;puts("PASS production binding: default native,192->96->192 buffer, fade epochs, no stale block, pause/rate states, allocation fallback/backoff, DoP/16bit bypass; only one warmgloss");return 0;
}
