/* One requested tone update through the same shared lock/atomic store model.
 * Refuses stale snapshots, changed switches/order, or changes to other nodes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <dlfcn.h>
#include "../shared_profile.h"
#define ROOT "/usr/data/disc-runtime/module-data/disc-remote-control/"
static int read_all(const char *path,char *b){int fd=open(path,0);if(fd<0)return 0;int n=read(fd,b,8192);close(fd);if(n<=0||n>=8192)return 0;b[n]=0;return n;}
static int run(int argc,char **argv,char **env){
 /* MIPS o32 flags, matching native_shared_ui.h (host headers differ). */
 (void)argc;(void)argv;(void)env;int lock=open(ROOT "controls.lock",2|0x100,0600);if(lock<0||flock(lock,LOCK_EX|LOCK_NB))return 2;
 char before[8193],after[8193];const char *next=getenv("DISC_NEXT"),*expected=getenv("DISC_EXPECTED_FNV"),*selected=getenv("DISC_NODE_ID");
 if(!next||!expected||!selected)return 3;int nb=read_all(ROOT "shared-profile.conf",before),na=read_all(next,after);if(!nb||!na||disc_chain_hash(before)!=(unsigned)strtoul(expected,0,10)){printf("REFUSED snapshot guard: current_bytes=%d next_bytes=%d actual_hash=%u expected_hash=%u\n",nb,na,nb?disc_chain_hash(before):0,(unsigned)strtoul(expected,0,10));return 4;}
 DiscChain a,b;unsigned ma,mb;if(!disc_shared_parse(before,&a,&ma)||!disc_shared_parse(after,&b,&mb)||ma!=mb||a.count!=b.count)return 5;
 unsigned id=(unsigned)strtoul(selected,0,10),count=0;
 for(unsigned i=0;i<a.count;i++){
  DiscChainNode *x=&a.nodes[i],*y=&b.nodes[i];
  if(x->id==id&&x->type==16){if(x->id!=y->id||x->type!=y->type||x->enabled!=y->enabled||memcmp(y->params,disc_warmgloss_default,sizeof y->params))return 6;count++;}
  else if(memcmp(x,y,sizeof *x))return 7;
 }
 if(count!=1)return 8;int fd=open(ROOT "shared-profile.conf.warmgloss-new",1|0x100|0x200,0600);if(fd<0)return 9;
 int ok=write(fd,after,na)==na&&!fsync(fd);if(close(fd))ok=0;if(!ok||rename(ROOT "shared-profile.conf.warmgloss-new",ROOT "shared-profile.conf"))return 10;
 fd=open(ROOT,0);if(fd>=0){fsync(fd);close(fd);}flock(lock,LOCK_UN);close(lock);
 puts("PASS requested warmgloss tone updated; all switches/order/master/other nodes preserved; locked and atomically persisted; no playback or volume operations");return 0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld)(void),void *stack){(void)fn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");return next?next(run,argc,argv,init,fini,rtld,stack):90;}
