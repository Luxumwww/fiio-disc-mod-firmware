 #define _GNU_SOURCE
/* Finite isolated old/new DSP comparison; never opens device PCM or controls playback. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include "../audio-dsp/dsp_api.h"
#include "../audio-dsp/chain_format.h"
#include "../settings-debug/cache_sync.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line=%u\n",__LINE__);fflush(stdout);return 2;}}while(0)
typedef void *(*Create)(unsigned,const DiscChain *);
typedef struct {const DiscDspApi *api;Create create;} Library;
static Library load(const char *path){Library r={0};void *h=dlopen(path,2);if(!h||!module_cache_sync(path))return r;const DiscDspApi *(*get)(void)=dlsym(h,"disc_dsp_get_api");r.create=dlsym(h,"disc_dsp_create_chain");if(get)r.api=get();return r;}
static unsigned rng=116;
static void input(int *x,unsigned n,unsigned phase){for(unsigned i=0;i<n*2;i++){rng=rng*1664525u+1013904223u;int v=(int)rng;if(phase==0)v/=3;if(phase==1)v=0;if(phase==2)v/=100000;x[i]=v;}}
static double cpu(void){struct timespec t;clock_gettime(2,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static void spec(DiscChain *c){memset(c,0,sizeof *c);c->count=10;for(unsigned j=0;j<10;j++){c->nodes[j].id=j+1;c->nodes[j].type=16;memcpy(c->nodes[j].params,disc_warmgloss_default,sizeof c->nodes[j].params);}c->nodes[9].enabled=1;float live[12]={1,1,3.3f,-2.5f,160,-1,.58f,.5f,0,0,0,0};memcpy(c->nodes[9].params,live,sizeof live);}
static int run(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;setpriority(PRIO_PROCESS,0,19);if(getenv("DSP_CPU")){cpu_set_t mask;CPU_ZERO(&mask);CPU_SET(*getenv("DSP_CPU")-'0',&mask);CHECK(!sched_setaffinity(0,sizeof mask,&mask));}
 Library a=load(getenv("DSP_OLD")),b=load(getenv("DSP_NEW"));CHECK(a.api&&a.create&&b.api&&b.create);int x[15360],y[15360];unsigned contexts=0;
 if(!getenv("DSP_BENCH_ONLY"))for(unsigned ri=0;ri<6;ri++)for(unsigned v=0;v<19;v++){
  unsigned rate=(unsigned[]){8000,44100,48000,96000,192000,384000}[ri];DiscChain c;spec(&c);if(v<16)c.nodes[9].params[v/2]=disc_warmgloss_range[v/2][v&1];if(v==17)memcpy(c.nodes[9].params,disc_warmgloss_default,sizeof c.nodes[9].params);if(v==18)c.nodes[9].enabled=0;
  void *da=a.create(rate,&c),*db=b.create(rate,&c);CHECK(da&&db);
  for(unsigned k=0;k<24;k++){if(k==12){a.api->set_mask(da,0);b.api->set_mask(db,0);}if(k==20){a.api->set_mask(da,4096);b.api->set_mask(db,4096);}input(x,512,k%3);memcpy(y,x,512*8);CHECK(a.api->process(da,x,512)&&b.api->process(db,y,512));if(memcmp(x,y,512*8)){printf("DIFF rate=%u variant=%u block=%u\n",rate,v,k);CHECK(0);}}
  a.api->destroy(da);b.api->destroy(db);contexts++;
 }
 printf("PASS full DSP S32 exact contexts=%u; saved10 enabled<=1\n",contexts);
 if(getenv("DSP_OTHER_ONLY")){
  unsigned tested=0;
  for(unsigned type=0;type<16;type++)if(type!=1)for(unsigned ri=0;ri<2;ri++){
   DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=type;c.nodes[0].enabled=1;
   const float *p=type<12?disc_param_default[type]:type==12?disc_chain_eq_default:type==13?disc_haas_default:type==14?disc_ffhaas_default:disc_stereofx_default;
   memcpy(c.nodes[0].params,p,sizeof c.nodes[0].params);unsigned rate=ri?384000:48000;
   void *da=a.create(rate,&c),*db=b.create(rate,&c);CHECK(da&&db);
   for(unsigned k=0;k<8;k++){input(x,512,k%3);memcpy(y,x,512*8);CHECK(a.api->process(da,x,512)&&b.api->process(db,y,512));if(memcmp(x,y,512*8)){printf("OTHER_DIFF type=%u rate=%u block=%u\n",type,rate,k);CHECK(0);}}
   a.api->destroy(da);b.api->destroy(db);tested++;
  }
  printf("PASS other DSP exact individual contexts=%u active1 each; no performance comparison\n",tested);fflush(stdout);return 0;
 }
 DiscChain c;spec(&c);void *d[2]={a.create(384000,&c),b.create(384000,&c)};Library libs[2]={a,b};CHECK(d[0]&&d[1]);double total[2]={0},max[2]={0};
 for(unsigned k=0;k<8;k++){input(x,7680,k%3);memcpy(y,x,sizeof x);for(unsigned j=0;j<2;j++){double t=cpu();CHECK(libs[j].api->process(d[j],j?y:x,7680));double dt=cpu()-t;if(k>=4){total[j]+=dt;if(dt>max[j])max[j]=dt;}}CHECK(!memcmp(x,y,sizeof x));}
 printf("BENCH full chain rate=384000 frames=7680 blocks=4 old_total_ms=%.3f new_total_ms=%.3f old_max_ms=%.3f new_max_ms=%.3f ratio=%.4f\n",total[0]*1000,total[1]*1000,max[0]*1000,max[1]*1000,total[1]/total[0]);
 a.api->destroy(d[0]);b.api->destroy(d[1]);fflush(stdout);return 0;
}
__attribute__((visibility("default"))) int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld)(void),void *stack){(void)fn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");return next?next(run,argc,argv,init,fini,rtld,stack):90;}
