#include "syscalls.h"
/* BusyBox on this device lacks mv -T. rename(2) replaces a symlink atomically. */
static int permitted(const char *p) {
 const char *prefix="/usr/data/disc-runtime/";
 int i=0;for(;prefix[i];i++)if(p[i]!=prefix[i])return 0;
 for(;p[i];i++)if(p[i]=='.' && p[i+1]=='.')return 0;
 return 1;
}
void disc_main(long *stack) {
 char **argv=(char **)(stack+1);
 if(stack[0]!=3 || !permitted(argv[1]) || !permitted(argv[2]))disc_exit(2);
 long rc=disc_sys3(4038,(long)argv[1],(long)argv[2],0);
 disc_exit(rc<0?1:0);
}
__asm__(".text\n.globl _start\n_start:\nmove $4,$29\naddiu $29,$29,-32\njal disc_main\nnop\n1: b 1b\nnop\n");
