#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dlfcn.h>
#include <time.h>
#include <assert.h>
#include "third_party/soxr/src/soxr.h"
#ifdef __mips__
extern int sched_setaffinity(int,unsigned,const void *);
#endif
static float input[8192*2],low[8192*2],output[65536*2];
static double ms(void){struct timespec t;clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);return t.tv_sec*1000.+t.tv_nsec*1e-6;}
int main(int argc,char **argv){
 assert(argc==2);
#ifdef __mips__
 unsigned mask=1;assert(!sched_setaffinity(0,sizeof(mask),&mask));
#endif
 void *lib=dlopen(argv[1],2);if(!lib){puts(dlerror());return 2;}
 soxr_t (*create)(double,double,unsigned,soxr_error_t *,const soxr_io_spec_t *,const soxr_quality_spec_t *,const soxr_runtime_spec_t *)=dlsym(lib,"soxr_create");
 soxr_quality_spec_t (*quality)(unsigned long,unsigned long)=dlsym(lib,"soxr_quality_spec");soxr_io_spec_t (*io_spec)(soxr_datatype_t,soxr_datatype_t)=dlsym(lib,"soxr_io_spec");soxr_runtime_spec_t (*runtime)(unsigned)=dlsym(lib,"soxr_runtime_spec");
 soxr_error_t (*process)(soxr_t,soxr_in_t,size_t,size_t *,soxr_out_t,size_t,size_t *)=dlsym(lib,"soxr_process");void (*destroy)(soxr_t)=dlsym(lib,"soxr_delete");double (*delay)(soxr_t)=dlsym(lib,"soxr_delay");assert(create&&quality&&io_spec&&runtime&&process&&destroy&&delay);
 for(unsigned i=0;i<8192*2;i++)input[i]=.4f*sin(i*.003);
 const unsigned rates[]={88200,192000,384000};
 for(unsigned j=0;j<3;j++)for(unsigned ratio=2;ratio<=8;ratio*=2){unsigned rate=rates[j],target=rate/ratio;if(target<44100)continue;
  soxr_error_t err=0;soxr_quality_spec_t q=quality(SOXR_VHQ,SOXR_DOUBLE_PRECISION);q.passband_end=.95;soxr_io_spec_t io=io_spec(SOXR_FLOAT32_I,SOXR_FLOAT32_I);soxr_runtime_spec_t r=runtime(1);r.log2_min_dft_size=8;r.log2_large_dft_size=12;
  soxr_t down=create(rate,target,2,&err,&io,&q,&r),up=create(target,rate,2,&err,&io,&q,&r);assert(down&&up&&!err);
  double start=ms();size_t frames=0,low_total=0,out_total=0;
  for(unsigned i=0;i<8;i++){size_t used,nlow,nout,low_used;assert(!process(down,input,8192,&used,low,8192,&nlow));assert(used==8192);assert(!process(up,low,nlow,&low_used,output,65536,&nout));assert(low_used==nlow);frames+=8192;low_total+=nlow;out_total+=nout;}
  double cost=ms()-start;printf("SOXR VHQ %u -> %u -> %u frames=%u cpu_ms=%.3f budget_ms=%.3f low=%u out=%u delay=%.3f\n",rate,target,rate,(unsigned)frames,cost,frames*1000./rate,(unsigned)low_total,(unsigned)out_total,delay(down)*ratio+delay(up));destroy(down);destroy(up);
 }
 fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nmove $a0,$sp\nlw $a0,0($a0)\naddiu $a1,$sp,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
