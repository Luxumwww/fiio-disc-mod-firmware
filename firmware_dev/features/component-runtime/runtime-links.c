#include "syscalls.h"
/* Read-only batch readlink: one short process for the entire module catalog.
 * Missing links emit an empty target, preserving registrations on absent TF.
 * No libc, audio device, writes to files, threads, or resident process. */
static void valid_text(const char *s, long length) {
 for(long j=0;j<length;j++)
  if(s[j]=='|' || s[j]=='\n' || s[j]=='\r')disc_exit(2);
}
void disc_main(long *stack) {
 char **argv=(char **)(stack+1);char target[1024];
 if(stack[0]<2 || stack[0]>257)disc_exit(2);
 for(long i=1;i<stack[0];i++) {
  long length=disc_strlen(argv[i]);
  if(length==0 || length>1023)disc_exit(2);
  valid_text(argv[i],length);
  long size;
  do{size=disc_sys3(4085,(long)argv[i],(long)target,sizeof(target));}while(size==-DISC_EINTR);
  if(size==(long)sizeof(target))disc_exit(3);
  if(size<0)size=0;
  valid_text(target,size);
  if(disc_emit(argv[i],length)!=length || disc_emit("|",1)!=1 ||
     disc_emit(target,size)!=size || disc_emit("\n",1)!=1)disc_exit(4);
 }
 disc_exit(0);
}
__asm__(".text\n.globl _start\n_start:\nmove $4,$29\naddiu $29,$29,-32\njal disc_main\nnop\n1: b 1b\nnop\n");
