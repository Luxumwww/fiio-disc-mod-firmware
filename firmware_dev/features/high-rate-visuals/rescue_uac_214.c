#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <time.h>
/* One finite call to stock input-close, only for the exact saved UAC leak.
 * No arbitrary address/action arguments, code replacement, playback or gain. */
struct regs {uint64_t r[32],lo,hi,epc,bad,status,cause;} __attribute__((aligned(8)));
static int peek_error;
static unsigned number(const char *s){unsigned n=0,base=10;if(s[0]=='0'&&s[1]=='x'){base=16;s+=2;}for(;*s;s++){unsigned d=*s>='0'&&*s<='9'?*s-'0':*s>='a'&&*s<='f'?*s-'a'+10:99;if(d>=base)return 0;n=n*base+d;}return n;}
static long peek(int pid,unsigned address){errno=0;long x=ptrace(PTRACE_PEEKDATA,pid,(void *)address,0);if(errno){perror("peek");peek_error=1;return 0;}return x;}
static int stop_wait(int tid,int ms,int *status){for(int i=0;i<ms/10;i++){int n=waitpid(tid,status,__WALL|WNOHANG);if(n==tid)return WIFSTOPPED(*status);if(n<0)return 0;usleep(10000);}return 0;}
int main(int argc,char **argv){
 if(argc!=3)return 2;int tid=number(argv[1]),status=0,ok=0,have_regs=0,have_fp=0,wrote_break=0,wrote_path=0;unsigned loader=number(argv[2]),path_address=0;struct regs saved,call,after;unsigned char fp[512]={0};const unsigned trap=0x08100300;long original_break=0,path_words[16];
 if(loader<0x70000000u||loader>=0x80000000u)return 2;
 if(ptrace(PTRACE_SEIZE,tid,0,0)){perror("seize");return 2;}
 if(ptrace(PTRACE_INTERRUPT,tid,0,0)||!stop_wait(tid,2000,&status))goto done;
 if(ptrace(12,tid,0,&saved)){perror("getregs");goto done;}have_regs=1;
 if(ptrace(14,tid,0,fp)){perror("getfp");goto done;}have_fp=1;
 unsigned owner=(unsigned)peek(tid,0x83dfe4);if(owner<0x10000)goto done;unsigned handle=(unsigned)peek(tid,owner+6*0x54+0x314);
 if(!handle||peek(tid,owner+0x5c)!=6||peek(tid,handle)!=25||peek(tid,handle+4)!=0x10000000u||peek_error){puts("precondition changed; no call");goto done;}
 char path[80],value[100];snprintf(path,sizeof path,"/proc/%d/fd/25",tid);int n=readlink(path,value,sizeof value-1);if(n<0)goto done;value[n]=0;
 if(strcmp(value,"/dev/snd/pcmC1D0c (deleted)")){puts("descriptor changed; no call");goto done;}
 original_break=peek(tid,trap);if(original_break){puts("scratch page is not empty; no call");goto done;}
 if(ptrace(PTRACE_POKETEXT,tid,(void *)trap,(void *)0x0000000d))goto done;wrote_break=1;
 const char library[64]="/tmp/disc-uac-owner215.so";path_address=((unsigned)saved.r[29]-512)&~7u;
 for(unsigned i=0;i<16;i++)path_words[i]=peek(tid,path_address+i*4);if(peek_error)goto done;
 wrote_path=1;for(unsigned i=0;i<16;i++){unsigned word;memcpy(&word,library+i*4,4);if(ptrace(PTRACE_POKEDATA,tid,(void *)(path_address+i*4),(void *)word))goto done;}
 for(unsigned phase=0;phase<2;phase++){
 call=saved;call.r[0]=0;call.r[2]=0;call.r[4]=phase?0x10000000:path_address;call.r[5]=phase?0:2;call.r[6]=call.r[7]=0;call.r[25]=phase?0x478354:loader;call.r[29]=((unsigned)saved.r[29]-1024)&~7u;call.r[31]=trap;call.epc=call.r[25];
 if(ptrace(13,tid,0,&call)||ptrace(PTRACE_CONT,tid,0,0))goto done;
 if(!stop_wait(tid,3000,&status)){ptrace(PTRACE_INTERRUPT,tid,0,0);if(!stop_wait(tid,1000,&status)){puts("call did not stop; needs attention");return 3;}goto done;}
 if(ptrace(12,tid,0,&after))goto done;
 ok=WSTOPSIG(status)==SIGTRAP&&((unsigned)after.epc==trap||(unsigned)after.epc==trap+4);
 printf("phase=%u returned=%d result=%llx signal=%d pc=%x\n",phase,ok,(unsigned long long)after.r[2],WSTOPSIG(status),(unsigned)after.epc);fflush(stdout);
 if(!ok)goto done;
 if(!phase&&(!after.r[2]||peek(tid,0x461188)!=(long)0x27bdffb8u)){ok=0;puts("hot owner setup failed");goto done;}
 }
 printf("stock input-close completed; old_handle=%x current_handle=%lx\n",handle,peek(tid,owner+6*0x54+0x314));
done:
 if(wrote_path)for(unsigned i=0;i<16;i++)if(ptrace(PTRACE_POKEDATA,tid,(void *)(path_address+i*4),(void *)path_words[i]))perror("restore scratch");
 if(wrote_break&&ptrace(PTRACE_POKETEXT,tid,(void *)trap,(void *)original_break))perror("restore break");
 if(have_fp&&ptrace(15,tid,0,fp))perror("restore fp");
 if(have_regs&&ptrace(13,tid,0,&saved))perror("restore registers");
 if(ptrace(PTRACE_DETACH,tid,0,0))perror("detach");return ok?0:2;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nmove $a0,$sp\nlw $a0,0($a0)\naddiu $a1,$sp,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
