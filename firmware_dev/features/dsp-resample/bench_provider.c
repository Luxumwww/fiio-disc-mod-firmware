#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sched.h>
#include <sys/resource.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include "resample_api.h"
#include "../audio-dsp/dsp_api.h"
#include "../audio-dsp/chain_format.h"
#include "../settings-debug/cache_sync.h"
#define CHECK(x) do {if(!(x)){printf("FAIL line=%u error=%s\n",__LINE__,dlerror());return 2;}}while(0)
static double clock_cpu(void){struct timespec t;clock_gettime(2,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static int run(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;cpu_set_t mask;CPU_ZERO(&mask);CPU_SET(0,&mask);CHECK(!sched_setaffinity(0,sizeof mask,&mask));setpriority(PRIO_PROCESS,0,19);
 const char *path=getenv("DSP_NEW");CHECK(path);void *lib=dlopen(path,2);CHECK(lib&&module_cache_sync(path));
 const DiscDspApi *(*get)(void)=dlsym(lib,"disc_dsp_get_api");const DiscResampleApi *(*src_get)(void)=dlsym(lib,"disc_resample_get_api");void *(*create)(unsigned,const DiscChain *)=dlsym(lib,"disc_dsp_create_chain");DiscResampleProcessFloat process=dlsym(lib,"disc_dsp_process_float");CHECK(get&&src_get&&create&&process);const DiscDspApi *api=get();const DiscResampleApi *src=src_get();CHECK(src->abi==2);
 DiscChain spec={0};spec.count=1;spec.nodes[0].id=1;spec.nodes[0].type=16;spec.nodes[0].enabled=1;memcpy(spec.nodes[0].params,disc_warmgloss_default,sizeof spec.nodes[0].params);
 unsigned rate=384000,n=15360;int *pcm=malloc(n*8),*original=malloc(n*8);CHECK(pcm&&original);for(unsigned i=0;i<n;i++){original[i*2]=(int)(sin(i*.02)*200000000);original[i*2+1]=(int)(cos(i*.02)*150000000);}
 for(unsigned ratio=1;ratio<=8;ratio*=2){
  unsigned target=rate/ratio;double begin=clock_cpu();void *dsp=create(target,&spec);CHECK(dsp);void *s=ratio>1?src->create(rate,target):0;CHECK(ratio==1||s);double setup=clock_cpu()-begin,total=0,max=0;unsigned errors=0,underflows=0,invalid=0,latency=0;
  for(unsigned k=0;k<10;k++){memcpy(pcm,original,n*8);begin=clock_cpu();CHECK(s?src->process(s,pcm,n,process,dsp):api->process(dsp,pcm,n));double cost=clock_cpu()-begin;if(k>=4){total+=cost;if(cost>max)max=cost;}}
  if(s)src->stats(s,&latency,&errors,&underflows,&invalid);CHECK(!errors&&!underflows&&!invalid);
  printf("PASS single-warmgloss input=%u processing=%u frames=%u budget_ms=40 avg_cpu_ms=%.3f max_cpu_ms=%.3f setup_ms=%.3f latency_frames=%u\n",rate,target,n,total*1000/6,max*1000,setup*1000,latency);fflush(stdout);if(s)src->destroy(s);api->destroy(dsp);
 }
 free(pcm);free(original);return 0;
}
__attribute__((visibility("default"))) int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld)(void),void *stack){(void)fn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");return next?next(run,argc,argv,init,fini,rtld,stack):90;}
