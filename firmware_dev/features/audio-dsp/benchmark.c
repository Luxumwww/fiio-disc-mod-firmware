/* Bounded offline native-CPU measurement. Never opens PCM or DAC hardware. */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
#include <fcntl.h>
#include "../settings-debug/cache_sync.h"
#include "dsp_api.h"
static int benchmark(int argc,char **argv){
 if(argc!=3)return 2;void *lib=dlopen(argv[1],RTLD_NOW);if(!lib){puts(dlerror());return 3;}
 if(!module_cache_sync(argv[1]))return 9;const DiscDspApi *(*get)(void)=dlsym(lib,"disc_dsp_get_api");if(!get)return 4;const DiscDspApi *api=get();
 const unsigned rates[]={48000,96000,192000,384000,768000};const unsigned masks[]={1,2,4,5,7};for(unsigned i=0;i<5;i++)for(unsigned mi=0;mi<5;mi++){unsigned mask=masks[mi];
  unsigned rate=rates[i],frames=rate;char *only_rate=getenv("DSP_BENCH_RATE"),*only_mask=getenv("DSP_BENCH_MASK"),*realtime=getenv("DSP_BENCH_REALTIME");if(realtime)frames*=5;if((only_rate&&rate!=(unsigned)atoi(only_rate))||(only_mask&&mask!=(unsigned)atoi(only_mask)))continue;void *d=api->create(rate,mask,argv[2]);if(!d)return 5;
  unsigned available,latency,clipped;api->stats(d,&available,&latency,&clipped);if((available&mask)!=mask)return 6;
  int *pcm=calloc(frames*2,sizeof(int));if(!pcm)return 7;for(unsigned j=0;j<frames;j++)pcm[j*2]=pcm[j*2+1]=(j%48<24)?1700000000:-1700000000;
  struct timespec a,b;struct rusage ua,ub;getrusage(RUSAGE_SELF,&ua);clock_gettime(CLOCK_MONOTONIC,&a);double peak=0;
  if(realtime){for(unsigned pos=0;pos<frames;){unsigned n=1920;if(n>frames-pos)n=frames-pos;struct timespec begin,end;clock_gettime(CLOCK_MONOTONIC,&begin);api->process(d,pcm+2*pos,n);clock_gettime(CLOCK_MONOTONIC,&end);double cost=(end.tv_sec-begin.tv_sec)+(end.tv_nsec-begin.tv_nsec)*1e-9;if(cost>peak)peak=cost;pos+=n;double deadline=(double)pos/rate,spent=(end.tv_sec-a.tv_sec)+(end.tv_nsec-a.tv_nsec)*1e-9;if(deadline>spent)usleep((unsigned)((deadline-spent)*1e6));}}
  else api->process(d,pcm,frames);
  clock_gettime(CLOCK_MONOTONIC,&b);getrusage(RUSAGE_SELF,&ub);
  double elapsed=(b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)*1e-9,user=(ub.ru_utime.tv_sec-ua.ru_utime.tv_sec)*1000+(ub.ru_utime.tv_usec-ua.ru_utime.tv_usec)*.001,sys=(ub.ru_stime.tv_sec-ua.ru_stime.tv_sec)*1000+(ub.ru_stime.tv_usec-ua.ru_stime.tv_usec)*.001;printf("rate=%u mask=%u audio_ms=%u process_ms=%.3f user_ms=%.3f sys_ms=%.3f latency_frames=%u max_chunk_ms=%.3f\n",rate,mask,frames*1000/rate,elapsed*1000,user,sys,latency,peak*1000);api->stats(d,&available,&latency,&clipped);unsigned late,done;api->health(d,&late,&done);printf("available=%u underflows=%u completed=%u\n",available,late,done);fflush(stdout);free(pcm);api->destroy(d);if(realtime&&(late||(available&mask)!=mask||elapsed>(double)frames/rate+0.05))return 8;
 }
 dlclose(lib);return 0;
}
static int probe(int argc,char **argv,char **env){(void)argc;(void)argv;(void)env;char *lib=getenv("DSP_BENCH_LIB"),*root=getenv("DSP_BENCH_ROOT");char *args[]={"benchmark",lib?lib:"/tmp/sdcard/DISC_MODULES/disc-audio-dsp/1.0.0/libdisc-audio-dsp.so",root?root:"/tmp/sdcard/DISC_MODULES/disc-audio-dsp/1.0.0"};return benchmark(3,args);}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(probe,argc,argv,init,fini,rtld_fini,stack):90;
}
