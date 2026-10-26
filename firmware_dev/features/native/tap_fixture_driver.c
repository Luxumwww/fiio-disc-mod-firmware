/* Runs after constructors, using a normal host main or the stock MIPS crt. */
extern int *__errno_location(void);
extern void *pcm_open(unsigned,unsigned,unsigned,const void *);
extern int pcm_close(void *),pcm_read(void *,void *,unsigned);
extern void disc_fixture_fail_read(int);
extern unsigned disc_fixture_reads(const void *);
extern long write(int,const void *,unsigned long);
static unsigned char samples[2048];
static int run_tests(void){
 const unsigned config[]={2,48000,256,4,0};
 void *p=pcm_open(3,0,0x10000000,config);
 if(!p||*__errno_location()!=73)return 1;
 if(pcm_read(p,samples,1024)||*__errno_location()!=76||disc_fixture_reads(p)!=1)return 2;
 for(unsigned i=0;i<1024;i++)if(samples[i]!=(unsigned char)(i*3+17))return 3;
 disc_fixture_fail_read(-5);
 if(pcm_read(p,samples,1024)!=-5||*__errno_location()!=75||disc_fixture_reads(p)!=2)return 4;
 disc_fixture_fail_read(0);
 if(pcm_read(p,samples,512)||disc_fixture_reads(p)!=3)return 8;
 if(pcm_close(p)||*__errno_location()!=74)return 5;
 p=pcm_open(2,0,0x10000000,config); /* A different input card must not be captured. */
 if(pcm_read(p,samples,1024)||pcm_close(p))return 6;
 p=pcm_open(3,0,0,config); /* UAC card, output direction: no capture. */
 if(pcm_read(p,samples,1024)||pcm_close(p))return 7;
 const unsigned formats[]={5,6,7};
 for(unsigned j=0;j<3;j++){
  unsigned c[]={2,96000,256,4,formats[j]};unsigned n=formats[j]==5?1536:2048;
  p=pcm_open(3,0,0x10000000,c);
  if(pcm_read(p,samples,n)||disc_fixture_reads(p)!=1||*__errno_location()!=76)return 9;
  for(unsigned i=0;i<n;i++)if(samples[i]!=(unsigned char)(i*3+17))return 10;
  if(pcm_close(p))return 11;
 }
 const unsigned invalid[][5]={{2,48000,256,4,42},{3,48000,256,4,0},{2,0,256,4,0}};
 for(unsigned j=0;j<3;j++){
  p=pcm_open(3,0,0x10000000,invalid[j]);
  if(pcm_read(p,samples,1024)||pcm_close(p))return 12;
 }
 const char ok[]="DISC_TAP_FIXTURE_OK passthrough=exact errno=preserved errors=preserved direction=filtered formats=4 unsupported=ignored position=preserved\n";
 write(1,ok,sizeof(ok)-1);return 0;
}
#ifdef DISC_HOST_TEST
int main(void){return run_tests();}
#else
extern void *dlsym(void *,const char *);
static int fixture_main(int argc,char **argv,char **env){(void)argc;(void)argv;(void)env;return run_tests();}
int __libc_start_main(int (*main_function)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)main_function;
 int (*start)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 return start(fixture_main,argc,argv,init,fini,rtld_fini,stack);
}
#endif
