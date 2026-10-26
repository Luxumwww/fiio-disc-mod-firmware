#define _GNU_SOURCE
/* Isolated executable at09000000: stock reference mapped only in this probe. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/resource.h>
#include <sched.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include "../settings-debug/cache_sync.h"
#include "reports/eq-candidate/stock_function.h"
extern void native_eq_candidate(int *,unsigned,double *,const unsigned *,const float [10][5],double [10][5],double [10][5]);
extern int cacheflush(void *,int,int);
#define CHECK(x) do{if(!(x)){printf("FAIL line%u\n",__LINE__);fflush(stdout);return 1;}}while(0)
static _Alignas(16) double oldbuffer[32768],newbuffer[32768];static int original[32768],test[32768];
static unsigned *active=(void *)0x85f42c;static float (*coeff)[5]=(void *)0x85f458;static double (*left)[5]=(void *)0x85f6d8,(*right)[5]=(void *)0x85f548;
static double ls[10][5],rs[10][5];
static void (*kernel)(int *,unsigned,double *,const unsigned *,const float [10][5],double [10][5],double [10][5])=native_eq_candidate;
static void candidate(int *pcm,unsigned n,double *buf,const unsigned *a,const float c[10][5],double l[10][5],double r[10][5]){
 if(!getenv("EQ_SELF")){kernel(pcm,n,buf,a,c,l,r);return;}
 double sl[10][5],sr[10][5];memcpy(sl,left,400);memcpy(sr,right,400);memcpy(left,l,400);memcpy(right,r,400);
 ((int (*)(void *,int))0x45b960)(pcm,n*8);memcpy(l,left,400);memcpy(r,right,400);memcpy(left,sl,400);memcpy(right,sr,400);
}
static unsigned rng=206;static void input(unsigned n,unsigned phase){for(unsigned i=0;i<n*2;i++){rng=rng*1664525u+1013904223u;original[i]=phase==1?0:(int)rng/(phase==2?10000:2);}}
static double cpu(void){struct timespec t;clock_gettime(2,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(void){
 setpriority(PRIO_PROCESS,0,19);if(getenv("EQ_CPU")){cpu_set_t mask;CPU_ZERO(&mask);CPU_SET((*getenv("EQ_CPU")-'0'),&mask);CHECK(!sched_setaffinity(0,sizeof mask,&mask));}
 if(getenv("EQ_LIB")){void *lib=dlopen(getenv("EQ_LIB"),2);CHECK(lib&&module_cache_sync(getenv("EQ_LIB")));kernel=dlsym(lib,"disc_dsp_native_eq");CHECK(kernel);}
 CHECK(mmap((void *)0x830000,0x30000,3,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)==(void *)0x830000);
 CHECK(mmap((void *)0x45b000,4096,3,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)==(void *)0x45b000);
 CHECK(mmap((void *)0x6c2000,4096,3,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)==(void *)0x6c2000);
 memcpy((void *)0x45b000,stock_eq_function,sizeof stock_eq_function);double limit=1.7976931348623157e308;memcpy((void *)0x6c2d10,&limit,8);CHECK(!cacheflush((void *)0x45b000,4096,3)&&!mprotect((void *)0x45b000,4096,5));
 *(double **)0x85f420=oldbuffer+1;*(unsigned *)0x85f424=sizeof oldbuffer-8;
 int (*reference)(void *,int)=(void *)0x45b960;unsigned blocks=0;
 for(unsigned ratei=0;ratei<3;ratei++)for(unsigned variant=0;variant<16;variant++){
  unsigned rate=(unsigned[]){48000,192000,384000}[ratei];memset(left,0,400);memset(right,0,400);memset(ls,0,sizeof ls);memset(rs,0,sizeof rs);
  for(unsigned j=0;j<10;j++){
   active[j]=(variant==0||variant==15)?1:(j+variant)%3!=0;
   double f=32*pow(2,j),w=6.283185307179586*f/rate,A=pow(10,((int)(variant%5)-2)*3/40.),alpha=sin(w)/1.4142135623730951,a0=1+alpha/A;
   coeff[j][0]=(1+alpha*A)/a0;coeff[j][1]=-2*cos(w)/a0;coeff[j][2]=(1-alpha*A)/a0;coeff[j][3]=2*cos(w)/a0;coeff[j][4]=-(1-alpha/A)/a0;
  }
  if(variant==15){coeff[3][0]=INFINITY;coeff[7][1]=NAN;}
  for(unsigned k=0;k<12;k++){
   unsigned n=(unsigned[]){1,64,257,1024}[k%4];input(n,k%3);memcpy(test,original,n*8);reference(original,n*8);candidate(test,n,newbuffer+1,active,coeff,ls,rs);
   if(memcmp(test,original,n*8)||memcmp(ls,left,400)||memcmp(rs,right,400)){for(unsigned z=0;z<n*2;z++)if(test[z]!=original[z]){printf("sample%u new=%d old=%d\n",z,test[z],original[z]);break;}for(unsigned z=0;z<50;z++)if(((double *)ls)[z]!=((double *)left)[z]){printf("leftstate%u new=%a old=%a\n",z,((double *)ls)[z],((double *)left)[z]);break;}for(unsigned z=0;z<50;z++)if(((double *)rs)[z]!=((double *)right)[z])printf("state%u new=%a old=%a\n",z,((double *)rs)[z],((double *)right)[z]);printf("DIFF rate=%u variant=%u block=%u samples=%u pcm=%d L=%d R=%d\n",rate,variant,k,n,memcmp(test,original,n*8),memcmp(ls,left,400),memcmp(rs,right,400));CHECK(0);}blocks++;
  }
 }
 printf("PASS actual stock45b960/S32+complete histories exact blocks=%u rates48/192/384/all10+6bands/silence/nonfinite-reset\n",blocks);
 for(unsigned j=0;j<10;j++){active[j]=(j>=3&&j!=6);coeff[j][0]=1;coeff[j][1]=-.9f;coeff[j][2]=.2f;coeff[j][3]=.8f;coeff[j][4]=-.1f;}
 memset(left,0,400);memset(right,0,400);memset(ls,0,sizeof ls);memset(rs,0,sizeof rs);double totals[2]={0},max[2]={0};
 for(unsigned k=0;k<8;k++){input(7680,k%3);memcpy(test,original,7680*8);double t=cpu();reference(original,7680*8);double old=cpu()-t;t=cpu();candidate(test,7680,newbuffer+1,active,coeff,ls,rs);double newer=cpu()-t;if(memcmp(test,original,7680*8)||memcmp(ls,left,400)||memcmp(rs,right,400)){printf("BENCH_DIFF block=%u PCM=%d L=%d R=%d\n",k,memcmp(test,original,7680*8),memcmp(ls,left,400),memcmp(rs,right,400));for(unsigned z=0;z<15360;z++)if(test[z]!=original[z]){printf("first sample%u new=%d old=%d\n",z,test[z],original[z]);break;}for(unsigned z=0;z<50;z++)if(((double *)rs)[z]!=((double *)right)[z]){printf("right state%u new=%a old=%a\n",z,((double *)rs)[z],((double *)right)[z]);break;}CHECK(0);}if(k>=4){totals[0]+=old;totals[1]+=newer;if(old>max[0])max[0]=old;if(newer>max[1])max[1]=newer;}}
 printf("BENCH six native EQ bands384000 frames7680 blocks4 old_ms=%.3f new_ms=%.3f old_max=%.3f new_max=%.3f ratio=%.4f\n",totals[0]*1000,totals[1]*1000,max[0]*1000,max[1]*1000,totals[1]/totals[0]);fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
