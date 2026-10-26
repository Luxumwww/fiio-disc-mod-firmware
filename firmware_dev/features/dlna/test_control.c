/* Temporary protocol test driver. No UI navigation or firmware writes. */
extern int mq_open(const char *,int,...),mq_send(int,const char *,unsigned,unsigned),mq_close(int);
extern unsigned strlen(const char *);
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nlw $a0,0($sp)\naddiu $a1,$sp,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
int main(int argc,char **argv){
 if(argc!=2||strlen(argv[1])!=12)return 2;
 int q=mq_open("/player",1|0x80);if(q<0)return 3;
 int result=mq_send(q,argv[1],12,0);mq_close(q);return result?4:0;
}
