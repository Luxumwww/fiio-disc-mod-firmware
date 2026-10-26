extern int open(const char *,int,...),close(int),pread(int,void *,unsigned,long),write(int,const void *,unsigned);
static unsigned char data[518400];
int main(void){int f=open("/dev/mem",0);if(f<0)return 2;unsigned n=0;while(n<sizeof data){int r=pread(f,data+n,sizeof data-n,0x079fa400+n);if(r<=0)return 3;n+=r;}close(f);int out=open("/tmp/disc-plane-physical.bin",0x301,0600);if(out<0)return 4;n=0;while(n<sizeof data){int r=write(out,data+n,sizeof data-n);if(r<=0)return 5;n+=r;}close(out);return 0;}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
