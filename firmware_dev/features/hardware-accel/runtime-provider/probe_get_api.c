/* Isolated process only: maps two stock code pages as DATA and checks API.
 * Never calls DPU callbacks, accesses a device, or touches the running UI. */
extern int open(const char *,int,...),close(int),printf(const char *,...);
extern void *mmap(void *,unsigned,int,int,int,long),*dlopen(const char *,int),*dlsym(void *,const char *);
#include "accel_api.h"
int main(void){
 int fd=open("/usr/bin/mq_ui",0);if(fd<0)return 2;
 if(mmap((void *)0x51a000,4096,3,0x12,fd,0x11a000)!=(void *)0x51a000)return 3;
 if(mmap((void *)0x565000,4096,3,0x12,fd,0x165000)!=(void *)0x565000)return 4;
 close(fd);
 void *library=dlopen("/tmp/disc-api-probe-provider.so",2);if(!library)return 5;
 const DiscAccelApi *(*get)(unsigned)=(const DiscAccelApi *(*)(unsigned))dlsym(library,"disc_accel_get_api");
 if(!get)return 6;
 printf("isolated_getter=%p\n",(void *)get);
 if(get(2))return 7;
 const DiscAccelApi *api=get(1);if(!api||api->abi!=1||api->size!=60||api->width!=360||api->height!=360||api->stride!=1440)return 8;
 volatile unsigned char *code=(void *)0x51a7e0;unsigned char saved=*code;*code=(unsigned char)(saved^1);
 int rejected=get(1)==0;*code=saved;if(!rejected)return 9;
 for(unsigned i=0;i<32;i++)if(get(1)!=api)return 10;
 printf("PASS: native ABI mismatch rejected; stock code verified; modified private code rejected;32 repeated interface reads stable. No DPU/device calls.\n");
 return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
