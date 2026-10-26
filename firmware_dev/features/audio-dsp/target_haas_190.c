/* Loads only the DSP; replaces the stock main before any audio device opens. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include "../settings-debug/cache_sync.h"
#include "dsp_api.h"
#include "chain_format.h"
#define CHECK(x) do{if(!(x)){printf("FAIL %d %s\n",__LINE__,#x);return 2;}}while(0)
static int check(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;const char *path=getenv("DSP_CANDIDATE"),*old=getenv("DSP_BASELINE");
 void *lib=dlopen(path,2),*base=dlopen(old,2);CHECK(lib&&base&&module_cache_sync(path)&&module_cache_sync(old));
 const DiscDspApi *(*get)(void)=dlsym(lib,"disc_dsp_get_api"),*(*getold)(void)=dlsym(base,"disc_dsp_get_api");
 void *(*create)(unsigned,const DiscChain*)=dlsym(lib,"disc_dsp_create_chain");CHECK(get&&getold&&create);
 const DiscDspApi *api=get(),*baseline=getold();CHECK(api->abi==3&&baseline->abi==3);
 const unsigned rates[]={8000,44100,48000,96000,192000,384000};unsigned rng=190;int x[1024],y[1024];
 DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=13;c.nodes[0].enabled=1;
 for(unsigned r=0;r<6;r++){
  for(unsigned param=0;param<5;param++)for(unsigned end=0;end<2;end++){
   memcpy(c.nodes[0].params,disc_haas_default,sizeof(c.nodes[0].params));c.nodes[0].params[param]=disc_haas_range[param][end];void *d=create(rates[r],&c);CHECK(d);
   for(unsigned k=0;k<48;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=(int)rng;}CHECK(api->process(d,x,512));}
   unsigned limited,bad,available,latency,clipped;float peak,gain;api->diagnostics(d,&limited,&bad,&peak,&gain);api->stats(d,&available,&latency,&clipped);CHECK(!bad&&!clipped&&isfinite(peak)&&isfinite(gain));
   api->set_mask(d,0);memset(x,0,sizeof(x));for(unsigned k=0;k<80;k++)api->process(d,x,512);
   for(unsigned i=0;i<1024;i++)x[i]=y[i]=(i&1)?INT32_MIN+(int)i:INT32_MAX-(int)i;api->process(d,x,512);CHECK(!memcmp(x,y,sizeof(x)));
   api->set_mask(d,4096);memset(x,0,sizeof(x));for(unsigned k=0;k<48;k++){api->process(d,x,512);for(unsigned i=0;i<1024;i++)CHECK(!x[i]);}api->destroy(d);
  }
  printf("PASS Haas single effect rate=%u endpoints/finite/guard/OFF/reset\n",rates[r]);fflush(stdout);
 }
 for(unsigned r=1;r<6;r++)for(unsigned fx=0;fx<12;fx++){if(fx==1)continue;void *a=baseline->create(rates[r],1u<<fx,"."),*b=api->create(rates[r],1u<<fx,".");CHECK(a&&b);
  for(unsigned k=0;k<48;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=y[i]=(int)rng/4;}baseline->process(a,x,512);api->process(b,y,512);CHECK(!memcmp(x,y,sizeof(x)));}baseline->destroy(a);api->destroy(b);
 }
 puts("PASS 55 legacy single-effect contexts byte-identical to installed 1.8.0. No live audio or combined performance tests.");return 0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void(*init)(void),void(*fini)(void),void(*rtld)(void),void *stack){(void)fn;int(*next)(int(*)(int,char**,char**),int,char**,void(*)(void),void(*)(void),void(*)(void),void*)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(check,argc,argv,init,fini,rtld,stack):90;}
