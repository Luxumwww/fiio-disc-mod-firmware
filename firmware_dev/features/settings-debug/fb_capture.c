/* One-shot read-only capture through the stock loader/CRT. No display writes. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),ioctl(int,unsigned,...);
extern long write(int,const void *,size_t);
extern void *mmap(void *,size_t,int,int,int,long),*dlsym(void *,const char *);
extern int munmap(void *,size_t);
static int capture(int argc,char **argv,char **env) {
 (void)argc;(void)argv;(void)env;
 unsigned v[40],after[40],fix[17];int f=open("/dev/fb0",0),result=1;
 if(f<0)return 1;
 if(ioctl(f,0x4600,v)||ioctl(f,0x4602,fix))goto done;
 unsigned length=fix[5],stride=fix[11];
 if(v[0]!=360||v[1]!=360||v[6]!=32||stride!=1440||length>4*1024*1024||length<518400)goto done;
 if(v[8]!=16||v[9]!=8||v[11]!=8||v[12]!=8||v[14]!=0||v[15]!=8)goto done;
 void *map=mmap(0,length,1,1,f,0);if(map==(void *)-1)goto done;
 for(int attempt=0;attempt<3;attempt++){
  if(ioctl(f,0x4600,v))break;
  unsigned offset=v[5]*stride+v[4]*4;
  if(v[4]||v[5]>v[3]||offset>length||518400>length-offset)break;
  int out=open("/tmp/disc-settings-fb.bin",0x301,0600);if(out<0)break;
  unsigned wrote=0;
  while(wrote<518400){long n=write(out,(char *)map+offset+wrote,518400-wrote);if(n<=0)break;wrote+=n;}
  close(out);
  if(wrote==518400&&!ioctl(f,0x4600,after)&&after[4]==v[4]&&after[5]==v[5]){result=0;break;}
 }
 munmap(map,length);
done:close(f);return result;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,
 void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack) {
 (void)mainfn;
 int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 if(!next)return 90;
 return next(capture,argc,argv,init,fini,rtld_fini,stack);
}
