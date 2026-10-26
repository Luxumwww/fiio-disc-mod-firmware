#include "../audio-visuals/polar/polar_api.h"
#include "polar_provider_guard.h"
static void *polar_handle;
static const DiscPolarApi *polar_loaded;
static const DiscStereoApi *stereo_loaded;
static char polar_loaded_path[400];
static void (*bt_set_polar)(const DiscPolarApi *);
static void (*bt_set_stereo)(const DiscStereoApi *);
static void polar_bridge_refresh(void){
 static int traced;
 const DiscPolarApi *api=0;const DiscStereoApi *stereo=0;char target[240],path[400];
 int n=readlink("/usr/data/disc-runtime/components/disc-audio-polar/current",target,sizeof(target)-1);
 int enabled=!access("/usr/data/disc-runtime/components/disc-audio-polar/enabled",0)&&access("/usr/data/disc-runtime/components/disc-audio-polar/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
 if(n<=9||n>180||memcmp(target,"releases/",9))enabled=0;
 if(enabled){target[n]=0;for(int i=9;i<n;i++)if(!((target[i]>='0'&&target[i]<='9')||target[i]=='.'))enabled=0;}
 if(enabled){
  snprintf(path,sizeof(path),"/usr/data/disc-runtime/components/disc-audio-polar/%s/libdisc-audio-polar.so",target);
  if(!polar_handle){
   int tracing=!traced;if(tracing){traced=1;startup_trace("polar.verify.begin");}
   unsigned hash=2166136261u,total=0;char b[4096];int r,fd=open(path,0);if(fd<0)return;
   while((r=read(fd,b,sizeof(b)))>0){total+=r;for(int i=0;i<r;i++)hash=(hash^(unsigned char)b[i])*16777619u;}close(fd);
   if(r<0||hash!=POLAR_PROVIDER_FNV||total!=POLAR_PROVIDER_SIZE)return;
   if(tracing)startup_trace("polar.verify.ok");
   polar_handle=dlopen(path,2);if(!polar_handle)return;
   if(!module_cache_sync(path)){startup_trace("polar.cache.failed");return;}
   if(tracing)startup_trace("polar.cache.ok");
   const DiscPolarApi *(*get)(unsigned)=(const DiscPolarApi *(*)(unsigned))dlsym(polar_handle,"disc_polar_get_api");
   if(!get||!provider_mapped((const void *)get,4,1,path)){startup_trace("polar.symbol.invalid");return;}
   const DiscPolarApi *candidate=get(DISC_POLAR_ABI);
   if(!candidate||!provider_mapped(candidate,sizeof(DiscPolarApi),0,path)){startup_trace("polar.table.invalid");return;}
   const unsigned *callbacks=(const unsigned *)((const char *)candidate+2*sizeof(unsigned));
   for(unsigned i=0;i<6;i++)if(!provider_mapped((const void *)callbacks[i],4,1,path)){startup_trace("polar.callback.invalid");return;}
   if(candidate->abi==DISC_POLAR_ABI&&candidate->size==sizeof(DiscPolarApi)){polar_loaded=candidate;snprintf(polar_loaded_path,sizeof(polar_loaded_path),"%s",path);}
   const DiscStereoApi *(*get_stereo)(unsigned)=(const DiscStereoApi *(*)(unsigned))dlsym(polar_handle,"disc_stereo_get_api");
   if(get_stereo&&provider_mapped((const void *)get_stereo,4,1,path)){
    const DiscStereoApi *field=get_stereo(DISC_STEREO_ABI);
    if(field&&provider_mapped(field,sizeof(DiscStereoApi),0,path)&&field->abi==DISC_STEREO_ABI&&field->size==sizeof(DiscStereoApi)){
     const unsigned *functions=(const unsigned *)((const char *)field+2*sizeof(unsigned));unsigned valid=1;
     for(unsigned i=0;i<6;i++)if(!provider_mapped((const void *)functions[i],4,1,path))valid=0;
     if(valid)stereo_loaded=field;
    }
   }
   if(tracing)startup_trace(polar_loaded?"polar.api.ok":"polar.api.unavailable");
  }
  if(polar_loaded&&!strcmp(path,polar_loaded_path)){api=polar_loaded;stereo=stereo_loaded;}
 }
 if(bt_set_polar)bt_set_polar(api);
 if(bt_set_stereo)bt_set_stereo(stereo);
 if(api){int fd=open("/tmp/disc-runtime/integrated-disc-audio-polar",1|0x100|0x200,0600);if(fd>=0){write(fd,target,n);write(fd,"\n",1);close(fd);}}else unlink("/tmp/disc-runtime/integrated-disc-audio-polar");
}
