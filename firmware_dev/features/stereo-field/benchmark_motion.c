/* Finite isolated renderer timing; never opens an audio output or UI handle. */
#include "../audio-visuals/polar/polar_api.h"
extern void *dlopen(const char *,int),*dlsym(void *,const char *);
extern int printf(const char *,...),open(const char *,int,...),close(int),read(int,void *,unsigned),memcmp(const void *,const void *,unsigned),sscanf(const char *,const char *,...);
extern unsigned usleep(unsigned);
struct timespec {long tv_sec,tv_nsec;};
extern int clock_gettime(int,struct timespec *);
#include "../settings-debug/cache_sync.h"
static short pcm[960][2];
static double stamp(int clock){struct timespec t;clock_gettime(clock,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(void){
 const char *path="/tmp/disc-stereo-probe.so";void *h=dlopen(path,2);if(!h||!module_cache_sync(path))return 3;
 const DiscStereoApi *(*get)(unsigned)=dlsym(h,"disc_stereo_get_api");const DiscStereoApi *a=get?get(1):0;if(!a)return 4;
 a->reset();double total=0,maximum=0,wallmax=0;
 for(unsigned f=0;f<64;f++){
  for(unsigned i=0;i<960;i++){pcm[i][0]=(short)((i+f*131)*137);pcm[i][1]=(short)((i+f*211)*227);}
  double w=stamp(1),c=stamp(2);a->feed(pcm,960,48000,1000+f*16);a->tick(1000+f*16);if(a->compose(360,0)->size!=518400)return 5;
  double cost=stamp(2)-c,elapsed=stamp(1)-w;total+=cost;if(cost>maximum)maximum=cost;if(elapsed>wallmax)wallmax=elapsed;
  if(elapsed<.016)usleep((unsigned)((.016-elapsed)*1e6));
 }
 printf("frames=64 cpu_ms=%.3f avg_cpu_ms=%.3f max_cpu_ms=%.3f max_wall_ms=%.3f\n",total*1000,total*1000/64,maximum*1000,wallmax*1000);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
