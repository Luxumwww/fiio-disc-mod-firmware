/* Eight-second read-only observer. No ptrace, signals, device control or UI writes. */
#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
extern unsigned long old_strtoul(const char *,char **,int) __asm__("strtoul");
#define strtoul old_strtoul
static unsigned rows[2200][10];
static unsigned us(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (unsigned)t.tv_sec*1000000+(unsigned)t.tv_nsec/1000;}
int main(int argc,char **argv){
 if(argc==1){printf("%ld\n",sysconf(_SC_CLK_TCK));fflush(stdout);return 0;}
 if(argc!=11)return 2;
 unsigned pid=(unsigned)strtoul(argv[1],0,0),addr[9];if(pid<2)return 2;
 for(unsigned j=0;j<9;j++){addr[j]=(unsigned)strtoul(argv[j+2],0,0);if(addr[j]<0x10000||addr[j]>=0x80000000u||(addr[j]&3))return 2;}
 char path[64];snprintf(path,sizeof path,"/proc/%u/mem",pid);int fd=open(path,O_RDONLY);if(fd<0)return 3;
 unsigned used=0,start=us(),previous[3]={0,0,0},row[10];
 do{
  row[0]=us()-start;
  for(unsigned j=0;j<9;j++)if(pread(fd,row+j+1,4,addr[j])!=4){close(fd);return 4;}
  if(!used||row[1]!=previous[0]||row[2]!=previous[1]||row[4]!=previous[2]){
   if(used>=2199){close(fd);return 5;}
   for(unsigned j=0;j<10;j++)rows[used][j]=row[j];used++;
   previous[0]=row[1];previous[1]=row[2];previous[2]=row[4];
  }
  usleep(4000);
 }while(us()-start<8000000u);
 row[0]=us()-start;
 for(unsigned j=0;j<9;j++)if(pread(fd,row+j+1,4,addr[j])!=4){close(fd);return 4;}
 for(unsigned j=0;j<10;j++)rows[used][j]=row[j];used++;close(fd);
 for(unsigned i=0;i<used;i++){for(unsigned j=0;j<10;j++)printf(j?",%u":"%u",rows[i][j]);putchar('\n');}
 fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nmove $t0,$sp\nlw $a0,0($t0)\naddiu $a1,$t0,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
