#define _GNU_SOURCE
#include <linux/capability.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
/* Host-only launcher used after mounting private proc/mqueue. */
int main(int argc,char **argv){
 if(argc<3||chroot(argv[1])||chdir("/")){perror("emu chroot");return 2;}
 const int drop[]={CAP_NET_ADMIN,CAP_SYS_TIME,CAP_SYS_BOOT,CAP_SYS_MODULE,CAP_SYS_RAWIO,CAP_SYS_ADMIN};
 struct __user_cap_header_struct h={_LINUX_CAPABILITY_VERSION_3,0};
 struct __user_cap_data_struct d[2];
 if(syscall(SYS_capget,&h,d)){perror("emu capget");return 2;}
 for(unsigned i=0;i<sizeof(drop)/sizeof(drop[0]);i++){
  if(prctl(PR_CAPBSET_DROP,drop[i],0,0,0)){perror("emu bounding");return 2;}
  unsigned bit=1u<<(drop[i]%32),bank=drop[i]/32;
  d[bank].effective&=~bit;d[bank].permitted&=~bit;d[bank].inheritable&=~bit;
 }
 if(syscall(SYS_capset,&h,d)||prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0)){perror("emu capabilities");return 2;}
 execv(argv[2],argv+2);perror("emu exec");return 127;
}
