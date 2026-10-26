/* USB-only output owner for pinned V2.57 / kernel #236. The stock UAC
 * receiver and format classification remain; this replaces its final DSD
 * output stage and every competing quiet-output call. */
#include "packet.h"
static int dsde_enabled,dsde_installed,dsde_attempted;
static volatile unsigned dsde_active,dsde_ready,dsde_primed,dsde_failed;
static unsigned dsde_rate,dsde_checked,dsde_reported,dsde_generation;
static unsigned dsde_packets,dsde_payload_frames,dsde_silent_frames,dsde_invalid,dsde_quiet_skips,dsde_recoveries,dsde_errors,dsde_health_skips;
static unsigned dsde_pcm_fd,dsde_pcm_generation,dsde_start_saved,dsde_stop_saved;
static void *dsde_pcm;
static int dsde_mutex[16];
static unsigned char dsde_buffer[65536];
static DiscDsdPacket dsde_packet;
static int (*dsde_real_write)(void *,const void *,unsigned),(*dsde_stop)(void *),(*dsde_prepare)(void *);
static const unsigned *(*dsde_config)(void *);
static int (*dsde_stamp)(void *,unsigned *,long *);
static int (*dsde_original_configure)(unsigned,unsigned,unsigned);
#ifndef DSDE_PCM_WORD
#define DSDE_PCM_WORD(p,o) (*(volatile unsigned *)((unsigned char *)(p)+(o)))
#define DSDE_PCM_STATE(p) (*(volatile unsigned *)(unsigned long)DSDE_PCM_WORD(p,0xb8))
#endif
static void dsde_gate(void){
 unsigned t=pc_seconds();if(dsde_checked==t+1)return;dsde_checked=t+1;
 char b[384];int f=open("/usr/data/disc-runtime/components/disc-usbdac-dsd-engine/current/FEATURES",0),n=f<0?0:read(f,b,40);if(f>=0)close(f);if(n<0)n=0;b[n]=0;
 dsde_enabled=!strcmp(b,"usbdac-dsd-engine-abi1\n")&&access("/usr/data/disc-runtime/components/disc-usbdac-dsd-engine/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0)&&
  !access("/usr/data/disc-runtime/components/disc-usbdac-dop/disabled",0)&&!access("/usr/data/disc-runtime/components/disc-usbdac-d2p/disabled",0);
 if(dsde_enabled){f=open("/proc/version",0);n=f<0?0:read(f,b,sizeof(b)-1);if(f>=0)close(f);int match=0;if(n>=22&&!memcmp(b,"Linux version 4.4.94 ",20))for(int i=20;i+5<n;i++)if(!memcmp(b+i,"#236 ",5)){match=1;break;}dsde_enabled=match;}
}
static int dsde_scope(void){return pc_workmode()==4&&PC_BYTE(0x83a75a)==6&&pc_route()==6;}
static int dsde_source(void){
 unsigned p=PC_WORD(0x83dfc4),r=p?PC_WORD(p+0xc):0;
 return dsde_scope()&&p&&PC_WORD(0x839a74)==1&&(r==176400||r==352800)&&PC_WORD(p+0x10)==2&&PC_WORD(p+0x14)==32&&PC_WORD(p+0x18)==0&&PC_WORD(p+0x2c)==2&&PC_WORD(p+0x58)==1&&PC_WORD(p+0x98)==2;
}
/* A periodic native DSD check otherwise compares ASP rate to fixed0x0b.
 * Never let it reset an engine transition or a verified healthy stream. */
static int dsde_ioctl(int fd,unsigned request,void *arg){
 int owned=dsde_active&&dsde_source()&&PC_BYTE(0x899060)==1;
 int left=fd==(int)PC_WORD(0x83a7d0),right=fd==(int)PC_WORD(0x83a7cc);
 if(owned&&((left&&request==0x20004d28)||(right&&request==0x20004d2b))){
  if(!dsde_primed||dop_bus_check(dsde_rate,0)){dsde_health_skips++;return 0;}
 }
 /* Native wake paths may unmute while the DMA buffer still has stale PCM.
  * This writer unmutes explicitly only after validated DSD prefill. */
 if(owned&&left&&request==0x20004d2e){
  if(!dsde_primed)return 0;
  if(!dop_bus_check(dsde_rate,1)){dsde_failed=1;dsde_errors++;return -1;}
 }
 return dop_real_ioctl(fd,request,arg);
}
static int dsde_layout(void *pcm){
 if(!pcm||!dsde_config||!dsde_real_write||!dsde_stop||!dsde_prepare||!dsde_stamp)return 0;
 const unsigned *c=dsde_config(pcm);unsigned size=DSDE_PCM_WORD(pcm,0x10);
 return c==(const unsigned *)((unsigned char *)pcm+0x98)&&(int)DSDE_PCM_WORD(pcm,0)>=0&&!(DSDE_PCM_WORD(pcm,4)&0x10000003u)&&DSDE_PCM_WORD(pcm,0xb8)&&
  c[0]==2&&c[1]==dsde_rate&&c[2]==dsde_rate/25&&c[3]==2&&c[4]==7&&size==c[2]*c[3]&&DSDE_PCM_WORD(pcm,0x14)>=size;
}
static int dsde_sw(void *pcm,unsigned start,unsigned stop){
 unsigned sw[26]={0};sw[0]=sw[1]=sw[3]=1;sw[4]=DSDE_PCM_WORD(pcm,0xa0)/2;sw[5]=start;sw[6]=stop;sw[7]=DSDE_PCM_WORD(pcm,0xb4);sw[9]=DSDE_PCM_WORD(pcm,0x14);
 if(dop_real_ioctl((int)DSDE_PCM_WORD(pcm,0),0xc0684113u,sw))return 0;
 DSDE_PCM_WORD(pcm,0xac)=start;DSDE_PCM_WORD(pcm,0xb0)=stop;return 1;
}
/* Engine mutex then the native output mutex: native close/open cannot retire
 * a PCM handle during validation, prefill or writes. No new DAC/PCM opens. */
static void *dsde_current_pcm(unsigned sink){return (void *)(unsigned long)PC_WORD(sink+6*0x54+0x74);}
static int dsde_start(void *pcm){
 dsde_primed=0;
 if(dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2d,0)<0||dsde_stop(pcm)||dsde_prepare(pcm))goto failed;
 unsigned size=DSDE_PCM_WORD(pcm,0x10);
 if(pcm!=dsde_pcm||dsde_pcm_generation!=dsde_generation||dsde_pcm_fd!=DSDE_PCM_WORD(pcm,0)||DSDE_PCM_WORD(pcm,0xac)!=size||DSDE_PCM_WORD(pcm,0xb0)!=size){
  /* A same-format reconfigure may retain this already-managed handle.
   * Save native thresholds only for a new handle or native-reset fields. */
  if(pcm!=dsde_pcm||dsde_pcm_fd!=DSDE_PCM_WORD(pcm,0)||DSDE_PCM_WORD(pcm,0xac)!=size||DSDE_PCM_WORD(pcm,0xb0)!=size){
   dsde_start_saved=DSDE_PCM_WORD(pcm,0xac);dsde_stop_saved=DSDE_PCM_WORD(pcm,0xb0);
  }
  if(!dsde_sw(pcm,size,size))goto failed;
  dsde_pcm=pcm;dsde_pcm_fd=DSDE_PCM_WORD(pcm,0);dsde_pcm_generation=dsde_generation;
 }
 if(!dop_bus_check(dsde_rate,1))goto failed;
 dsde_packet_reset(&dsde_packet);
 /* Prime a whole DMA buffer with correctly framed DSD silence under mute.
  * This also provides far more than the32 DoP identification markers. */
 for(unsigned remaining=size;remaining;){
  unsigned frames=remaining>8192?8192:remaining;
  dsde_packet_copy(&dsde_packet,0,frames*8,dsde_buffer,1);
  if(dsde_real_write(pcm,dsde_buffer,frames*8)<0)goto failed;
  dsde_silent_frames+=frames;remaining-=frames;
 }
 if(DSDE_PCM_STATE(pcm)!=3||dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2e,0)<0)goto failed;
 dsde_primed=1;return 1;
