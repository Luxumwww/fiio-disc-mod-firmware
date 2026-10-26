/* Thin pinned host binding; independent backend implements the audio path. */
#include "provider_guard.h"
extern void *dlopen(const char *,int);
static int dsb_host_attempted,dsb_host_installed;
static void (*dsb_host_tick)(void);
static int (*dsb_host_enable)(unsigned);
static void dsb_host_install(void){
 if(dsb_host_installed)return;
 if(dsb_host_attempted){if(dsb_host_enable)dsb_host_installed=dsb_host_enable(1);return;}
 if(!access("/usr/data/disc-runtime/components/disc-usbdac-dsd-backend/disabled",0)||!access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0))return;
 const char *path="/usr/data/disc-runtime/components/disc-usbdac-dsd-backend/current/libdisc-usbdac-dsd-backend.so";
 int f=open(path,0);if(f<0)return;unsigned hash=2166136261,size=0;char b[4096];int n;
 while((n=read(f,b,sizeof(b)))>0){for(int i=0;i<n;i++)hash=(hash^(unsigned char)b[i])*16777619u;size+=n;}close(f);
 if(n<0||size!=DSB_SIZE||hash!=DSB_FNV)return;dsb_host_attempted=1;
 void *h=dlopen(path,2);if(!h)return;dsb_host_enable=dlsym(h,"disc_dsd_backend_install");dsb_host_tick=dlsym(h,"disc_dsd_backend_tick");
 if(dsb_host_enable&&dsb_host_tick)dsb_host_installed=dsb_host_enable(1);
}
static void dsb_host_poll(void){dsb_host_install();if(dsb_host_installed&&dsb_host_tick)dsb_host_tick();}
