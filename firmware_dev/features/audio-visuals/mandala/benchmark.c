/* Bounded renderer probe, no audio devices and no UI manipulation. */
#include "mandala_api.h"
extern void *dlopen(const char *,int),*dlsym(void *,const char *);
extern int printf(const char *,...),open(const char *,int,...),close(int),read(int,void *,unsigned),memcmp(const void *,const void *,unsigned),sscanf(const char *,const char *,...);
extern unsigned usleep(unsigned);
struct timespec {long tv_sec,tv_nsec;};
extern int clock_gettime(int,struct timespec *);
#include "../../settings-debug/cache_sync.h"
static double stamp(int clock){struct timespec t;clock_gettime(clock,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static int probe(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;
 const char *path="/tmp/disc-runtime/mandala-probe-1374/libdisc-audio-mandala.so";
 void *lib=dlopen(path,2);if(!lib||!module_cache_sync(path))return 3;
 const DiscMandalaApi *(*get)(unsigned)=dlsym(lib,"disc_mandala_get_api");if(!get)return 4;
 const DiscMandalaApi *api=get(1);if(!api)return 5;api->reset();
 short pcm[960][2];for(unsigned i=0;i<960;i++){pcm[i][0]=(short)(i*137);pcm[i][1]=(short)(i*227);}
 api->feed(pcm,960,48000,1000);api->tick(1000);double cpu=0,peak=0,wallpeak=0;
 for(unsigned i=0;i<120;i++){
  double w=stamp(1),c=stamp(2);api->feed(pcm,960,48000,1016+i*16);api->tick(1016+i*16);
  const DiscVisualImage *image=api->compose(360,0);if(!image||image->size!=518400)return 6;
  double cost=stamp(2)-c,elapsed=stamp(1)-w;cpu+=cost;if(cost>peak)peak=cost;if(elapsed>wallpeak)wallpeak=elapsed;
  if(elapsed<.016)usleep((unsigned)((.016-elapsed)*1e6));
 }
 printf("frames=120 cpu_ms=%.3f avg_cpu_ms=%.3f max_cpu_ms=%.3f max_wall_ms=%.3f\n",cpu*1000,cpu*1000/120,peak*1000,wallpeak*1000);return 0;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");return next?next(probe,argc,argv,init,fini,rtld_fini,stack):90;
}
