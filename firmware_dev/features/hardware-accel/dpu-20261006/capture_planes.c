extern int open(const char *,int,...),close(int),ioctl(int,unsigned,...),write(int,const void *,unsigned);
extern void *mmap(void *,unsigned,int,int,int,long);extern int munmap(void *,unsigned);
static unsigned v[40],fix[32];
int main(void){int f=open("/dev/fb0",0);if(f<0||ioctl(f,0x4600,v))return 2;unsigned offset=v[5]*1440;close(f);for(int i=0;i<2;i++){char path[]="/dev/fb0";path[7]='0'+i;f=open(path,0);if(f<0||ioctl(f,0x4602,fix)||fix[5]!=1555200||offset>1036800)return 3;unsigned bias=fix[4]&4095;void *p=mmap(0,1555200+bias,1,1,f,0);if(p==(void*)-1)return 4;int out=open(i?"/tmp/disc-plane-capture.bin":"/tmp/disc-base-capture.bin",0x301,0600);if(out<0)return 5;unsigned n=0;while(n<518400){int r=write(out,(char*)p+bias+offset+n,518400-n);if(r<=0)return 6;n+=r;}close(out);munmap(p,1555200+bias);close(f);}return 0;}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");

