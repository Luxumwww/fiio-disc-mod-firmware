#include "../audio-visuals/xy/visual_api.h"
#include "xy_provider_guard.h"
static void *xy_handle;
static const DiscXyApi *xy_loaded;
static char xy_loaded_path[400];
static void (*bt_set_xy)(const DiscXyApi *);
static void xy_bridge_refresh(void){
 static int traced;
 const DiscXyApi *api=0;char target[240],path[400];int n=readlink("/usr/data/disc-runtime/components/disc-audio-xy/current",target,sizeof(target)-1);
 int enabled=!access("/usr/data/disc-runtime/components/disc-audio-xy/enabled",0)&&access("/usr/data/disc-runtime/components/disc-audio-xy/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
 if(n<=9||n>180||memcmp(target,"releases/",9))enabled=0;
 if(enabled){target[n]=0;for(int i=9;i<n;i++)if(!((target[i]>='0'&&target[i]<='9')||target[i]=='.'))enabled=0;}
 if(enabled){snprintf(path,sizeof(path),"/usr/data/disc-runtime/components/disc-audio-xy/%s/libdisc-audio-xy.so",target);
  if(!xy_handle){int tracing=!traced;if(tracing){traced=1;startup_trace("xy.verify.begin");}unsigned hash=2166136261u,total=0;char b[4096];int r,fd=open(path,0);if(fd<0)return;
   while((r=read(fd,b,sizeof(b)))>0){total+=r;for(int i=0;i<r;i++)hash=(hash^(unsigned char)b[i])*16777619u;}close(fd);
   if(r<0||hash!=XY_PROVIDER_FNV||total!=XY_PROVIDER_SIZE)return;
   if(tracing)startup_trace("xy.verify.ok");
   xy_handle=dlopen(path,2);if(!xy_handle)return;
   if(!module_cache_sync(path)){startup_trace("xy.cache.failed");return;}
   if(tracing)startup_trace("xy.cache.ok");
   const DiscXyApi *(*get)(unsigned)=(const DiscXyApi *(*)(unsigned))dlsym(xy_handle,"disc_xy_get_api");
   if(tracing)startup_trace("xy.dlopen.ok");
   const DiscXyApi *candidate=get?get(DISC_XY_ABI):0;if(candidate&&candidate->abi==DISC_XY_ABI&&candidate->size==sizeof(DiscXyApi)){xy_loaded=candidate;snprintf(xy_loaded_path,sizeof(xy_loaded_path),"%s",path);}
   if(tracing)startup_trace(xy_loaded?"xy.api.ok":"xy.api.unavailable");
  }
  if(xy_loaded&&!strcmp(path,xy_loaded_path))api=xy_loaded;
 }
 if(bt_set_xy)bt_set_xy(api);
 if(api){int fd=open("/tmp/disc-runtime/integrated-disc-audio-xy",1|0x100|0x200,0600);if(fd>=0){write(fd,target,n);write(fd,"\n",1);close(fd);}}else unlink("/tmp/disc-runtime/integrated-disc-audio-xy");
}
