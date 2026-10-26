#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
int main(void){
 unsigned *p=mmap(0,4096,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);if(p==MAP_FAILED)return 2;
 p[0]=0x24020007;p[1]=0x03e00008;p[2]=0;
 if(syscall(4147,p,4096,3))return 3;
 int (*run)(void)=(int (*)(void))p;if(run()!=7)return 4;
 for(unsigned i=0;i<32;i++){unsigned value=10+i;p[0]=0x24020000|value;
  if(syscall(4147,p,4096,3))return 5;if(run()!=(int)value)return 6;}
 printf("PASS: actual MIPS cacheflush accepted; 32 code updates execute refreshed instructions. No UI/audio/device calls.\n");return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
