#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ring.h"
static DiscVisualRing *r;
static int input[(DISC_VS_FRAMES+64)*2],output[DISC_VS_FRAMES*2];
static void init(void){memset(r,0,sizeof(*r));r->magic=DISC_VS_MAGIC;r->abi=1;r->bytes=sizeof(*r);r->owner=42;r->request=3;r->lease=100;r->ui_mode=4;}
static void fill(unsigned begin,unsigned n){for(unsigned i=0;i<n;i++){input[i*2]=(int)(begin+i);input[i*2+1]=-(int)(begin+i);}}
static void check(unsigned begin,unsigned n){for(unsigned i=0;i<n;i++){assert(output[i*2]==(int)(begin+i));assert(output[i*2+1]==-(int)(begin+i));}}
static int get(unsigned frames,unsigned rate,unsigned mode,unsigned pid,unsigned t){DiscVisualWindow w;return disc_vs_read(r,output,frames,rate,mode,pid,t,&w);}
int main(void){
 r=mmap(0,sizeof(*r),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);assert(r!=MAP_FAILED);init();
 assert(!get(1,384000,4,42,100));
 unsigned cursor=0;
 for(unsigned n=1;n<200000;n+=37){unsigned frames=n%16384+1;fill(cursor,frames);unsigned hash=0;for(unsigned i=0;i<frames*2;i++)hash=hash*33u+(unsigned)input[i];disc_vs_write(r,input,frames,384000,4,6,100);unsigned after=0;for(unsigned i=0;i<frames*2;i++)after=after*33u+(unsigned)input[i];assert(hash==after);cursor+=frames;unsigned count=r->valid<32768?r->valid:32768;assert(get(count,384000,4,42,100));check(cursor-count,count);}
 fill(cursor,DISC_VS_FRAMES+64);disc_vs_write(r,input,DISC_VS_FRAMES+64,384000,4,6,100);assert(get(DISC_VS_FRAMES,384000,4,42,100));check(cursor+64,DISC_VS_FRAMES);
 assert(!get(1,192000,4,42,100));assert(!get(1,384000,2,42,100));assert(!get(1,384000,4,43,100));assert(!get(1,384000,4,42,401));assert(!get(DISC_VS_FRAMES+1,384000,4,42,100));
 r->sequence|=1;assert(!get(1,384000,4,42,100));r->sequence++;
 r->request=4;unsigned seq=r->sequence;disc_vs_write(r,input,10,384000,4,6,100);assert(r->sequence==seq);assert(!get(1,384000,4,42,100));
 r->request=7;assert(!get(1,384000,4,42,100));fill(123,100);disc_vs_write(r,input,100,384000,4,6,100);assert(r->valid==100);assert(get(100,384000,4,42,100));check(123,100);
 r->ui_mode=2;r->lease=200;disc_vs_write(r,input,10,192000,2,6,200);assert(r->valid==10);assert(!get(11,192000,2,42,200));assert(get(10,192000,2,42,200));
 disc_vs_write(r,input,9,192000,2,3,200);assert(r->valid==9);disc_vs_invalidate(r);assert(!get(1,192000,2,42,200));
 r->lease=0xfffffff0u;disc_vs_write(r,input,10,192000,2,3,10);assert(get(10,192000,2,42,10));assert(!get(1,192000,2,42,1000));
 /* A concurrently replacing audio window must never publish torn channels. */
 init();int ready[2];assert(!pipe(ready));pid_t child=fork();assert(child>=0);
 if(!child){for(unsigned n=0;n<16000;n++){fill(n*256,256);disc_vs_write(r,input,256,384000,4,6,100);if(n==255)assert(write(ready[1],"x",1)==1);if((n&31)==0)usleep(10);} _exit(0);}
 char marker;assert(read(ready[0],&marker,1)==1);
 unsigned accepted=0;for(unsigned n=0;n<32000;n++){if(get(32768,384000,4,42,100)){unsigned begin=(unsigned)output[0];check(begin,32768);accepted++;}}
 int status;assert(waitpid(child,&status,0)==child&&WIFEXITED(status)&&!WEXITSTATUS(status));assert(accepted>0);
 printf("PASS: wraps, 384k FFT window, immutable input, format/route/source/PID/lease invalidation, bounded torn-read rejection (%u snapshots)\n",accepted);munmap(r,sizeof(*r));fflush(stdout);return 0;
}

#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