failed:
 dsde_failed=1;dsde_errors++;return 0;
}
static int dsde_xrun(void *pcm){
 unsigned a=0;long t[2]={0};int rc=dsde_stamp(pcm,&a,t);unsigned state=DSDE_PCM_STATE(pcm);
 return state==4||(state==3&&!rc&&a>DSDE_PCM_WORD(pcm,0x10));
}
static int dsde_configure(unsigned rate,unsigned bits,unsigned dsd){
 unsigned p=PC_WORD(0x83dfc4);pthread_mutex_lock(dsde_mutex);
 if((dsde_enabled||dsde_active)&&bits==32&&dsd==1&&dsde_source()&&rate==PC_WORD(p+0xc)){
  dsde_active=1;dsde_ready=dsde_primed=dsde_failed=0;dsde_rate=rate;dsde_generation++;
  /* Reuse the verified local DSD mode/volume setup, but keep output muted
   * until this engine has discarded previous DMA data and primed DoP. */
  PC_FN(0x4e8858,void (*)(unsigned,unsigned,unsigned))(rate,bits,1);
  int ok=dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2d,0)>=0&&dop_bus_check(rate,1);
  dsde_ready=ok;if(!ok){dsde_failed=1;dsde_errors++;}
  pthread_mutex_unlock(dsde_mutex);return ok?0:-1;
 }
 unsigned sink=PC_WORD(0x83dfe4);
 if(dsde_pcm&&sink&&pc_pcm_open(6)){
  pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));void *pcm=dsde_current_pcm(sink);
  if(pcm==dsde_pcm&&dsde_layout(pcm)&&DSDE_PCM_WORD(pcm,0)==dsde_pcm_fd)dsde_sw(pcm,dsde_start_saved,dsde_stop_saved);
  pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));
 }
 dsde_active=dsde_ready=dsde_primed=dsde_failed=0;dsde_pcm=0;dsde_generation++;
 int rc=dsde_original_configure(rate,bits,dsd);pthread_mutex_unlock(dsde_mutex);return rc;
}
static int dsde_output(void *data,int bytes,int quiet){
 if(!dsde_scope()||(!dsde_active&&!dsde_enabled))return PC_FN(0x465718,int (*)(void *,int,int))(data,bytes,quiet);
 if(!dsde_source()){
  if(!dsde_active||!PC_BYTE(0x899060))return PC_FN(0x465718,int (*)(void *,int,int))(data,bytes,quiet);
  pc_real_usleep(1000);return 0; /* Wait for the native PCM format transition. */
 }
 if(bytes<=0||bytes>0x12c000||(bytes&7)){dsde_invalid++;pc_real_usleep(1000);return 0;}
 pthread_mutex_lock(dsde_mutex);
 unsigned sink=PC_WORD(0x83dfe4);int valid=quiet?1:dsde_packet_valid(data,(unsigned)bytes);
 if(!dsde_active||!dsde_ready||dsde_failed||!sink||!pc_pcm_open(6)||(!dsde_primed&&(quiet||!valid||(unsigned)bytes<32*8))){
  pthread_mutex_unlock(dsde_mutex);pc_real_usleep(5000);return 0;
 }
 pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));void *pcm=dsde_current_pcm(sink);
 if(!dsde_layout(pcm)||PC_WORD(PC_WORD(0x83dfc4)+0x1c)!=dsde_rate){
  dsde_errors++;goto done;
 }
 if(quiet){
  unsigned available=0;long t[2]={0};int rc=dsde_stamp(pcm,&available,t);
  if(rc||available<DSDE_PCM_WORD(pcm,0xa0)||available>DSDE_PCM_WORD(pcm,0x10)){
   dsde_quiet_skips++;goto done;
  }
  if((unsigned)bytes>available*8)bytes=(int)(available*8);
 }
 if(!dsde_primed||dsde_xrun(pcm)){if(dsde_primed)dsde_recoveries++;if(!dsde_start(pcm))goto done;}
 if(!valid)dsde_invalid++;
 /* All source bytes are consumed: never truncate the native large bursts.
  * Small writes use a persistent buffer instead of clearing240KB on stack. */
 for(unsigned offset=0;offset<(unsigned)bytes;){
  unsigned n=(unsigned)bytes-offset;if(n>sizeof(dsde_buffer))n=sizeof(dsde_buffer);
  unsigned underruns=DSDE_PCM_WORD(pcm,0xc);
  dsde_packet_copy(&dsde_packet,(const unsigned char *)data+offset,n,dsde_buffer,quiet||!valid);
  int rc=dsde_real_write(pcm,dsde_buffer,n);
  if(rc<0||DSDE_PCM_WORD(pcm,0xc)!=underruns){
   /* One bounded resynchronization, then reframe this unchanged payload.
    * Never leave a hidden tinyalsa prepare running with stale DMA contents. */
   dsde_recoveries++;if(!dsde_start(pcm))break;
   dsde_packet_copy(&dsde_packet,(const unsigned char *)data+offset,n,dsde_buffer,quiet||!valid);
   underruns=DSDE_PCM_WORD(pcm,0xc);rc=dsde_real_write(pcm,dsde_buffer,n);
   if(rc<0||DSDE_PCM_WORD(pcm,0xc)!=underruns){dsde_failed=1;dsde_errors++;dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2d,0);break;}
  }
  if(quiet||!valid)dsde_silent_frames+=n/8;else dsde_payload_frames+=n/8;
  offset+=n;
 }
 dsde_packets++;
