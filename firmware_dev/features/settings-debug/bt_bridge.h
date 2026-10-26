/* Trusted integrated module selected and hash-checked by the existing runtime.
 * dlopen is done in the UI loop; no worker ever calls LVGL. A mapped release
 * remains mapped until UI restart, preventing callbacks into unloaded code. */
extern void *dlopen(const char *,int);
#include "bt_module_guard.h"
static int (*bt_presentation_ready)(void);
#include "hw_bridge.h"
#include "xy_bridge.h"
#include "polar_bridge.h"
#include "spectrum_bridge.h"
#include "mandala_bridge.h"
static void (*bt_set_accel)(const DiscAccelApi *);
static void *bt_handle;
static void (*bt_after)(void),(*bt_prepare)(void),(*bt_poll)(void),(*bt_activate)(int);
static void (*bt_rendered)(void);
static int (*bt_fast)(void);
static unsigned bt_checked;
static char bt_loaded_path[400];
static void bt_bridge_refresh(void){
 static int traced,verify_traced,load_traced;
 char path[400];int f=open("/tmp/disc-runtime/bt-module.path",0),n=-1;
 if(f>=0){n=read(f,path,sizeof(path)-1);close(f);}
 if(n<=0){
  /* Cold boot precedes the delayed runtime. Only the embedded, reviewed
   * module fingerprint may be selected directly before the runtime exists. */
  const char fallback[]="/usr/data/disc-runtime/components/disc-bt-meter/current/libdisc-bt-meter.so";
  if(access("/usr/data/disc-runtime/components/disc-bt-meter/enabled",0)||!access("/usr/data/disc-runtime/components/disc-bt-meter/disabled",0)||!access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0)){if(bt_activate)bt_activate(0);return;}
  if(!bt_handle){if(!verify_traced){verify_traced=1;startup_trace("bt.verify.begin");}unsigned hash=2166136261u,total=0;char buf[4096];int r;f=open(fallback,0);if(f<0)return;
   while((r=read(f,buf,sizeof(buf)))>0){total+=r;for(int k=0;k<r;k++)hash=(hash^(unsigned char)buf[k])*16777619u;}close(f);
   if(r<0||total!=BT_MODULE_SIZE||hash!=BT_MODULE_FNV)return;
   if(verify_traced==1){verify_traced=2;startup_trace("bt.verify.ok");}
  }
  n=readlink("/usr/data/disc-runtime/components/disc-bt-meter/current",path,sizeof(path)-64);
  if(n<=0||n>200||memcmp(path,"releases/",9))return;path[n]=0;
  char full[400];snprintf(full,sizeof(full),"/usr/data/disc-runtime/components/disc-bt-meter/%s/libdisc-bt-meter.so",path);snprintf(path,sizeof(path),"%s",full);n=0;while(path[n])n++;
 }
 path[n]=0;
 while(n>0&&(path[n-1]=='\n'||path[n-1]=='\r'))path[--n]=0;
 const char prefix[]="/usr/data/disc-runtime/components/disc-bt-meter/releases/";
 const char tf[]="/tmp/sdcard/DISC_MODULES/disc-bt-meter/";
 if((n<(int)sizeof(prefix)||memcmp(path,prefix,sizeof(prefix)-1))&&(n<(int)sizeof(tf)||memcmp(path,tf,sizeof(tf)-1)))return;
 if(!bt_handle){
  if(!load_traced){load_traced=1;startup_trace("bt.dlopen.begin");}
  bt_handle=dlopen(path,2);if(!bt_handle)return;
  if(load_traced==1){load_traced=2;startup_trace("bt.dlopen.ok");}
  if(!module_cache_sync(path)){startup_trace("bt.cache.failed");return;}
  startup_trace("bt.cache.ok");
  bt_set_xy=(void (*)(const DiscXyApi *))dlsym(bt_handle,"disc_bt_set_xy");
  bt_set_polar=(void (*)(const DiscPolarApi *))dlsym(bt_handle,"disc_bt_set_polar");
  bt_set_stereo=(void (*)(const DiscStereoApi *))dlsym(bt_handle,"disc_bt_set_stereo");
  bt_set_spectrum=(void (*)(const DiscSpectrumApi *))dlsym(bt_handle,"disc_bt_set_spectrum");
  bt_set_mandala=(void (*)(const DiscMandalaApi *))dlsym(bt_handle,"disc_bt_set_mandala");
  bt_set_accel=(void (*)(const DiscAccelApi *))dlsym(bt_handle,"disc_bt_set_accel");
  bt_after=(void (*)(void))dlsym(bt_handle,"disc_bt_after");
  bt_rendered=(void (*)(void))dlsym(bt_handle,"disc_bt_rendered");
  bt_prepare=(void (*)(void))dlsym(bt_handle,"disc_bt_prepare");
  bt_poll=(void (*)(void))dlsym(bt_handle,"disc_bt_poll");
  bt_activate=(void (*)(int))dlsym(bt_handle,"disc_bt_activate");
  bt_fast=(int (*)(void))dlsym(bt_handle,"disc_bt_fast");bt_presentation_ready=(int (*)(void))dlsym(bt_handle,"disc_bt_presentation_ready");
  if(!bt_prepare||!bt_poll||!bt_activate){bt_prepare=bt_poll=0;bt_activate=0;return;}
  snprintf(bt_loaded_path,sizeof(bt_loaded_path),"%s",path);
 }
 /* A changed release is activated only after controlled UI reload. */
 if(bt_set_accel)bt_set_accel(hw_api);xy_bridge_refresh();polar_bridge_refresh();spectrum_bridge_refresh();mandala_bridge_refresh();
 if(bt_activate)bt_activate(!strcmp(path,bt_loaded_path));
 if(!traced){traced=1;startup_trace("bt.bind.ok");}
}
static void bt_bridge_tick(void){
 xy_bridge_refresh();polar_bridge_refresh();spectrum_bridge_refresh();mandala_bridge_refresh();hw_bridge_refresh();if(bt_set_accel)bt_set_accel(hw_api);
 if(!(bt_checked++%12))bt_bridge_refresh();
}
