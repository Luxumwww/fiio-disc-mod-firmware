/* Read only, exact two stock CS43131 clients; no bus scan or register data write. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),ioctl(int,unsigned,...),printf(const char *,...);
extern void *dlsym(void *,const char *);
struct message {unsigned short addr,flags,len;unsigned char *buf;};
struct transfer {struct message *msgs;unsigned count;};
static int capture(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;int f=open("/dev/i2c-3",2),result=0;if(f<0)return 2;
 const unsigned registers[]={0x10000,0x10001,0x10002,0x10005,0x10006,0x1000b,0x1000c,0x20000,0x40010,0x40011,0x40012,0x40013,0x40014,0x40015,0x40016,0x40017,0x40018,0x40019,0x50000,0x50001,0x5000a,0x5000b,0x70000,0x70001,0x70002,0x70003,0x70004,0x70005,0x70006,0x90001,0x90002};
 for(unsigned address=0x30;address<=0x32;address+=2){
  for(unsigned i=0;i<sizeof(registers)/sizeof(registers[0]);i++){
   unsigned r=registers[i];unsigned char a[4]={r>>16,r>>8,r,0},v=0;
   struct message msgs[2]={{address,0,4,a},{address,1,1,&v}};struct transfer x={msgs,2};
   int n=ioctl(f,0x707,&x);printf("addr=0x%x register=0x%x value=0x%x result=%d\n",address,r,v,n);if(n!=2)result=3;
  }
 }
 close(f);return result;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 return next?next(capture,argc,argv,init,fini,rtld_fini,stack):90;
}
