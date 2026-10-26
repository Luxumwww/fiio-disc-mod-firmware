#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#ifdef __mips__
extern int sched_setaffinity(int,unsigned,const void *);
#endif
#include "resample_api.h"
static int input[8192*2],a[8192*2],b[8192*2];
static int identity(void *context,float *data,unsigned frames){(void)context;(void)data;(void)frames;return 1;}
static double milliseconds(void){struct timespec t;clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);return t.tv_sec*1000.+t.tv_nsec*1e-6;}
int main(int argc,char **argv){
 assert(argc==3);
#ifdef __mips__
 unsigned affinity=1;assert(!sched_setaffinity(0,sizeof(affinity),&affinity));
#endif
 void *first=dlopen(argv[1],2),*second=dlopen(argv[2],2);if(!first||!second){puts(dlerror());return 2;}
 const DiscResampleApi *(*get1)(void)=dlsym(first,"disc_resample_get_api"),*(*get2)(void)=dlsym(second,"disc_resample_get_api");assert(get1&&get2);const DiscResampleApi *x=get1(),*y=get2();
 const unsigned rates[]={88200,96000,176400,192000,352800,384000};unsigned cases=0;
 for(unsigned r=0;r<6;r++)for(unsigned ratio=2;ratio<=8;ratio*=2){
  unsigned rate=rates[r],target=rate/ratio;if(target<44100)continue;void *c=x->create(rate,target),*d=y->create(rate,target);assert(c&&d);
  double scalar=0,msa=0;
  for(unsigned pass=0;pass<3;pass++){
   unsigned frames=pass==0?8192:pass==1?17:4093;
   for(unsigned i=0;i<frames;i++){input[2*i]=(int)(sin((pass*8192.+i)*.123)*700000000.);input[2*i+1]=(int)(cos((pass*8192.+i)*.057)*400000000.);}
   memcpy(a,input,frames*8);memcpy(b,input,frames*8);
   double t=milliseconds();assert(x->process(c,a,frames,identity,0));scalar+=milliseconds()-t;
   t=milliseconds();assert(y->process(d,b,frames,identity,0));msa+=milliseconds()-t;
   assert(!memcmp(a,b,frames*8));
  }
  unsigned latency,errors,underflows,invalid;y->stats(d,&latency,&errors,&underflows,&invalid);assert(!errors&&!underflows&&!invalid);
  printf("PASS %u -> %u exact; frames12302 scalar_ms=%.3f msa_ms=%.3f latency=%u\n",rate,target,scalar,msa,latency);cases++;x->destroy(c);y->destroy(d);
 }
 printf("PASS contexts=%u exact MSA against original Speex Q10 scalar; no audio access\n",cases);fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nmove $a0,$sp\nlw $a0,0($a0)\naddiu $a1,$sp,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
