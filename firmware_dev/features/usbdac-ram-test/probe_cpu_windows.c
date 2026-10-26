/* Bounded quarter-second CPU accounting intervals; no logging in player. */
extern int open(const char *,int,...),read(int,void *,unsigned),close(int),sscanf(const char *,const char *,...),printf(const char *,...),clock_gettime(int,void *),usleep(unsigned);
extern void *dlsym(void *,const char *);
#ifndef CPU_SAMPLES
#define CPU_SAMPLES 17
#endif
struct sample {unsigned long long ns;unsigned v[8];};static struct sample samples[CPU_SAMPLES];
static int probe(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;long first[2],now[2];clock_gettime(1,first);
 for(unsigned i=0;i<CPU_SAMPLES;i++){
  char b[512];int fd=open("/proc/stat",0);if(fd<0)return 1;int n=read(fd,b,sizeof(b)-1);close(fd);if(n<=0)return 2;b[n]=0;
  struct sample *s=&samples[i];if(sscanf(b,"cpu %u %u %u %u %u %u %u %u",s->v,s->v+1,s->v+2,s->v+3,s->v+4,s->v+5,s->v+6,s->v+7)!=8)return 3;
  clock_gettime(1,now);s->ns=(unsigned long long)(now[0]-first[0])*1000000000u+now[1]-first[1];
  if(i+1<CPU_SAMPLES)usleep(250000);
 }
 for(unsigned i=0;i<CPU_SAMPLES;i++){printf("%llu",samples[i].ns);for(unsigned j=0;j<8;j++)printf(",%u",samples[i].v[j]);printf("\n");}
 return 0;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 return next?next(probe,argc,argv,init,fini,rtld_fini,stack):90;
}
