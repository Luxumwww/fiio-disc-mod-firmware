#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
extern int sched_setaffinity(int,unsigned,const void *);
#include "../dsp_api.h"
#include "../chain_format.h"
#include "../../settings-debug/cache_sync.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line%d %s\n",__LINE__,#x);return 2;}}while(0)
typedef void *(*ChainCreate)(unsigned,const DiscChain *);
static double cpu(void){struct timespec t;clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static void input(int *x,unsigned n,unsigned *rng,unsigned loud){for(unsigned i=0;i<n*2;i++){*rng=*rng*1664525u+1013904223u;x[i]=(int)*rng/(loud?1:32);}}
static void spec(DiscChain *c,unsigned type){memset(c,0,sizeof *c);c->count=1;c->nodes[0].id=1;c->nodes[0].type=type;c->nodes[0].enabled=1;memcpy(c->nodes[0].params,type==15?disc_stereofx_default:disc_ffhaas_default,sizeof c->nodes[0].params);}
static void saved_slots(DiscChain *c,unsigned count){DiscChainNode on=c->nodes[0];c->count=count;for(unsigned i=0;i<count;i++){memset(c->nodes+i,0,sizeof c->nodes[i]);c->nodes[i].id=i+1;c->nodes[i].type=5;memcpy(c->nodes[i].params,disc_param_default[5],sizeof c->nodes[i].params);}c->nodes[count-1]=on;c->nodes[count-1].id=count;}
static int compare(const DiscDspApi *a,ChainCreate ca,const DiscDspApi *b,ChainCreate cb){
 unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1130,max_delta=0,contexts=0;int x[1024],y[1024];
 for(unsigned r=0;r<6;r++)for(unsigned v=0;v<8;v++){
  DiscChain c;spec(&c,15);if(v>=1&&v<=6)c.nodes[0].params[(v-1)/2]=disc_stereofx_range[(v-1)/2][(v-1)&1];if(v==7)c.nodes[0].params[0]=c.nodes[0].params[2]=1,c.nodes[0].params[1]=.7f;
  void *old=ca(rates[r],&c),*opt=cb(rates[r],&c);CHECK(old&&opt);
  for(unsigned k=0;k<64;k++){
   input(x,512,&rng,k&1);memcpy(y,x,sizeof x);a->process(old,x,512);b->process(opt,y,512);
   for(unsigned i=0;i<1024;i++){long long d=(long long)x[i]-y[i];unsigned delta=(unsigned)(d<0?-d:d);if(delta>max_delta)max_delta=delta;if(delta>512){printf("STEREOFX DIFFER rate=%u variant=%u block=%u sample=%u old=%d new=%d delta=%u\n",rates[r],v,k,i,x[i],y[i],delta);return 5;}}
  }
  unsigned lim,bad,av,lat,clip;float peak,gain;b->diagnostics(opt,&lim,&bad,&peak,&gain);b->stats(opt,&av,&lat,&clip);CHECK(!bad&&!clip&&isfinite(peak)&&isfinite(gain));
  b->set_mask(opt,0);for(unsigned k=0;k<80;k++){memset(y,0,sizeof y);b->process(opt,y,512);}input(x,512,&rng,1);memcpy(y,x,sizeof x);b->process(opt,y,512);CHECK(!memcmp(x,y,sizeof x));
  a->destroy(old);b->destroy(opt);contexts++;
 }
 printf("PASS %u single-StereoFX contexts/6 rates: min/default/max/combined parameters, alternating loud/quiet, finite/guard/bit-exact OFF; baseline S32 max_delta=%u\n",contexts,max_delta);fflush(stdout);
 /* Other effects retain their original numerical path; one enabled at a time. */
 for(unsigned r=1;r<6;r++)for(unsigned type=0;type<12;type++)if(type!=1){
  void *old=a->create(rates[r],1u<<type,"."),*opt=b->create(rates[r],1u<<type,".");CHECK(old&&opt);
  for(unsigned k=0;k<32;k++){input(x,512,&rng,0);memcpy(y,x,sizeof x);a->process(old,x,512);b->process(opt,y,512);if(memcmp(x,y,sizeof x)){for(unsigned i=0;i<1024;i++)if(x[i]!=y[i]){printf("LEGACY DIFFER rate=%u type=%u block=%u sample=%u old=%d new=%d\n",rates[r],type,k,i,x[i],y[i]);break;}return 3;}}a->destroy(old);b->destroy(opt);
 }
 for(unsigned r=0;r<6;r++)for(unsigned v=0;v<25;v++){
  DiscChain c;spec(&c,14);if(v)c.nodes[0].params[(v-1)/2]=disc_ffhaas_range[(v-1)/2][(v-1)&1];void *old=ca(rates[r],&c),*opt=cb(rates[r],&c);CHECK(old&&opt);
  for(unsigned k=0;k<16;k++){input(x,512,&rng,0);memcpy(y,x,sizeof x);a->process(old,x,512);b->process(opt,y,512);if(memcmp(x,y,sizeof x)){for(unsigned i=0;i<1024;i++)if(x[i]!=y[i]){printf("HAAS DIFFER rate=%u variant=%u block=%u sample=%u old=%d new=%d\n",rates[r],v,k,i,x[i],y[i]);break;}return 4;}}a->destroy(old);b->destroy(opt);
 }
 puts("PASS 55 legacy plus150 Haas single-effect contexts bit-identical to1.12.0");
 unsigned types[]={5,6,7,14,15};
 for(unsigned r=0;r<6;r++)for(unsigned t=0;t<5;t++)for(unsigned slots=9;slots<=16;slots+=7){
  DiscChain c;spec(&c,types[t]);if(types[t]<12)memcpy(c.nodes[0].params,disc_param_default[types[t]],sizeof c.nodes[0].params);saved_slots(&c,slots);
  void *old=ca(rates[r],&c),*opt=cb(rates[r],&c);CHECK(old&&opt);
  for(unsigned k=0;k<48;k++){input(x,512,&rng,0);memcpy(y,x,sizeof x);a->process(old,x,512);b->process(opt,y,512);CHECK(!memcmp(x,y,sizeof x));}
  a->set_mask(old,0);b->set_mask(opt,0);for(unsigned k=0;k<80;k++){input(x,512,&rng,0);memcpy(y,x,sizeof x);a->process(old,x,512);b->process(opt,y,512);CHECK(!memcmp(x,y,sizeof x));}a->destroy(old);b->destroy(opt);
 }
 puts("PASS60 9/16-saved single-enabled chains and OFF fades byte-identical; no multi-effect performance test");return 0;
}
static int bench(const DiscDspApi *a,ChainCreate ca,const DiscDspApi *b,ChainCreate cb){
 unsigned rates[]={48000,96000,192000,384000},rng=1130;int *x=malloc(7680*8);CHECK(x);
 for(unsigned r=0;r<4;r++)for(unsigned loud=0;loud<2;loud++){
  DiscChain c;spec(&c,15);const char *slots=getenv("DISC_SAVED_COUNT");unsigned saved=slots?(unsigned)atoi(slots):1;if(saved!=1){CHECK(saved==9||saved==16);saved_slots(&c,saved);}double cost[2]={0,0};
  unsigned context_seed=rng;for(unsigned j=0;j<2;j++){
   rng=context_seed;
   const DiscDspApi *api=j?b:a;void *d=(j?cb:ca)(rates[r],&c);CHECK(d);
   for(unsigned k=0;k<8;k++){input(x,7680,&rng,loud);api->process(d,x,7680);}
   for(unsigned k=0;k<3;k++){input(x,7680,&rng,loud);double start=cpu();api->process(d,x,7680);cost[j]+=cpu()-start;}
   api->destroy(d);
  }
  double period=3.*7680/rates[r];printf("BENCH single_StereoFX saved=%u rate=%u loud=%u old_cpu_ms=%.3f new_cpu_ms=%.3f old_realtime=%.4f new_realtime=%.4f new_over_old=%.4f\n",saved,rates[r],loud,cost[0]*1000,cost[1]*1000,cost[0]/period,cost[1]/period,cost[1]/cost[0]);fflush(stdout);
  CHECK(cost[1]<cost[0]*.9);CHECK(cost[1]<period*.6);
 }free(x);return 0;
}
static int run(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;setpriority(PRIO_PROCESS,0,19);
 if(getenv("DISC_PIN_TEST_CPU")){unsigned affinity=1;CHECK(!sched_setaffinity(0,sizeof affinity,&affinity));}
 const char *old=getenv("DSP_BASELINE"),*opt=getenv("DSP_CANDIDATE");void *la=dlopen(old,2),*lb=dlopen(opt,2);CHECK(la&&lb&&module_cache_sync(old)&&module_cache_sync(opt));
 const DiscDspApi *(*ga)(void)=dlsym(la,"disc_dsp_get_api"),*(*gb)(void)=dlsym(lb,"disc_dsp_get_api");ChainCreate ca=dlsym(la,"disc_dsp_create_chain"),cb=dlsym(lb,"disc_dsp_create_chain");CHECK(ga&&gb&&ca&&cb);
 if(getenv("DISC_BENCH_ONLY"))CHECK(!bench(ga(),ca,gb(),cb));else CHECK(!compare(ga(),ca,gb(),cb));
 puts("PASS isolated single-effect only; no PCM/real profile/playback/volume changes");return 0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld)(void),void *stack){(void)fn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");return next?next(run,argc,argv,init,fini,rtld,stack):90;}
