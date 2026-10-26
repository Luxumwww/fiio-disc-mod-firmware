#define _GNU_SOURCE
#include <signal.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
static void existing(int s){(void)s;_exit(42);}
int main(int argc,char **argv){
 if(argc!=2)return 2;void *h=dlopen("/tmp/disc-crash-recorder-test.so",2);if(!h)return 3;
 int (*install)(const char *)=dlsym(h,"disc_diag_install");void (*checkpoint)(unsigned,const void *)=dlsym(h,"disc_diag_checkpoint");
 if(!install||!checkpoint)return 4;
 if(!strcmp(argv[1],"chain")){struct sigaction a={0};a.sa_handler=existing;sigemptyset(&a.sa_mask);if(sigaction(SIGTERM,&a,0))return 5;}
 if(!install("/tmp/disc-crash-test"))return 6;checkpoint(3,install);
 if(!strcmp(argv[1],"segv")){*(volatile unsigned *)0x1000=1;return 7;}
 if(!strcmp(argv[1],"chain")||!strcmp(argv[1],"term")){raise(SIGTERM);return 8;}
 if(!strcmp(argv[1],"abort")){raise(SIGABRT);return 9;}
 return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nmove $t0,$sp\nlw $a0,0($t0)\naddiu $a1,$t0,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
