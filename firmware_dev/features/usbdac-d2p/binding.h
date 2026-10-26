/* Pinned V2.57 / kernel236. The original digital consumer stays in place.
 * Transform only its final classified USB DoP output; leave every PCM,
 * local DSD, Bluetooth and external-USB output call unchanged. */
#include "convert.h"
static int d2p_enabled,d2p_installed,d2p_attempted;
static volatile unsigned d2p_active,d2p_ready,d2p_generation;
static unsigned d2p_checked,d2p_report_at,d2p_carrier,d2p_saved_rate,d2p_filter_generation;
static unsigned d2p_blocks,d2p_frames,d2p_bad,d2p_silence,d2p_reopens,d2p_failures,d2p_write_errors;
static int d2p_mutex[16];
static DiscD2P d2p_filter;
static int d2p_buffer[16384];
static int (*d2p_original_configure)(unsigned,unsigned,unsigned);
static void d2p_gate(void){
 unsigned now=pc_seconds();if(d2p_checked==now+1)return;d2p_checked=now+1;
 char b[32];int f=open("/usr/data/disc-runtime/components/disc-usbdac-d2p/current/FEATURES",0),n=f<0?0:read(f,b,sizeof(b)-1);if(f>=0)close(f);if(n<0)n=0;b[n]=0;
 d2p_enabled=!strcmp(b,"usbdac-d2p-abi1\n")&&access("/usr/data/disc-runtime/components/disc-usbdac-d2p/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
}
static int d2p_source(void){
 unsigned p=PC_WORD(0x83dfc4);return p&&pc_workmode()==4&&PC_BYTE(0x83a75a)==6&&pc_route()==6&&PC_WORD(0x839a74)==1&&
 (PC_WORD(p+0xc)==176400||PC_WORD(p+0xc)==352800)&&PC_WORD(p+0x10)==2&&PC_WORD(p+0x14)==32&&PC_WORD(p+0x18)==0&&PC_WORD(p+0x2c)==2&&PC_WORD(p+0x58)==1;
}
/* Must be called with d2p_mutex held. Native close/write use their original
 * shared output mutex. Capture/UAC USB handle is never closed here. */
static int d2p_open_pcm(unsigned p,unsigned rate){
 PC_FN(0x4e8b94,void (*)(unsigned))(1);
 PC_FN(0x478354,void (*)(unsigned))(0);
 int rc=d2p_original_configure(rate,32,0);
 if(!rc)rc=PC_FN(0x47859c,int (*)(unsigned,unsigned,unsigned,unsigned,unsigned))(rate,32,2,0,0);
 if(!rc&&pc_pcm_open(6)){PC_WORD(p+0x1c)=rate;PC_WORD(p+0x24)=32;PC_WORD(p+0x28)=0;d2p_reopens++;PC_FN(0x4e8b94,void (*)(unsigned))(0);return 0;}
 d2p_failures++;return -1; /* Stay muted; never feed DoP to a PCM DAC. */
}
static int d2p_configure(unsigned rate,unsigned bits,unsigned is_dsd){
 unsigned p=PC_WORD(0x83dfc4);
 if(d2p_enabled&&bits==32&&is_dsd==1&&d2p_source()){
  pthread_mutex_lock(d2p_mutex);d2p_ready=0;
  if(!d2p_active)d2p_saved_rate=PC_WORD(p+0x1c);
  d2p_active=1;d2p_carrier=PC_WORD(p+0xc);
  int rc=d2p_open_pcm(p,88200);d2p_generation++;__sync_synchronize();d2p_ready=!rc;
  pthread_mutex_unlock(d2p_mutex);return rc;
 }
 if(d2p_active){
  pthread_mutex_lock(d2p_mutex);d2p_active=d2p_ready=0;d2p_generation++;
  if(!is_dsd&&pc_workmode()==4&&pc_route()==6&&p&&PC_WORD(p+0x1c)==88200){
   unsigned restore=PC_WORD(p+0xc);if(restore<8000||restore>384000)restore=d2p_saved_rate;
   int rc=d2p_open_pcm(p,restore);pthread_mutex_unlock(d2p_mutex);return rc;
  }
  pthread_mutex_unlock(d2p_mutex);
 }
 return d2p_original_configure(rate,bits,is_dsd);
}
static int d2p_output(void *data,int bytes,int quiet){
 if(!d2p_source())return PC_FN(0x465718,int (*)(void *,int,int))(data,bytes,quiet);
 if(!d2p_enabled&&!d2p_active)return PC_FN(0x465718,int (*)(void *,int,int))(data,bytes,quiet);
 if(bytes<=0||bytes>0x12c000){pc_real_usleep(1000);return 0;}
 pthread_mutex_lock(d2p_mutex);
 if(!d2p_ready||!d2p_enabled){pthread_mutex_unlock(d2p_mutex);pc_real_usleep(5000);return 0;}
 if(d2p_filter_generation!=d2p_generation){d2p_reset(&d2p_filter,d2p_carrier);d2p_filter_generation=d2p_generation;}
 unsigned consumed=0;
 while(consumed<(unsigned)bytes){
  unsigned chunk=(unsigned)bytes-consumed;if(chunk>131072)chunk=131072;chunk&=~7u;
  if(!chunk){d2p_bad++;break;}
  unsigned frames;int n=0;
  if(quiet){frames=chunk/(d2p_carrier==176400?16:32);d2p_silence++;d2p_reset(&d2p_filter,d2p_carrier);}
  else{
   n=d2p_convert(&d2p_filter,(const unsigned char *)data+consumed,chunk,d2p_buffer,8192);
   if(n<0){frames=chunk/(d2p_carrier==176400?16:32);d2p_bad++;d2p_reset(&d2p_filter,d2p_carrier);}
   else frames=(unsigned)n;
  }
  if(quiet||n<0)for(unsigned i=0;i<frames*2;i++)d2p_buffer[i]=0;
  if(frames){
   /* Existing EQ now receives real PCM, never DoP markers/payload. Hardware
    * analogue volume retains the original key/remote controls. */
   if(!quiet&&n>=0)PC_FN(0x45c648,int (*)(void *,int,int))(d2p_buffer,frames*8,88200);
   int result=PC_FN(0x47824c,int (*)(void *,unsigned))(d2p_buffer,frames*8);
   if(result)d2p_write_errors++;d2p_frames+=frames;
  }
  d2p_blocks++;consumed+=chunk;
 }
 pthread_mutex_unlock(d2p_mutex);return 0;
}
#ifndef DISC_D2P_TEST
static int d2p_patch(void){
 if(d2p_installed)return 1;if(d2p_attempted)return 0;d2p_attempted=1;
 const unsigned sites[7]={0x465970,0x465af8,0x465d64,0x465ec0,0x466338,0x466420,0x466980};
 const unsigned delay[7]={0x34058000,0x02202025,0x34058000,0x00402825,0xafa20040,0x34058000,0x00402825};
 for(unsigned i=0;i<7;i++)if(PC_WORD(sites[i])!=0x0c1195c6||PC_WORD(sites[i]+4)!=delay[i])return 0;
 unsigned *stub=mmap((void *)0x07100000,4096,3,0x802,-1,0);
 unsigned address=(unsigned)(unsigned long)stub,target=(unsigned)(unsigned long)d2p_output;
 if(address<0x10000||address>=0x10000000u||(address&4095)){if(stub!=(void *)-1)munmap(stub,4096);return 0;}
 stub[0]=0x3c190000|(target>>16);stub[1]=0x37390000|(target&65535);stub[2]=0x03200008;stub[3]=0;
 if(cacheflush(stub,16,3)||mprotect(stub,4096,5)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x465000,8192,7)){munmap(stub,4096);return 0;}
 for(unsigned i=0;i<7;i++)PC_WORD(sites[i])=0x0c000000|(address>>2);
 __sync_synchronize();int rc=cacheflush((void *)0x465000,8192,3);
 if(rc){for(unsigned i=0;i<7;i++)PC_WORD(sites[i])=0x0c1195c6;cacheflush((void *)0x465000,8192,3);}
 int protection=mprotect((void *)0x465000,8192,5);d2p_installed=!rc&&!protection;return d2p_installed;
}
#else
static int d2p_patch(void){d2p_installed=1;return 1;}
#endif
static void d2p_install(void){
 d2p_gate();if(!d2p_enabled)return;
 if(PC_WORD(0x83a6bc)==0x4e9534&&d2p_patch()){d2p_original_configure=PC_FN(0x4e9534,int (*)(unsigned,unsigned,unsigned));PC_WORD(0x83a6bc)=(unsigned)(unsigned long)d2p_configure;}
}
static void d2p_tick(void){
 d2p_gate();if(!d2p_original_configure||PC_WORD(0x83a6bc)==0x4e9534)d2p_install();
 if(d2p_enabled&&d2p_installed&&d2p_source()&&!d2p_active)d2p_configure(PC_WORD(PC_WORD(0x83dfc4)+0xc),32,1);
 unsigned t=pc_seconds();if(t==d2p_report_at)return;d2p_report_at=t;
 char b[512];int n=snprintf(b,sizeof(b),"version=1.0.0\nenabled=%d\ninstalled=%d\nactive=%u\nready=%u\ncarrier=%u\noutput_rate=88200\noutput_bits=32\nblocks=%u\nframes=%u\ninvalid_packets=%u\nquiet_blocks=%u\nreopens=%u\nconfig_failures=%u\nwrite_errors=%u\nDAC_dsd=%u\n",d2p_enabled,d2p_installed,d2p_active,d2p_ready,d2p_carrier,d2p_blocks,d2p_frames,d2p_bad,d2p_silence,d2p_reopens,d2p_failures,d2p_write_errors,PC_BYTE(0x899060));
 int f=open("/tmp/disc-runtime/usbdac-d2p.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}
}
