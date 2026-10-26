/* One-shot profile update only; never opens or controls audio. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include "shared_profile.h"
extern int flock(int,int),fsync(int);
static int readall(const char *path,char *buf){int f=open(path,0);if(f<0)return -1;int n=read(f,buf,8192);close(f);if(n<0||n>=8192)return -1;buf[n]=0;return n;}
static int check(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;const char *src=getenv("DISC_NEXT"),*expect=getenv("DISC_EXPECTED_FNV");if(!src||!expect)return 2;
 const char *root="/usr/data/disc-runtime/module-data/disc-remote-control/";char path[180],tmp[180];snprintf(path,sizeof(path),"%scontrols.lock",root);int lock=open(path,2|0x100,0600);if(lock<0)return 3;if(flock(lock,6)){close(lock);return 4;}
 snprintf(path,sizeof(path),"%sshared-profile.conf",root);snprintf(tmp,sizeof(tmp),"%sshared-profile.conf.center-next",root);char before[8193],after[8193];int nb=readall(path,before),na=readall(src,after),result=5;DiscChain a,b;unsigned on_a,on_b;
 if(nb>0&&na>0&&disc_chain_hash(before)==(unsigned)strtoul(expect,0,10)&&disc_shared_parse(before,&a,&on_a)&&disc_shared_parse(after,&b,&on_b)&&on_a==on_b&&a.count==b.count){
  unsigned changed=0;for(unsigned i=0;i<a.count;i++)if(a.nodes[i].type==13&&a.nodes[i].params[2]!=2){a.nodes[i].params[2]=2;changed++;}
  if(changed&&!memcmp(&a,&b,sizeof(a))){int f=open(tmp,1|0x100|0x200,0600);if(f>=0){int ok=write(f,after,na)==na&&!fsync(f);close(f);if(ok&&!rename(tmp,path)){result=0;printf("Updated %u surround node(s) to centered mode; all other parameters/master/order unchanged\n",changed);}else unlink(tmp);}}
 }
 flock(lock,8);close(lock);return result;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void(*init)(void),void(*fini)(void),void(*rtld)(void),void *stack){(void)fn;int(*next)(int(*)(int,char**,char**),int,char**,void(*)(void),void(*)(void),void(*)(void),void*)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(check,argc,argv,init,fini,rtld,stack):90;}
