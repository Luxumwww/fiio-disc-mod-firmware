/* Narrow, on-demand diagnostic. Interrupt status registers are read-to-clear.
 * No DAC configuration writes, audio read, bus scan, or persistent logger. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),ioctl(int,unsigned,...),printf(const char *,...);
extern void *dlsym(void *,const char *);
struct message {unsigned short addr,flags,len;unsigned char *buf;};
struct transfer {struct message *msgs;unsigned count;};
static int capture(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;int f=open("/dev/i2c-3",2),result=0;if(f<0)return 2;
 const unsigned registers[]={0x10000,0x10001,0x10002,0xb0000,0x20000,0x10006,0x10010,0x80032,0x80046,0x90003,0xf0010};
 for(unsigned address=0x30;address<=0x32;address+=2){
  unsigned char id[3]={0};
  for(unsigned i=0;i<sizeof(registers)/sizeof(registers[0]);i++){
   unsigned r=registers[i];unsigned char a[4]={r>>16,r>>8,r,0},v=0;
   struct message msgs[2]={{address,0,4,a},{address,1,1,&v}};struct transfer x={msgs,2};
   int n=ioctl(f,0x707,&x);printf("addr=0x%x register=0x%x value=0x%x result=%d\n",address,r,v,n);
   if(n!=2){result=3;break;}
   if(i<3){id[i]=v;if(i==2&&(id[0]!=0x43||id[1]!=0x13||id[2]!=0x10)){result=4;break;}}
  }
 }
 close(f);return result;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 return next?next(capture,argc,argv,init,fini,rtld_fini,stack):90;
}
