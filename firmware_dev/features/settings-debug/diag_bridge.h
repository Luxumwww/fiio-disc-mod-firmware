/* Optional diagnostics reused from disc-diagnostics; disabled means no hook. */
#include "diag_provider_guard.h"
extern void *dlopen(const char *,int);
static void *diag_handle;
static void (*diag_checkpoint)(unsigned,const void *);
static void diag_start(void){
 if(access("/usr/data/disc-runtime/components/disc-diagnostics/enabled",0)||!access("/usr/data/disc-runtime/components/disc-diagnostics/disabled",0)||!access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0))return;
 const char *path="/usr/data/disc-runtime/components/disc-diagnostics/current/libdisc-crash-recorder.so";
 unsigned hash=2166136261u,total=0;char block[1024];int fd=open(path,0),n;if(fd<0)return;
 while((n=read(fd,block,sizeof block))>0){total+=(unsigned)n;for(int i=0;i<n;i++)hash=(hash^(unsigned char)block[i])*16777619u;}close(fd);
 if(n<0||total!=DIAG_PROVIDER_SIZE||hash!=DIAG_PROVIDER_FNV){startup_trace("diag.verify.invalid");return;}
 diag_handle=dlopen(path,2);if(!diag_handle){startup_trace("diag.load.failed");return;}
 if(!module_cache_sync("/usr/data/disc-runtime/components/disc-diagnostics/releases/0.3/libdisc-crash-recorder.so")){startup_trace("diag.cache.failed");return;}
 int (*install)(const char *)=(int (*)(const char *))dlsym(diag_handle,"disc_diag_install");
 diag_checkpoint=(void (*)(unsigned,const void *))dlsym(diag_handle,"disc_diag_checkpoint");
 if(install&&diag_checkpoint&&install("/tmp/disc-crash")){startup_trace("diag.armed");diag_checkpoint(1,0);}else {diag_checkpoint=0;startup_trace("diag.arm.failed");}
}
