#include "provider_guard.h"
static int ram_host_loaded,ram_host_installed;
static int (*ram_host_install)(unsigned,unsigned);
static void (*ram_host_tick)(void);
static void ram_host_poll(void){
 if(!ram_host_installed){
  if(!access("/usr/data/disc-runtime/components/disc-usbdac-ram-test/disabled",0)||!access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0))return;
  if(!ram_host_loaded){
   const char *path="/usr/data/disc-runtime/components/disc-usbdac-ram-test/current/libdisc-ram-dop.so";
   int f=open(path,0),n;unsigned hash=2166136261u,size=0;char b[4096];if(f<0)return;
   while((n=read(f,b,sizeof(b)))>0){for(int i=0;i<n;i++)hash=(hash^(unsigned char)b[i])*16777619u;size+=(unsigned)n;}close(f);
   if(n<0||size!=RAM_DOP_SIZE||hash!=RAM_DOP_FNV)return;
   void *h=dlopen(path,2);if(!h)return;ram_host_loaded=1;ram_host_install=dlsym(h,"disc_ram_dop_install");ram_host_tick=dlsym(h,"disc_ram_dop_tick");
  }
  if(ram_host_install&&ram_host_tick)ram_host_installed=ram_host_install(PLAYER_PCM_WRITE_GOT,PLAYER_IOCTL_GOT);
 }
 if(ram_host_installed)ram_host_tick();
}
