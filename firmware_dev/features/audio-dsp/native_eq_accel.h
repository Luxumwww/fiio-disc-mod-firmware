#include "native_eq_api.h"
static DiscNativeEqKernel native_eq_kernel;
static unsigned native_eq_calls,native_eq_fallbacks,native_eq_stub_address;
static int native_eq_ready,native_eq_attempted;
static int native_eq_process(void *pcm,int bytes){
 unsigned pointer=PC_WORD(0x85f420),capacity=PC_WORD(0x85f424);
 /* Let the original function allocate/grow its own buffer, handle malformed
  * packets and retain its return value. malloc's native 8-byte alignment is
  * sufficient for the explicitly aligned(8) vector kernel. */
 if(!native_eq_ready||!dsp_enabled||!native_eq_kernel||!pcm||bytes<=0||
    (bytes&7)||bytes>245760||!pointer||(pointer&7)||(unsigned)bytes>capacity/2){
  native_eq_fallbacks++;return PC_FN(0x45b960,int (*)(void *,int))(pcm,bytes);
 }
 native_eq_kernel(pcm,(unsigned)bytes/8,(void *)(unsigned long)pointer,(const void *)0x85f42c,
  (const void *)0x85f458,(void *)0x85f6d8,(void *)0x85f548);
 native_eq_calls++;return (int)capacity;
}
#ifndef DISC_NATIVE_EQ_TEST
extern void *mmap(void *,unsigned,int,int,int,long);
extern int mprotect(void *,unsigned,int),munmap(void *,unsigned),cacheflush(void *,int,int);
static int native_eq_install(void){
 if(native_eq_ready)return 1;
 if(native_eq_attempted||!native_eq_kernel)return 0;native_eq_attempted=1;
 if(PC_WORD(0x45c8d0)!=0x0c116e58u||PC_WORD(0x45c8d4)!=0x02e02025u)return 0;
 unsigned *stub=mmap((void *)0x08000000,4096,3,0x802,-1,0);
 unsigned address=(unsigned)(unsigned long)stub,target=(unsigned)(unsigned long)native_eq_process;
 if(address<0x10000||address>=0x10000000u||(address&4095)){
  if(stub!=(void *)-1)munmap(stub,4096);return 0;
 }
 stub[0]=0x3c190000u|(target>>16);stub[1]=0x37390000u|(target&0xffff);
 stub[2]=0x03200008u;stub[3]=0;
 if(cacheflush(stub,16,3)||mprotect(stub,4096,5)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x45c000,4096,7)){munmap(stub,4096);return 0;}
 PC_WORD(0x45c8d0)=0x0c000000u|(address>>2);__sync_synchronize();
 int flush=cacheflush((void *)0x45c000,4096,3);
 if(flush){PC_WORD(0x45c8d0)=0x0c116e58u;cacheflush((void *)0x45c000,4096,3);}
 int protect=mprotect((void *)0x45c000,4096,5);
 /* Once published, keep the thunk mapped for the process lifetime. A failed
  * setup always calls the unchanged original kernel through the wrapper. */
 native_eq_stub_address=address;native_eq_ready=!flush&&!protect;return native_eq_ready;
}
#endif
