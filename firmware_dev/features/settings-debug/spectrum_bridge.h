#include "../audio-visuals/spectrum/spectrum_api.h"
#include "spectrum_provider_guard.h"
static void *spectrum_handle;
static const DiscSpectrumApi *spectrum_loaded;
static char spectrum_loaded_path[400];
static void (*bt_set_spectrum)(const DiscSpectrumApi *);
static void spectrum_bridge_refresh(void){
 static int traced;
 const DiscSpectrumApi *api=0;char target[240],path[400];
 int n=readlink("/usr/data/disc-runtime/components/disc-audio-spectrum/current",target,sizeof(target)-1);
 int enabled=!access("/usr/data/disc-runtime/components/disc-audio-spectrum/enabled",0)&&access("/usr/data/disc-runtime/components/disc-audio-spectrum/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
 if(n<=9||n>180||memcmp(target,"releases/",9))enabled=0;
 if(enabled){target[n]=0;for(int i=9;i<n;i++)if(!((target[i]>='0'&&target[i]<='9')||target[i]=='.'))enabled=0;}
 if(enabled){
  snprintf(path,sizeof(path),"/usr/data/disc-runtime/components/disc-audio-spectrum/%s/libdisc-audio-spectrum.so",target);
  if(!spectrum_handle){
   int tracing=!traced;if(tracing){traced=1;startup_trace("spectrum.verify.begin");}
   unsigned hash=2166136261u,total=0;char b[4096];int r,fd=open(path,0);if(fd<0)return;
   while((r=read(fd,b,sizeof(b)))>0){total+=r;for(int i=0;i<r;i++)hash=(hash^(unsigned char)b[i])*16777619u;}close(fd);
   if(r<0||hash!=SPECTRUM_PROVIDER_FNV||total!=SPECTRUM_PROVIDER_SIZE)return;
   if(tracing)startup_trace("spectrum.verify.ok");
   spectrum_handle=dlopen(path,2);if(!spectrum_handle)return;
   if(!module_cache_sync(path)){startup_trace("spectrum.cache.failed");return;}
   if(tracing)startup_trace("spectrum.cache.ok");
   const DiscSpectrumApi *(*get)(unsigned)=(const DiscSpectrumApi *(*)(unsigned))dlsym(spectrum_handle,"disc_spectrum_get_api");
   if(!get||!provider_mapped((const void *)get,4,1,path)){startup_trace("spectrum.symbol.invalid");return;}
   const DiscSpectrumApi *candidate=get(DISC_SPECTRUM_ABI);
   if(!candidate||!provider_mapped(candidate,sizeof(DiscSpectrumApi),0,path)){startup_trace("spectrum.table.invalid");return;}
   const unsigned *callbacks=(const unsigned *)((const char *)candidate+2*sizeof(unsigned));
   for(unsigned i=0;i<6;i++)if(!provider_mapped((const void *)callbacks[i],4,1,path)){startup_trace("spectrum.callback.invalid");return;}
   if(candidate->abi==DISC_SPECTRUM_ABI&&candidate->size==sizeof(DiscSpectrumApi)){spectrum_loaded=candidate;snprintf(spectrum_loaded_path,sizeof(spectrum_loaded_path),"%s",path);}
   if(tracing)startup_trace(spectrum_loaded?"spectrum.api.ok":"spectrum.api.unavailable");
  }
  if(spectrum_loaded&&!strcmp(path,spectrum_loaded_path))api=spectrum_loaded;
 }
 if(bt_set_spectrum)bt_set_spectrum(api);
 if(api){int fd=open("/tmp/disc-runtime/integrated-disc-audio-spectrum",1|0x100|0x200,0600);if(fd>=0){write(fd,target,n);write(fd,"\n",1);close(fd);}}else unlink("/tmp/disc-runtime/integrated-disc-audio-spectrum");
}
