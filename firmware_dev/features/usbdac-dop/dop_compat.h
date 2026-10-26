/* Stock #236 DoP_init configures DSD128 only. Complete its muted transition
 * for DSD64/128, without replacing the bit-perfect audio data path. */
#include "dop_bus.h"
static int dop_enabled;
static int dop_stock_profile;
static unsigned dop_checked,dop_repaired,dop_repair_attempted;
static unsigned dop_startup_pending,dop_startup_wait_reason,dop_startup_applied;
static unsigned dop_reported;
static int (*dop_original_configure)(unsigned,unsigned,unsigned);
static unsigned dop_rate_repairs,dop_rate_failures,dop_saved_player,dop_saved_rate,dop_carrier;
static int dop_serial_profile;
#include "dop_eq.h"
#ifndef DISC_DOP_TEST
#include "dop_trace.h"
#endif
static void dop_gate_refresh(void){
 unsigned t=pc_seconds();if(dop_checked==t+1)return;dop_checked=t+1;
 char tag[24];int f=open("/usr/data/disc-runtime/components/disc-usbdac-dop/current/FEATURES",0),n=f<0?0:read(f,tag,sizeof(tag)-1);if(f>=0)close(f);
 if(n<0)n=0;tag[n]=0;
 dop_serial_profile=!strcmp(tag,"usbdac-serial-abi1\n");
 dop_stock_profile=!strcmp(tag,"usbdac-stock-abi1\n")||dop_serial_profile;
 dop_enabled=access("/usr/data/disc-runtime/components/disc-usbdac-dop/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0)&&(!strcmp(tag,"usbdac-dop-abi3\n")||dop_stock_profile);
 if(dop_enabled){char kernel[384];f=open("/proc/version",0);n=f<0?0:read(f,kernel,sizeof(kernel)-1);if(f>=0)close(f);int match=0;if(n>=22&&!memcmp(kernel,"Linux version 4.4.94 ",20))for(int i=20;i+5<n;i++)if(!memcmp(kernel+i,"#236 ",5)){match=1;break;}dop_enabled=match;}
}
static int dop_source(void){
 unsigned p=PC_WORD(0x83dfc4),rate=p?PC_WORD(p+0xc):0;
 return dop_enabled&&pc_workmode()==4&&PC_BYTE(0x83a75a)==6&&pc_route()==6&&p&&PC_WORD(0x839a74)==1&&(rate==176400||rate==352800)&&
 PC_WORD(p+0x10)==2&&PC_WORD(p+0x14)==32&&PC_WORD(p+0x18)==0&&PC_WORD(p+0x2c)==2&&PC_WORD(p+0x58)==1&&PC_WORD(p+0x98)==2;
}
static int dop_output_rate(unsigned rate){
 unsigned p=PC_WORD(0x83dfc4),old=p?PC_WORD(p+0x1c):0;
 if(old==rate)return 1;
 if(!p||old<8000||old>384000||PC_WORD(p+0x20)!=2||PC_WORD(p+0x24)!=32||!pc_pcm_open(6))return 0;
 /* This is the same close/open pair used by native output hot-switching.
  * Close is serialized with native writes; capture remains open. */
 dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2d,0);
 PC_FN(0x478354,void (*)(unsigned))(0);
 int rc=PC_FN(0x47859c,int (*)(unsigned,unsigned,unsigned,unsigned,unsigned))(rate,32,2,0,0);
 if(rc||!pc_pcm_open(6)){
  PC_FN(0x47859c,int (*)(unsigned,unsigned,unsigned,unsigned,unsigned))(old,32,2,0,0);
  dop_rate_failures++;return 0;
 }
 PC_WORD(p+0x1c)=rate;__sync_synchronize();dop_rate_repairs++;return 1;
}
static int dop_stream(unsigned rate,unsigned bits,unsigned is_dsd){
 unsigned p=PC_WORD(0x83dfc4);
 return dop_enabled&&pc_workmode()==4&&PC_BYTE(0x83a75a)==6&&pc_route()==6&&pc_pcm_open(6)&&p&&PC_WORD(0x839a74)==1&&(rate==176400||rate==352800)&&bits==32&&is_dsd==1&&
  PC_WORD(p+0xc)==rate&&PC_WORD(p+0x10)==2&&PC_WORD(p+0x14)==32&&PC_WORD(p+0x18)==0&&PC_WORD(p+0x1c)==rate&&PC_WORD(p+0x20)==2&&PC_WORD(p+0x24)==32&&PC_WORD(p+0x2c)==2&&PC_WORD(p+0x58)==1&&
  (int)PC_WORD(0x83a7d0)>=0&&(int)PC_WORD(0x83a7cc)>=0;
}
#include "dop_serial.h"
static int dop_ioctl(int fd,unsigned request,void *arg){
 unsigned p=PC_WORD(0x83dfc4),rate=p?PC_WORD(p+0xc):0;
 int stream=dop_stream(rate,32,1)&&PC_BYTE(0x899060)==1;
 int left=fd==(int)PC_WORD(0x83a7d0),right=fd==(int)PC_WORD(0x83a7cc);
 /* The stock periodic check compares against a fixed PCM code even in DoP.
  * Read the actual DoP source/rate/speed instead; healthy playback is not reset. */
 if(stream&&((left&&request==0x20004d28)||(right&&request==0x20004d2b))){
  dop_health_checks++;if(dop_bus_check(rate,0)){dop_health_avoided++;return 0;}
  /* Never run the PCM health reset while valid DSD packets continue. */
  dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2d,0);
  if(!dop_bus_check(rate,1))return -1;
  dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2e,0);return 0;
 }
 /* Correct both chips while the native transition is still muted, before
  * its existing unmute. Also covers stock wake/unmute restoring DoP_init. */
 if(stream&&left&&request==0x20004d2e&&!dop_bus_check(rate,1))return -1;
 return dop_real_ioctl(fd,request,arg);
}
static int dop_configure(unsigned rate,unsigned bits,unsigned is_dsd){
 if(dop_stock_profile){
  /* Reuse the working local DAC transition verbatim. No register repair,
   * PCM reopen, health override or starvation recovery in this profile. */
  if(dop_stream(rate,bits,is_dsd)&&dop_eq_ready&&dop_original_gain){
   PC_FN(0x4e8858,void (*)(unsigned,unsigned,unsigned))(rate,bits,is_dsd);
   dop_serial_finish(rate);
   dop_repaired++;return 0;
  }
  return dop_original_configure(rate,bits,is_dsd);
 }
 unsigned source_player=PC_WORD(0x83dfc4);int output_reopened=0;
 if(dop_source()&&bits==32&&is_dsd==1&&(rate==PC_WORD(source_player+0x1c)||rate==PC_WORD(source_player+0xc))){
  unsigned p=PC_WORD(0x83dfc4),input=PC_WORD(p+0xc);
  if(!dop_eq_ready||!dop_original_gain)return -1;
  if(PC_WORD(p+0x1c)!=input){
   if(dop_saved_player!=p){dop_saved_player=p;dop_saved_rate=PC_WORD(p+0x1c);}
   if(!dop_output_rate(input))return -1;
   output_reopened=1;
   dop_carrier=input;
  }
  rate=input;
 }else if(dop_saved_player&&(!is_dsd||!dop_source())){
  unsigned p=PC_WORD(0x83dfc4);
  if(p==dop_saved_player&&PC_WORD(p+0x2c)==1&&PC_WORD(p+0x1c)==dop_carrier&&pc_workmode()==4&&pc_route()==6&&dop_enabled){
   if(!dop_output_rate(dop_saved_rate))return -1;rate=dop_saved_rate;output_reopened=1;
  }
  dop_saved_player=dop_saved_rate=dop_carrier=0;
 }
 if(dop_stream(rate,bits,is_dsd)){
  unsigned old_rate=PC_WORD(0x899084);int old_dsd=PC_BYTE(0x899060);
  /* Native mode-change path provides its own mute. A rate-only DoP change
   * does not, so bracket the two register updates explicitly. */
  if(old_dsd&&old_rate!=rate)dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2d,0);
  PC_FN(0x4e8858,void (*)(unsigned,unsigned,unsigned))(rate,bits,is_dsd);
  if(output_reopened||(old_dsd&&old_rate!=rate)){if(!dop_bus_check(rate,1))return -1;dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2e,0);}
  dop_repaired++;return 0;
 }
 int result=dop_original_configure(rate,bits,is_dsd);
 if(output_reopened&&!PC_BYTE(0x899060))dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2e,0);
 return result;
}
static void dop_install(void);
static void dop_tick(void){
 /* Stock initialization runs after the first extension callback and can
  * replace the callback table. Bind after that initialization as well. */
 if(PC_WORD(0x83a6bc)==0x4e9534)dop_install();
 if(PC_WORD(0x83a6bc)!=(unsigned)(unsigned long)dop_configure)return;
 int was_serial=dop_serial_profile;dop_gate_refresh();if(dop_enabled&&!dop_eq_attempted&&!dop_eq_ready)dop_eq_install();unsigned p=PC_WORD(0x83dfc4);
 if(dop_enabled&&dop_serial_profile&&dop_serial_bind()&&!was_serial&&p)dop_serial_finish(PC_WORD(p+0xc));
 if(dop_enabled)dop_capture_poll();
 unsigned now=pc_seconds();if(now!=dop_reported){dop_reported=now;
#ifndef DISC_DOP_TEST
 if(!dop_stock_profile){dop_trace_poll();dop_recovery_report();}
#endif
 char b[768];int n=snprintf(b,sizeof(b),"enabled=%d\nrepairs=%u\nhealth_checks=%u\nresets_avoided=%u\nregister_writes=%u\nbus_failures=%u\nframe_repairs=%u\nrate=%u\nDAC_dsd=%u\neq_guard=%d\neq_bypasses=%u\ngain_bypasses=%u\nrate_repairs=%u\nrate_failures=%u\noutput_rate=%u\nserial_profile=%d\nserial_applied=%u\nserial_failed=%u\nserial_health_forwarded=%u\nstartup_pending=%u\nstartup_wait_reason=%u\nstartup_applied=%u\n",dop_enabled,dop_repaired,dop_health_checks,dop_health_avoided,dop_bus_writes,dop_bus_failures,dop_frame_repairs,p?PC_WORD(p+0xc):0,PC_BYTE(0x899060),dop_eq_ready,dop_eq_bypasses,dop_gain_bypasses,dop_rate_repairs,dop_rate_failures,p?PC_WORD(p+0x1c):0,dop_serial_profile,dop_serial_applied,dop_serial_failed,dop_serial_health_forwarded,dop_startup_pending,dop_startup_wait_reason,dop_startup_applied);int f=open("/tmp/disc-runtime/usbdac-dop.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n<(int)sizeof(b)?n:(int)sizeof(b)-1);close(f);}}
 unsigned rate=p?PC_WORD(p+0xc):0;
 if(!p||!dop_source()){dop_repair_attempted=0;dop_startup_pending=dop_startup_wait_reason=0;if(!p||pc_workmode()!=4||PC_WORD(p+0x48)==0||PC_WORD(p+0x48)==3)dop_saved_player=dop_saved_rate=dop_carrier=0;return;}
 /* One repair for a stream already classified before installation.
  * Later format changes use the native callback; no polling I2C. */
 if((!PC_BYTE(0x899060)||PC_WORD(p+0x1c)!=rate)&&!dop_repair_attempted){
  if(dop_stock_profile){
   /* A source may be classified before its output/gain guards are ready.
    * Do not consume the one-shot by falling back to stock's DSD no-op.
    * These are RAM readiness checks on the existing tick, not I2C polling. */
   unsigned reason=(!dop_eq_ready||!dop_original_gain)?1u:0u;
   if(!pc_pcm_open(6))reason|=2u;
   if(PC_WORD(p+0x1c)!=rate)reason|=4u;
   if((int)PC_WORD(0x83a7d0)<0||(int)PC_WORD(0x83a7cc)<0)reason|=8u;
   if(!reason&&!dop_stream(rate,32,1))reason=16u;
   dop_startup_pending=1;dop_startup_wait_reason=reason;
   if(reason)return;
   unsigned before=dop_repaired;
   dop_configure(rate,32,1);
   /* Count entry into native DAC configuration, not a verified HW reset.
    * If readiness changed, keep pending without calling hardware in a loop. */
   if(dop_repaired!=before){dop_repair_attempted=1;dop_startup_applied++;dop_startup_pending=dop_startup_wait_reason=0;}
  }else{dop_repair_attempted=1;dop_configure(rate,32,1);}
 }else dop_startup_pending=dop_startup_wait_reason=0;
}
static void dop_install(void){
 /* A disabled component must leave stock callbacks and imports untouched
  * in a fresh player. Existing active bindings keep their guarded forward
  * path until the normal player reload, because threads may still use them. */
 dop_gate_refresh();
 if(!dop_enabled&&!dop_original_configure){
#ifndef DISC_DOP_TEST
  unsigned now=pc_seconds();if(now!=dop_reported){dop_reported=now;
   const char off[]="enabled=0\neq_guard=0\naudio_hooks=0\nstate=stock_callbacks\n";
   int f=open("/tmp/disc-runtime/usbdac-dop.state",1|0x100|0x200,0600);
   if(f>=0){write(f,off,sizeof(off)-1);close(f);}
  }
#endif
  return;
 }
 if(dop_stock_profile){
  if(PC_WORD(0x83a6bc)!=0x4e9534)return;
  if(dop_serial_profile){
   if(!dop_serial_bind())return;
  }
  dop_original_configure=PC_FN(0x4e9534,int (*)(unsigned,unsigned,unsigned));
  if(PC_WORD(0x83a704)==0x4e94f4){dop_original_gain=PC_FN(0x4e94f4,int (*)(void));PC_WORD(0x83a704)=(unsigned)(unsigned long)dop_gain_filter;}
  if(dop_enabled)dop_eq_install();
  PC_WORD(0x83a6bc)=(unsigned)(unsigned long)dop_configure;return;
 }
#ifndef DISC_DOP_TEST
 if(!dop_pcm_config)dop_pcm_config=(const unsigned *(*)(void *))dlsym((void *)0,"pcm_get_config");
 if(!dop_pcm_stop)dop_pcm_stop=(int (*)(void *))dlsym((void *)0,"pcm_stop");
 if(!dop_pcm_prepare)dop_pcm_prepare=(int (*)(void *))dlsym((void *)0,"pcm_prepare");
 if(!dop_pcm_htimestamp)dop_pcm_htimestamp=(int (*)(void *,unsigned *,long *))dlsym((void *)0,"pcm_get_htimestamp");
 if(!dop_real_pcm_write){dop_real_pcm_write=(int (*)(void *,const void *,unsigned))dlsym((void *)0,"pcm_write");if(dop_real_pcm_write)PC_WORD(PLAYER_PCM_WRITE_GOT)=(unsigned)(unsigned long)dop_trace_write;}
 if(!dop_real_pcm_read){dop_real_pcm_read=(int (*)(void *,void *,unsigned))dlsym((void *)0,"pcm_read");if(dop_real_pcm_read)PC_WORD(PLAYER_PCM_READ_GOT)=(unsigned)(unsigned long)dop_trace_read;}
#endif
 if(PC_WORD(0x83a6bc)!=0x4e9534)return;
 if(!dop_real_ioctl){dop_real_ioctl=(int (*)(int,unsigned,void *))dlsym((void *)0,"ioctl");if(!dop_real_ioctl)return;PC_WORD(PLAYER_IOCTL_GOT)=(unsigned)(unsigned long)dop_ioctl;}
 dop_original_configure=PC_FN(0x4e9534,int (*)(unsigned,unsigned,unsigned));dop_gate_refresh();
 if(dop_enabled)dop_eq_install();
 if(PC_WORD(0x83a704)==0x4e94f4){dop_original_gain=PC_FN(0x4e94f4,int (*)(void));PC_WORD(0x83a704)=(unsigned)(unsigned long)dop_gain_filter;}
 PC_WORD(0x83a6bc)=(unsigned)(unsigned long)dop_configure;
}
