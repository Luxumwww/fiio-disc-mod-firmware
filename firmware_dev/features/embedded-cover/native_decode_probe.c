/* Isolated copy of pinned native executable; __libc_start_main runs only this
 * image decoder harness. Stock /lib/librt is selected, no player/UI main, PCM,
 * music selection or volume commands. Uses production hooks unchanged. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),read(int,void *,unsigned),write(int,const void *,unsigned),access(const char *,int),rename(const char *,const char *),unlink(const char *),getpid(void),printf(const char *,...),snprintf(char *,unsigned,const char *,...),memcmp(const void *,const void *,unsigned),strcmp(const char *,const char *),pthread_mutex_lock(void *),pthread_mutex_unlock(void *),mprotect(void *,unsigned,int),munmap(void *,unsigned),cacheflush(void *,int,int);
extern long readlink(const char *,char *,unsigned),strtol(const char *,char **,int),lseek(int,long,int);
extern void *dlopen(const char *,int),*dlsym(void *,const char *),*mmap(void *,unsigned,int,int,int,long),*malloc(unsigned);
extern void free(void *);
static int pc_enabled(void){return 1;}
static int pc_workmode(void){return 1;}
static void (*pc_cover_tick)(void);
#define PC_WORD(a) (*(volatile unsigned *)(unsigned long)(a))
#define PC_FN(a,t) ((t)(unsigned long)(a))
#define DISC_COVER_STATE "/tmp/disc-cover-native-probe.state"
#include "player_binding.h"
static int run(int argc,char **argv,char **env){
 (void)env;if(argc<2)return 2;
 if(argc==2){cover_install();if(!cover_installed){printf("hook failed\n");return 3;}}
 else {cover_key_create=(int (*)(unsigned *,void (*)(void *)))dlsym(0,"pthread_key_create");cover_key_get=(void *(*)(unsigned))dlsym(0,"pthread_getspecific");cover_key_set=(int (*)(unsigned,const void *))dlsym(0,"pthread_setspecific");cover_key_create(&cover_key,0);}
 int fd=open(argv[1],0);if(fd<0)return 4;long size=lseek(fd,0,2);lseek(fd,0,0);
 if(size<=0||(unsigned)size>DISC_COVER_MAX_BYTES){close(fd);return 5;}
 void *data=malloc((unsigned)size);if(!data){close(fd);return 6;}
 unsigned at=0;while(at<(unsigned)size){int n=read(fd,(char *)data+at,(unsigned)size-at);if(n<=0)break;at+=(unsigned)n;}close(fd);if(at!=(unsigned)size){free(data);return 7;}
 DiscCoverJob job={0};job.path=argv[1];job.bytes=(unsigned)size;
 cover_key_set(cover_key,&job);void *out=0;unsigned w=0,h=0;int components=0;
 write(2,"before decode\n",14);
 int result=argc==2?cover_jpeg_load(data,&out,(unsigned)size,&w,&h,&components):PC_FN(0x47e848,int (*)(const void *,void **,unsigned,unsigned *,unsigned *,int *))(data,&out,(unsigned)size,&w,&h,&components);
 write(2,"after decode\n",13);
 cover_key_set(cover_key,0);cover_report(&job);
 printf("result=%d input=%ux%u output=%ux%u components=%d heap=%u rejected=%u data=%u\n",result,job.width,job.height,w,h,components,job.jpeg_heap,job.jpeg_heap_rejected,out!=0);
 if(out)free(out);free(data);return result?8:0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)fn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 return next?next(run,argc,argv,init,fini,rtld_fini,stack):90;
}
