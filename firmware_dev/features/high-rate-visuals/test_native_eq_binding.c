#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
static void *scratch;
static void reserve(void){scratch=mmap((void *)0x08000000,4096,3,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);assert(scratch==(void *)0x08000000);}
static unsigned buffer_word,capacity_word,call_word,delay_word;
static unsigned *word(unsigned a){switch(a){case 0x85f420:return &buffer_word;case 0x85f424:return &capacity_word;case 0x45c8d0:return &call_word;case 0x45c8d4:return &delay_word;default:assert(0);return 0;}}
static unsigned old_calls,kernel_calls,flush_calls,protect_calls,unmaps;
static unsigned fail_flush,fail_protect,map_mode;
static int dsp_enabled=1;
static int original(void *p,int n){(void)p;(void)n;old_calls++;buffer_word=0x100008;capacity_word=128;return 77;}
static void *map_stub(void *a,unsigned n,int p,int f,int d,long o){assert(a==(void *)0x08000000&&n==4096&&p==3&&f==0x802&&d==-1&&!o);return map_mode==1?(void *)-1:map_mode==2?(void *)0x18000000:scratch;}
static int protect_stub(void *p,unsigned n,int mode){assert(n==4096);assert((p==scratch&&mode==5)||(p==(void *)0x45c000&&(mode==5||mode==7)));return ++protect_calls==fail_protect?-1:0;}
static int unmap_stub(void *p,unsigned n){assert(p&&n==4096);unmaps++;return 0;}
static int flush_stub(void *p,int n,int mode){assert(mode==3);assert((p==scratch&&n==16)||(p==(void *)0x45c000&&n==4096));return ++flush_calls==fail_flush?-1:0;}
#define PC_WORD(a) (*word(a))
#define PC_FN(a,t) original
#define mmap map_stub
#define mprotect protect_stub
#define munmap unmap_stub
#define cacheflush flush_stub
#include "../audio-dsp/native_eq_accel.h"
static void kernel(int *pcm,unsigned n,double *b,const unsigned *a,const float c[10][5],double l[10][5],double r[10][5]){
 assert(pcm&&n==4&&b==(void *)0x100008&&a==(void *)0x85f42c&&c==(void *)0x85f458&&l==(void *)0x85f6d8&&r==(void *)0x85f548);kernel_calls++;
}
static void reset(void){native_eq_ready=native_eq_attempted=0;native_eq_stub_address=0;call_word=0x0c116e58;delay_word=0x02e02025;flush_calls=protect_calls=unmaps=fail_flush=fail_protect=map_mode=0;native_eq_kernel=kernel;}
int main(void){
 reserve();reset();assert(native_eq_install());assert(native_eq_ready&&call_word==(0x0c000000|(0x08000000>>2)));assert(((unsigned *)scratch)[2]==0x03200008);assert(protect_calls==3);assert(native_eq_install()&&protect_calls==3);
 int pcm[8]={0};assert(native_eq_process(pcm,32)==77&&old_calls==1);
 assert(native_eq_process(pcm,32)==128&&kernel_calls==1);
 dsp_enabled=0;assert(native_eq_process(pcm,32)==77);dsp_enabled=1;
 buffer_word+=4;assert(native_eq_process(pcm,32)==77);
 capacity_word=16;assert(native_eq_process(pcm,32)==77);
 int invalid[]={0,-1,7,245768};for(unsigned i=0;i<4;i++)assert(native_eq_process(pcm,invalid[i])==77);
 native_eq_kernel=0;assert(native_eq_process(pcm,32)==77);assert(kernel_calls==1);
 reset();native_eq_kernel=0;assert(!native_eq_install()&&!native_eq_attempted);native_eq_kernel=kernel;assert(native_eq_install());
 reset();call_word=0;assert(!native_eq_install()&&call_word==0);
 reset();delay_word=0;assert(!native_eq_install()&&call_word==0x0c116e58);
 for(unsigned i=1;i<=2;i++){reset();map_mode=i;assert(!native_eq_install()&&call_word==0x0c116e58);}
 reset();fail_flush=1;assert(!native_eq_install()&&unmaps==1&&call_word==0x0c116e58);
 reset();fail_protect=1;assert(!native_eq_install()&&unmaps==1&&call_word==0x0c116e58);
 reset();fail_protect=2;assert(!native_eq_install()&&unmaps==1&&call_word==0x0c116e58);
 reset();fail_flush=2;assert(!native_eq_install()&&call_word==0x0c116e58&&unmaps==0);
 reset();fail_protect=3;assert(!native_eq_install()&&unmaps==0);assert(native_eq_process(pcm,32)==77);
 puts("PASS production native EQ binding: buffer allocation/growth/format fallback; native data/history addresses; disable/missing kernel; guarded atomic JAL; cache/protection failures and retained thunk");fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
