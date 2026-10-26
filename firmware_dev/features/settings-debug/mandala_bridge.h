#include "../audio-visuals/mandala/mandala_api.h"
#include "mandala_provider_guard.h"
static void *mandala_handle;
static const DiscMandalaApi *mandala_loaded;
static char mandala_loaded_path[400];
static void (*bt_set_mandala)(const DiscMandalaApi *);
static void mandala_bridge_refresh(void){
 static int traced;
 const DiscMandalaApi *api=0;char target[240],path[400];
 int n=readlink("/usr/data/disc-runtime/components/disc-audio-mandala/current",target,sizeof(target)-1);
 int enabled=!access("/usr/data/disc-runtime/components/disc-audio-mandala/enabled",0)&&access("/usr/data/disc-runtime/components/disc-audio-mandala/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
 if(n<=9||n>180||memcmp(target,"releases/",9))enabled=0;
 if(enabled){target[n]=0;for(int i=9;i<n;i++)if(!((target[i]>='0'&&target[i]<='9')||target[i]=='.'))enabled=0;}
 if(enabled){
  snprintf(path,sizeof(path),"/usr/data/disc-runtime/components/disc-audio-mandala/%s/libdisc-audio-mandala.so",target);
  if(!mandala_handle){
   int tracing=!traced;if(tracing){traced=1;startup_trace("mandala.verify.begin");}
   unsigned hash=2166136261u,total=0;char b[4096];int r,fd=open(path,0);if(fd<0)return;
   while((r=read(fd,b,sizeof(b)))>0){total+=r;for(int i=0;i<r;i++)hash=(hash^(unsigned char)b[i])*16777619u;}close(fd);
   if(r<0||hash!=MANDALA_PROVIDER_FNV||total!=MANDALA_PROVIDER_SIZE)return;
   if(tracing)startup_trace("mandala.verify.ok");
   mandala_handle=dlopen(path,2);if(!mandala_handle)return;
   if(!module_cache_sync(path)){startup_trace("mandala.cache.failed");return;}
   if(tracing)startup_trace("mandala.cache.ok");
   const DiscMandalaApi *(*get)(unsigned)=(const DiscMandalaApi *(*)(unsigned))dlsym(mandala_handle,"disc_mandala_get_api");
   if(!get||!provider_mapped((const void *)get,4,1,path)){startup_trace("mandala.symbol.invalid");return;}
   const DiscMandalaApi *candidate=get(DISC_MANDALA_ABI);
   if(!candidate||!provider_mapped(candidate,sizeof(DiscMandalaApi),0,path)){startup_trace("mandala.table.invalid");return;}
   const unsigned *callbacks=(const unsigned *)((const char *)candidate+2*sizeof(unsigned));
   for(unsigned i=0;i<6;i++)if(!provider_mapped((const void *)callbacks[i],4,1,path)){startup_trace("mandala.callback.invalid");return;}
   if(candidate->abi==DISC_MANDALA_ABI&&candidate->size==sizeof(DiscMandalaApi)){mandala_loaded=candidate;snprintf(mandala_loaded_path,sizeof(mandala_loaded_path),"%s",path);}
   if(tracing)startup_trace(mandala_loaded?"mandala.api.ok":"mandala.api.unavailable");
  }
  if(mandala_loaded&&!strcmp(path,mandala_loaded_path))api=mandala_loaded;
 }
 if(bt_set_mandala)bt_set_mandala(api);
 if(api){int fd=open("/tmp/disc-runtime/integrated-disc-audio-mandala",1|0x100|0x200,0600);if(fd>=0){write(fd,target,n);write(fd,"\n",1);close(fd);}}else unlink("/tmp/disc-runtime/integrated-disc-audio-mandala");
}
