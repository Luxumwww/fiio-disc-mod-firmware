/* Isolated provider load and finite memory-only native384k plotting probe. */
#include "xy/visual_api.h"
extern void *dlopen(const char *,int),*dlsym(void *,const char *);
extern int printf(const char *,...),open(const char *,int,...),close(int),read(int,void *,unsigned),memcmp(const void *,const void *,unsigned),sscanf(const char *,const char *,...);
extern float sinf(float),cosf(float);
extern int fflush(void *);
struct timespec{long s,n;};extern int clock_gettime(int,struct timespec *);
#include "../settings-debug/cache_sync.h"
static short input[8192][2];
static double cpu(void){struct timespec t;clock_gettime(2,&t);return t.s+t.n*1e-9;}
int main(void){
 const char *names[]={"xy","polar","stereo","mandala"};const char *symbols[]={"disc_xy_get_api","disc_polar_get_api","disc_stereo_get_api","disc_mandala_get_api"};
 for(unsigned k=0;k<4;k++){
  const char *path=k==0?"/tmp/disc-202-xy.so":k==3?"/tmp/disc-202-mandala.so":"/tmp/disc-202-polar.so";
  void *h=dlopen(path,2);if(!h||!module_cache_sync(path))return 2;
  const DiscXyApi *(*get)(unsigned)=dlsym(h,symbols[k]);const DiscXyApi *a=get?get(1):0;if(!a)return 3;
  for(unsigned rate=192000;rate<=384000;rate*=2)for(unsigned noise=0;noise<2;noise++){
   unsigned n=rate/50,random=123;for(unsigned i=0;i<n;i++){random=random*1664525u+1013904223u;input[i][0]=noise?(short)(random>>16):(short)(30000*sinf(6.283185307f*20000*i/rate));random=random*1664525u+1013904223u;input[i][1]=noise?(short)(random>>16):(short)(30000*cosf(6.283185307f*20000*i/rate));}
   a->reset();double total=0,max=0;
   for(unsigned f=0;f<8;f++){double begin=cpu();a->feed(input,n,rate,1000+f*32);a->tick(1000+f*32);if(a->compose(360,0)->size!=518400)return 4;double cost=cpu()-begin;total+=cost;if(cost>max)max=cost;}
   unsigned count=0,sampling=0;a->stats(&count,&sampling,0,1256);if(count!=n||sampling!=rate)return 5;
   printf("view=%s rate=%u noise=%u frames=8 avg_cpu_ms=%.3f max_cpu_ms=%.3f\n",names[k],rate,noise,total*125,max*1000);
  }
 }
 fflush(0);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