done:
 pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));pthread_mutex_unlock(dsde_mutex);
 if(quiet)pc_real_usleep(2000);return 0;
}
#ifndef DISC_DSD_ENGINE_TEST
static int dsde_patch(void){
 if(dsde_installed)return 1;if(dsde_attempted)return 0;dsde_attempted=1;
 const unsigned sites[7]={0x465970,0x465af8,0x465d64,0x465ec0,0x466338,0x466420,0x466980};
 const unsigned delay[7]={0x34058000,0x02202025,0x34058000,0x00402825,0xafa20040,0x34058000,0x00402825};
 for(unsigned i=0;i<7;i++)if(PC_WORD(sites[i])!=0x0c1195c6||PC_WORD(sites[i]+4)!=delay[i])return 0;
 unsigned *stub=mmap((void *)0x07200000,4096,3,0x802,-1,0);
 unsigned address=(unsigned)(unsigned long)stub,target=(unsigned)(unsigned long)dsde_output;
 if(address<0x10000||address>=0x10000000u||(address&4095)){if(stub!=(void *)-1)munmap(stub,4096);return 0;}
 stub[0]=0x3c190000|(target>>16);stub[1]=0x37390000|(target&65535);stub[2]=0x03200008;stub[3]=0;
 if(cacheflush(stub,16,3)||mprotect(stub,4096,5)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x465000,8192,7)){munmap(stub,4096);return 0;}
 for(unsigned i=0;i<7;i++)PC_WORD(sites[i])=0x0c000000|(address>>2);
 __sync_synchronize();int rc=cacheflush((void *)0x465000,8192,3);
 if(rc){for(unsigned i=0;i<7;i++)PC_WORD(sites[i])=0x0c1195c6;cacheflush((void *)0x465000,8192,3);}
 int protection=mprotect((void *)0x465000,8192,5);dsde_installed=!rc&&!protection;return dsde_installed;
}
#else
static int dsde_patch(void){dsde_installed=1;return 1;}
#endif
static void dsde_install(void){
 dsde_gate();if(!dsde_enabled||PC_WORD(0x83a6bc)!=0x4e9534)return;
 if(!dop_real_ioctl)dop_real_ioctl=(int (*)(int,unsigned,void *))dlsym((void *)0,"ioctl");
 dsde_real_write=(int (*)(void *,const void *,unsigned))dlsym((void *)0,"pcm_write");
 dsde_config=(const unsigned *(*)(void *))dlsym((void *)0,"pcm_get_config");dsde_stamp=(int (*)(void *,unsigned *,long *))dlsym((void *)0,"pcm_get_htimestamp");
 dsde_stop=(int (*)(void *))dlsym((void *)0,"pcm_stop");dsde_prepare=(int (*)(void *))dlsym((void *)0,"pcm_prepare");
 if(!dop_real_ioctl||!dsde_real_write||!dsde_config||!dsde_stamp||!dsde_stop||!dsde_prepare||PC_WORD(PLAYER_IOCTL_GOT)!=(unsigned)(unsigned long)dop_real_ioctl)return;
 if(!dsde_patch())return;
 dsde_original_configure=PC_FN(0x4e9534,int (*)(unsigned,unsigned,unsigned));
 PC_WORD(PLAYER_IOCTL_GOT)=(unsigned)(unsigned long)dsde_ioctl;PC_WORD(0x83a6bc)=(unsigned)(unsigned long)dsde_configure;
}
static void dsde_tick(void){
 dsde_gate();if(dsde_enabled&&PC_WORD(0x83a6bc)==0x4e9534)dsde_install();
 if(dsde_enabled&&dsde_installed&&dsde_source()&&!dsde_active)dsde_configure(PC_WORD(PC_WORD(0x83dfc4)+0xc),32,1);
 unsigned t=pc_seconds();if(t==dsde_reported)return;dsde_reported=t;
 char b[768];int n=snprintf(b,sizeof(b),"version=1.0.0\npid=%d\nenabled=%d\ninstalled=%d\nactive=%u\nready=%u\nprimed=%u\nfailure_latched=%u\ncarrier=%u\nDSD_conversion=0\npackets=%u\npayload_frames=%u\nsilent_frames=%u\ninvalid_packets=%u\nquiet_skips=%u\nrecoveries=%u\nerrors=%u\nhealth_skips=%u\nmarker_rebases=%u\nDAC_dsd=%u\n",getpid(),dsde_enabled,dsde_installed,dsde_active,dsde_ready,dsde_primed,dsde_failed,dsde_rate,dsde_packets,dsde_payload_frames,dsde_silent_frames,dsde_invalid,dsde_quiet_skips,dsde_recoveries,dsde_errors,dsde_health_skips,dsde_packet.rebases,PC_BYTE(0x899060));
 int f=open("/tmp/disc-runtime/usbdac-dsd-engine.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}
}
