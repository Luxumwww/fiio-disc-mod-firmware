/* Bounded test gesture, timed on the device to avoid Wi-Fi-long presses. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
typedef struct {int seconds,microseconds;unsigned short type,code;int value;} TouchEvent;
extern int stock_atoi(const char *) __asm__("atoi");
#define atoi stock_atoi
static int fd;
static void event(unsigned type,unsigned code,int value){TouchEvent e={0,0,type,code,value};if(write(fd,&e,sizeof(e))!=sizeof(e))_exit(2);}
static void move(int x,int y){event(3,0x35,359-x);event(3,0x36,359-y);event(3,0,359-x);event(3,1,359-y);event(0,0,0);}
int main(int argc,char **argv){
 if(argc!=4&&argc!=6&&argc!=7)return 2;
 int x=atoi(argv[2]),y=atoi(argv[3]),xx=argc>=6?atoi(argv[4]):x,yy=argc>=6?atoi(argv[5]):y;
 if(x<0||x>359||y<0||y>359||xx<0||xx>359||yy<0||yy>359)return 2;
 int drag=!strcmp(argv[1],"drag"),partial=!strcmp(argv[1],"partial");if(!drag&&!partial&&strcmp(argv[1],"tap"))return 2;
 fd=open("/dev/input/event1",O_WRONLY);if(fd<0)return 2;
 event(3,0x2f,0);event(3,0x39,0);move(x,y);event(1,0x14a,1);event(0,0,0);usleep(80000);
 if(drag||partial)for(int i=1;i<=10;i++){move(x+(xx-x)*i/10,y+(yy-y)*i/10);usleep(16000);}
 if(partial)usleep(700000);
 event(3,0x39,-1);event(1,0x14a,0);event(0,0,0);close(fd);puts("gesture complete");fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nmove $a0,$sp\nlw $a0,0($a0)\naddiu $a1,$sp,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
