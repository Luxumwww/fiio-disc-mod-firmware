/* One reviewed provider mapped for the lifetime of the original UI. */
#include "../hardware-accel/runtime-provider/accel_api.h"
#include "hw_provider_guard.h"
#include "provider_mapping.h"
static unsigned milliseconds(void);
static void *hw_handle;
static const DiscAccelApi *hw_api;
static unsigned hw_state_at,hw_pump_at;
static char hw_path[400];
static int hw_active;
static int (*hw_whitepoint_set)(unsigned);
static int (*hw_whitepoint_get)(void);
static void hw_bridge_refresh(void){
 static int traced;
 char target[240],path[400];int n=readlink("/usr/data/disc-runtime/components/disc-hwaccel/current",target,sizeof(target)-1);
 int enabled=!access("/usr/data/disc-runtime/components/disc-hwaccel/enabled",0)&&access("/usr/data/disc-runtime/components/disc-hwaccel/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
 if(n<=9||n>180||memcmp(target,"releases/",9))enabled=0;
 if(enabled){target[n]=0;for(int i=9;i<n;i++)if(!((target[i]>='0'&&target[i]<='9')||target[i]=='.'))enabled=0;}
 if(enabled){
  snprintf(path,sizeof(path),"/usr/data/disc-runtime/components/disc-hwaccel/%s/libdisc-hwaccel.so",target);
  if(!hw_handle){
   int tracing=!traced;if(tracing){traced=1;startup_trace("hw.verify.begin");}
   unsigned hash=2166136261u,total=0;char buffer[4096];int r,fd=open(path,0);
   if(fd<0)return;while((r=read(fd,buffer,sizeof(buffer)))>0){total+=r;for(int i=0;i<r;i++)hash=(hash^(unsigned char)buffer[i])*16777619u;}close(fd);
   if(r<0||hash!=HW_PROVIDER_FNV||total!=HW_PROVIDER_SIZE)return;
   if(tracing)startup_trace("hw.verify.ok");
   hw_handle=dlopen(path,2);if(!hw_handle)return;
   if(tracing)startup_trace("hw.dlopen.ok");
   if(!module_cache_sync(path)){startup_trace("hw.cache.failed");return;}
   if(tracing)startup_trace("hw.cache.ok");
   if(tracing)startup_trace("hw.symbol.begin");
   const DiscAccelApi *(*get_api)(unsigned)=(const DiscAccelApi *(*)(unsigned))dlsym(hw_handle,"disc_accel_get_api");
   if(!get_api||!provider_mapped((const void *)get_api,4,1,path)){startup_trace("hw.symbol.invalid");return;}
   if(tracing)startup_trace("hw.symbol.ok");
   if(tracing)startup_trace("hw.api.begin");
   if(diag_checkpoint)diag_checkpoint(2,(const void *)get_api);
   const DiscAccelApi *candidate=get_api?get_api(DISC_ACCEL_ABI):0;
   if(diag_checkpoint)diag_checkpoint(3,(const void *)get_api);
   if(tracing)startup_trace("hw.api.return");
   if(!candidate||!provider_mapped(candidate,sizeof(DiscAccelApi),0,path)){startup_trace("hw.table.invalid");return;}
   /* No callback is invoked until every table entry belongs to the reviewed
    * provider's executable mappings; malformed optional providers fail closed. */
   const unsigned *entries=(const unsigned *)((const char *)candidate+7*sizeof(unsigned));
   for(unsigned i=0;i<8;i++)if(!provider_mapped((const void *)entries[i],4,1,path)){startup_trace("hw.callback.invalid");return;}
   if(candidate&&candidate->abi==DISC_ACCEL_ABI&&candidate->size==sizeof(DiscAccelApi)&&candidate->capabilities==DISC_ACCEL_CAP_DPU&&candidate->width==360&&candidate->height==360&&candidate->stride==1440&&candidate->rotation==180){
    hw_api=candidate;snprintf(hw_path,sizeof(hw_path),"%s",path);
    int (*wp_set)(unsigned)=(int (*)(unsigned))dlsym(hw_handle,"disc_accel_whitepoint_set");
    int (*wp_get)(void)=(int (*)(void))dlsym(hw_handle,"disc_accel_whitepoint_get");
    if(wp_set&&wp_get&&provider_mapped((const void *)wp_set,4,1,path)&&provider_mapped((const void *)wp_get,4,1,path)){hw_whitepoint_set=wp_set;hw_whitepoint_get=wp_get;}
   }
   if(tracing)startup_trace(hw_api?"hw.api.ok":"hw.api.unavailable");
  }
  enabled=hw_api&&!strcmp(path,hw_path);
 }
 if(hw_api)hw_api->set_enabled(enabled);
 if(enabled){int fd=open("/tmp/disc-runtime/integrated-disc-hwaccel",1|0x100|0x200,0600);if(fd>=0){write(fd,target,n);write(fd,"\n",1);close(fd);}}
 else unlink("/tmp/disc-runtime/integrated-disc-hwaccel");
}
static void hw_bridge_pump(void){
 if(!hw_api)return;DiscAccelStats s;hw_api->stats(0,0,&s);
 /* The page owner submits through after()/rendered(), including its native
  * defer/drop handoff. Pumping it here repeats that submission and can wait
  * for a second vsync in the same UI callback. Other owners retain the
  * provider's periodic pump; static whitepoint is checked by its UI poll. */
 if(strcmp(s.owner,"disc-bt-meter/page"))hw_api->pump();
 hw_api->stats(0,0,&s);hw_active=s.active;
 unsigned t=milliseconds();if(t-hw_state_at>=1000){hw_state_at=t;char b[400];int n=snprintf(b,sizeof(b),"version=1.0.7\ncapability=DPU\nenabled=%u\nactive=%u\nfailed=%u\nowner=%s\nlease=%u\nwhitepoint=%d\n",s.enabled,s.active,s.failed,s.owner,s.lease,hw_whitepoint_get?hw_whitepoint_get():-1);int fd=open("/tmp/disc-runtime/hwaccel.state",1|0x100|0x200,0600);if(fd>=0){write(fd,b,n);close(fd);}}
}

const DiscAccelApi *disc_hwaccel_get_api(unsigned abi){return abi==DISC_ACCEL_ABI?hw_api:0;}
