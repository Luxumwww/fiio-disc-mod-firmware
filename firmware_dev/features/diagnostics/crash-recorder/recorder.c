#define _GNU_SOURCE
#include <signal.h>
#include <sys/ucontext.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
_Static_assert(sizeof(void *)==4,"MIPS o32 only");
_Static_assert(offsetof(ucontext_t,uc_mcontext)==24,"Verified o32 context offset");
_Static_assert(offsetof(mcontext_t,pc)==8,"Verified o32 PC offset");
_Static_assert(offsetof(mcontext_t,gregs)==16,"Verified o32 GPR offset");
typedef struct {uint32_t magic,version,pid,signal,code,address,sender,stage,interface_address;uint32_t pc,sp,ra,gp,t9;uint32_t regs[32];} Fault;
static struct sigaction previous[128];
static int armed[128],fd=-1;
static volatile sig_atomic_t stage;
static uint32_t interface_address;
static char prefix[160];
static void caught(int sig,siginfo_t *info,void *context){
 Fault fault={0};fault.magic=0x44435231;fault.version=1;fault.pid=(uint32_t)getpid();fault.signal=(uint32_t)sig;
 fault.code=info?(uint32_t)info->si_code:0;
 if(info){if(info->si_code>0)fault.address=(uint32_t)(uintptr_t)info->si_addr;else fault.sender=(uint32_t)info->si_pid;}
 fault.stage=(uint32_t)stage;fault.interface_address=interface_address;
 if(context){ucontext_t *u=context;fault.pc=(uint32_t)u->uc_mcontext.pc;for(unsigned i=0;i<32;i++)fault.regs[i]=(uint32_t)u->uc_mcontext.gregs[i];fault.sp=fault.regs[29];fault.ra=fault.regs[31];fault.gp=fault.regs[28];fault.t9=fault.regs[25];}
 if(fd>=0)(void)write(fd,&fault,sizeof fault);
 /* Record only; preserve the original action, including custom handlers.
  * Default signals are re-raised while blocked and delivered on return.
  * No recovery, GUI calls, allocation, formatting, or file-open in handler. */
 if(sig<=0||sig>=128)return;
 struct sigaction old=previous[sig];
 if(old.sa_flags&SA_RESETHAND){struct sigaction reset={0};reset.sa_handler=SIG_DFL;sigemptyset(&reset.sa_mask);sigaction(sig,&reset,0);}else sigaction(sig,&old,0);
 armed[sig]=0;
 if(old.sa_handler==SIG_IGN)return;
 if(old.sa_handler!=SIG_DFL){if(old.sa_flags&SA_SIGINFO)old.sa_sigaction(sig,info,context);else old.sa_handler(sig);return;}
 (void)raise(sig);
}
int disc_diag_install(const char *base){
 if(fd>=0)return 1;if(!base||strlen(base)>100)return 0;
 snprintf(prefix,sizeof prefix,"%s-%d",base,getpid());char path[192];snprintf(path,sizeof path,"%s.bin",prefix);
 fd=open(path,O_WRONLY|O_CREAT|O_APPEND,0600);if(fd<0)return 0;
 const int signals[]={SIGSEGV,SIGBUS,SIGILL,SIGFPE,SIGABRT,SIGTERM,SIGHUP};
 unsigned installed=0;
 for(unsigned i=0;i<sizeof signals/sizeof signals[0];i++){
  int s=signals[i];if(sigaction(s,0,&previous[s]))continue;
  struct sigaction action=previous[s];action.sa_sigaction=caught;
  action.sa_flags=(action.sa_flags&~(SA_RESETHAND|SA_NODEFER))|SA_SIGINFO;
  if(!sigaction(s,&action,0)){armed[s]=1;installed++;}
 }
 return installed==sizeof signals/sizeof signals[0];
}
void disc_diag_checkpoint(unsigned value,const void *pointer){
 interface_address=(uint32_t)(uintptr_t)pointer;stage=(sig_atomic_t)value;
 if(fd<0)return;
 /* Mapping snapshot outside the signal handler, after provider dlopen. */
 char path[192],buffer[1024];snprintf(path,sizeof path,"%s.maps",prefix);
 int source=open("/proc/self/maps",O_RDONLY),target=open(path,O_WRONLY|O_CREAT|O_TRUNC,0600);
 if(source>=0&&target>=0){unsigned total=0;ssize_t n;while(total<65536&&(n=read(source,buffer,sizeof buffer))>0){if(write(target,buffer,(size_t)n)!=n)break;total+=(unsigned)n;}}
 if(source>=0)close(source);if(target>=0)close(target);
 snprintf(path,sizeof path,"%s.context",prefix);target=open(path,O_WRONLY|O_CREAT|O_TRUNC,0600);
 if(target>=0){int n=snprintf(buffer,sizeof buffer,"pid=%d\nstage=%u\ninterface=0x%08x\n",getpid(),value,interface_address);write(target,buffer,(size_t)n);close(target);}
}
