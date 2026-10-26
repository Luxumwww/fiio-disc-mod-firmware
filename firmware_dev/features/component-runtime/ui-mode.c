#include "syscalls.h"
/* Read only. Caller pins the stock UI hash before passing its live PID.
 * V2.57 work_mode global is verified at 0x8e16d4. Never attach/write UI memory.
 */
void disc_main(long *stack) {
 char **argv=(char **)(stack+1);char path[40];unsigned i=0,j=0,mode;
 const char *prefix="/proc/";for(;prefix[j];j++)path[j]=prefix[j];
 if(stack[0]!=2)disc_exit(2);
 while(argv[1][i]){
  if(i>=10||argv[1][i]<'0'||argv[1][i]>'9')disc_exit(2);
  path[j++]=argv[1][i++];
 }
 if(!i)disc_exit(2);
 const char *tail="/mem";for(i=0;tail[i];i++)path[j++]=tail[i];path[j]=0;
 long fd=disc_sys3(4005,(long)path,0,0);if(fd<0)disc_exit(3);
 long rc=disc_sys3(4019,fd,0x8e16d4,0);
 if(rc!=0x8e16d4){disc_close(fd);disc_exit(4);}
 rc=disc_read(fd,&mode,4);disc_close(fd);
 if(rc!=4||mode>5)disc_exit(5);
 char result[2]={(char)('0'+mode),'\n'};
 if(disc_sys3(4004,1,(long)result,2)!=2)disc_exit(6);
 disc_exit(0);
}
__asm__(".text\n.globl _start\n_start:\nmove $4,$29\naddiu $29,$29,-32\njal disc_main\nnop\n1: b 1b\nnop\n");
