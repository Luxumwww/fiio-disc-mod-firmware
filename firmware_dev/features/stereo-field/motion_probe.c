/* Bounded, read-only renderer timing observer. Does not stop/attach the UI. */
#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
extern unsigned long old_strtoul(const char *,char **,int) __asm__("strtoul");
#define strtoul old_strtoul
static unsigned rows[1800][10];
static unsigned ms(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (unsigned)t.tv_sec*1000+(unsigned)t.tv_nsec/1000000;}
int main(int argc,char **argv){
 if(argc==1){printf("%ld\n",sysconf(_SC_CLK_TCK));fflush(stdout);return 0;}
 if(argc!=11)return 2;char path[64];snprintf(path,sizeof(path),"/proc/%u/mem",(unsigned)strtoul(argv[1],0,0));int fd=open(path,O_RDONLY);if(fd<0)return 3;
 unsigned addr[9];for(unsigned j=0;j<9;j++)addr[j]=(unsigned)strtoul(argv[j+2],0,0);
 unsigned used=0,start=ms(),previous=0xffffffff;
 while(ms()-start<8000&&used<1800){unsigned row[10];row[0]=ms()-start;for(unsigned j=0;j<9;j++)if(pread(fd,row+j+1,4,addr[j])!=4)return 4;
  if(row[1]!=previous){for(unsigned j=0;j<10;j++)rows[used][j]=row[j];used++;previous=row[1];}usleep(4000);
 }
 close(fd);for(unsigned i=0;i<used;i++){for(unsigned j=0;j<10;j++)printf(j?",%u":"%u",rows[i][j]);printf("\n");}fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nmove $t0,$sp\nlw $a0,0($t0)\naddiu $a1,$t0,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
