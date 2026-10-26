/* Pinned original PLAYER: output switching and isolated volume profiles. */
#include "player_controls_guard.h"
#ifndef PC_WORD
#define PC_WORD(a) (*(volatile unsigned *)(unsigned long)(a))
#define PC_BYTE(a) (*(volatile unsigned char *)(unsigned long)(a))
#define PC_FN(a,t) ((t)(unsigned long)(a))
#endif
static int (*pc_real_usleep)(unsigned);
static void (*pc_transport_tick)(void);
static void (*pc_cover_tick)(void);
static int (*pc_mixer_db)(void *,long,int),(*pc_mixer_switch)(void *,int),(*pc_mixer_set)(void *,long);
extern int pthread_mutex_lock(void *);
static unsigned pc_switches,pc_volume_updates,pc_last_mixer,pc_last_report;
static int pc_result,pc_last_volume=-1,pc_volume_active,pc_usb_volume=-1,pc_internal_volume;
static unsigned pc_volume_slot;static int pc_volume_mode,pc_volume_max=120,pc_loaded_mode;
static volatile int pc_switching;
static unsigned pc_volume_changed;static int pc_volume_dirty;
static int pc_enabled(void){return !access("/usr/data/disc-runtime/components/disc-bt-meter/enabled",0)&&access("/usr/data/disc-runtime/components/disc-bt-meter/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);}
static int pc_owned_request(void){return !access("/tmp/disc-runtime/bt-usb.request",0);}
static int pc_workmode(void){unsigned p=PC_WORD(0x83dfc0);return p?(int)PC_WORD(p):0;}
static unsigned pc_route(void){unsigned p=PC_WORD(0x83dfe4);return p?PC_WORD(p+0x58):0;}
static int pc_pcm_open(unsigned route){unsigned p=PC_WORD(0x83dfe4);return p&&(route==3||route==4||route==6)&&PC_WORD(p+route*0x54+0x68)&&PC_WORD(p+route*0x54+0x74);}
static void pc_reconcile_route(unsigned output){unsigned p=PC_WORD(0x83dfe4),player=PC_WORD(0x83dfc4);if(!p||!player)return;unsigned state=PC_WORD(player+0x48);if(state==0||state==3)return;
 /* Native USB hotplug can change the selected route without closing/opening
  * PCM. Restore the still-open device before native hot-switch compares IDs. */
 pthread_mutex_lock((void *)(unsigned long)(p+0x1c));
 if(PC_WORD(p+0x58)==output&&!pc_pcm_open(output)){const unsigned candidates[]={6,3,4};for(unsigned i=0;i<3;i++)if(candidates[i]!=output&&pc_pcm_open(candidates[i])){PC_WORD(p+0x58)=candidates[i];break;}}
 pthread_mutex_unlock((void *)(unsigned long)(p+0x1c));
}
static unsigned pc_seconds(void){long clock[2];return clock_gettime(1,clock)?0:(unsigned)clock[0];}
static void dsp_process_buffer(void *,int,int);
#include "../visual-signal/player.h"
#include "../usbdac-dop/dop_compat.h"
#include "../usbdac-d2p/binding.h"
#include "../usbdac-dsd-engine/binding.h"
#include "../usbdac-dsd-backend/binding.h"
#include "../usbdac-ram-test/binding.h"
#include "../audio-dsp/binding.h"
#include "../player-utilities/replaygain_player.h"
#include "../dlna/lifecycle.h"
#include "../dlna/output.h"
#include "../dlna/transport.h"
#include "lhdcv5_player.h"
/* Match the pinned stock key/get-volume selector (4ea718/4ec02c).
 * Pure playback normally uses83a74c; Bluetooth uses83a7a0. */
static unsigned pc_native_volume_slot(void){
 if(PC_BYTE(0x83a78a)==2)return 0x83a7a2;
 unsigned out=PC_BYTE(0x83a75a);
 if(PC_BYTE(0x83a78a)==5&&(out==3||out==7))return 0x83a789;
 if(out==2)return PC_WORD(0x8a143c)==1?0x83a758:0x83a7a1;
 if(PC_BYTE(0x83a78b)==2||PC_BYTE(0x83a799)==3)return 0x83a7a0;
 if(PC_BYTE(0x83a78b)==6||PC_BYTE(0x83a799)==5)return 0x83a7a3;
 return 0x83a74c;
}
static const char *pc_volume_file(int temporary){
 if(pc_loaded_mode==1)return temporary?"/usr/data/disc-runtime/module-data/disc-bt-meter/pure-usb-volume.new":"/usr/data/disc-runtime/module-data/disc-bt-meter/pure-usb-volume";
 return temporary?"/usr/data/disc-runtime/module-data/disc-bt-meter/usb-volume.new":"/usr/data/disc-runtime/module-data/disc-bt-meter/usb-volume";
}
static void pc_save_usb(void){if(!pc_volume_dirty||pc_usb_volume<0)return;char b[20];int n=snprintf(b,sizeof(b),"%d\n",pc_usb_volume);int f=open(pc_volume_file(1),1|0x100|0x200,0600);if(f>=0){int ok=write(f,b,n)==n;close(f);if(ok&&!rename(pc_volume_file(1),pc_volume_file(0)))pc_volume_dirty=0;}}
static void pc_load_usb(void){int mode=pc_workmode();if(pc_usb_volume>=0&&pc_loaded_mode==mode)return;pc_loaded_mode=mode;pc_usb_volume=0;pc_volume_dirty=0;char b[20];int f=open(pc_volume_file(0),0),n=f<0?0:read(f,b,sizeof(b)-1);if(f>=0)close(f);if(n>0){b[n]=0;char *end=0;long v=strtol(b,&end,10);if(end!=b&&(*end=='\n'||!*end)&&v>=0&&v<=120)pc_usb_volume=(int)v;}}
static void pc_echo_volume(unsigned value){PC_FN(0x43db60,void (*)(unsigned,unsigned))(0x3f,value);}
static void pc_volume_enter(void){if(pc_volume_active)return;pc_load_usb();pc_volume_mode=pc_workmode();pc_volume_slot=pc_volume_mode==1?pc_native_volume_slot():0x83a7a0;pc_volume_max=pc_volume_slot==0x83a789?100:120;pc_internal_volume=PC_BYTE(pc_volume_slot);unsigned v=(pc_usb_volume*pc_volume_max+60)/120;PC_BYTE(pc_volume_slot)=v;PC_BYTE(0x83a789)=(pc_usb_volume*100+60)/120;pc_volume_active=1;pc_last_volume=-1;pc_last_mixer=0;pc_echo_volume(v);}
static void pc_volume_leave(void){if(!pc_volume_active)return;unsigned v=PC_BYTE(pc_volume_slot);if(v>(unsigned)pc_volume_max)v=pc_volume_max;pc_usb_volume=(v*120+pc_volume_max/2)/pc_volume_max;pc_volume_dirty=1;pc_save_usb();PC_BYTE(pc_volume_slot)=pc_internal_volume;pc_volume_active=0;pc_last_volume=-1;pc_last_mixer=0;pc_echo_volume(pc_internal_volume);}
/* One stable USB hardware level. Physical keys change software gain only. */
static void pc_usb_hardware_ready(void){unsigned h=PC_WORD(0x85fde0);if(!h||h==pc_last_mixer||!PC_WORD(0x85fddc))return;pthread_mutex_lock((void *)0x85fdc0);unsigned e=PC_WORD(0x85fddc);int result=-1;if(h==PC_WORD(0x85fde0)&&e){if(pc_mixer_db)result=pc_mixer_db((void *)e,0,0);if(result&&pc_mixer_set){int lo=(int)PC_WORD(0x85fdd8),hi=(int)PC_WORD(0x839e70);if(hi>lo)result=pc_mixer_set((void *)e,hi);}if(!result&&pc_mixer_switch)result=pc_mixer_switch((void *)e,1);if(!result)pc_last_mixer=h;}pthread_mutex_unlock((void *)0x85fdc0);}
static void pc_apply_usb_volume(void){unsigned raw=PC_BYTE(pc_volume_slot);if(raw>(unsigned)pc_volume_max)raw=pc_volume_max;unsigned v=(raw*120+pc_volume_max/2)/pc_volume_max;unsigned percent=(raw*100+pc_volume_max/2)/pc_volume_max;PC_BYTE(0x83a789)=percent;if(pc_last_volume!=(int)v){PC_FN(0x4ea5d4,void (*)(unsigned))(percent);pc_usb_volume=v;pc_last_volume=v;pc_volume_updates++;pc_volume_dirty=1;pc_volume_changed=pc_seconds();}pc_usb_hardware_ready();}
#include "usb_reconnect_player.h"
#include "../player-utilities/usb_player.h"
static void pc_status(void){char b[1536];unsigned player=PC_WORD(0x83dfc4),sink=PC_WORD(0x898d70);int n=snprintf(b,sizeof(b),"version=1.3.217\npid=%d\nready=1\nwork_mode=%d\noutput=%u\nroute=%u\nswitches=%u\nresult=%d\nvolume=%d\ninternal_volume=%d\nusb_volume=%d\nvolume_profile=%s\nvolume_updates=%u\nserver=%u\nreader=%u\ndac_off=%u\nplayer_state=%u\nupdated=%u\nusb_present=%d\nusb_generation=%u\nusb_pcm_generation=%u\nusb_ready=%d\nusb_pcm_open=%d\nusb_pcm_fd=%d\nusb_pcm_state=%d\nusb_recovering=%d\nusb_error=%d\nusb_rebuilds=%u\n",getpid(),pc_workmode(),PC_BYTE(0x83a75a),pc_route(),pc_switches,pc_result,pc_last_volume,pc_volume_active?pc_internal_volume:PC_BYTE(pc_workmode()==1?pc_native_volume_slot():0x83a7a0),pc_usb_volume,pc_volume_active?"usb":"internal",pc_volume_updates,sink?PC_WORD(sink+0x18):0,sink?PC_WORD(sink+0x1c):0,PC_BYTE(0x83a764),player?PC_WORD(player+0x48):0,pc_seconds(),pc_usb_present,pc_usb_generation,pc_usb_pcm_generation,pc_usb_ready,pc_usb_real_open,pc_usb_fd,pc_usb_pcm_state,pc_usb_recovering,pc_usb_error,pc_usb_rebuilds);n+=snprintf(b+n,sizeof b-(unsigned)n,"usb_owned=%d\nusb_card=%d\nusb_device=%u\nsample_rate=%u\nsample_bits=%u\nmanual_token=%u\nmanual_result=%d\n",pc_usb_owned(),pc_usb_card,PC_WORD(0x83dfe4)?PC_WORD(PC_WORD(0x83dfe4)+3*0x54+0x64):0,player?PC_WORD(player+0x1c):0,player?PC_WORD(player+0x24):0,pu_usb_token,pu_usb_result);int f=open("/tmp/disc-runtime/player-usb.state.new",1|0x100|0x200,0600);if(f>=0){int ok=n>0&&n<(int)sizeof b&&write(f,b,(unsigned)n)==n;close(f);if(ok)rename("/tmp/disc-runtime/player-usb.state.new","/tmp/disc-runtime/player-usb.state");}}
static int pc_output_locked(void *request){
 int dlna_result;if(dlna_output_request(request,&dlna_result))return dlna_result;
 unsigned output=request?((unsigned *)request)[4]:0;
 if(!request||!pc_enabled()||!pc_owned_request()||(pc_workmode()!=1&&pc_workmode()!=2)||(output!=3&&output!=4&&output!=6))return PC_FN(0x4f1294,int (*)(void *))(request);
 unsigned player=PC_WORD(0x83dfc4);if(!player){pc_result=-2;pc_status();return -1;}
 if(output==3){
  pc_switching=1;PC_FN(0x4e8b94,void (*)(unsigned))(1);PC_FN(0x4536d0,void (*)(unsigned))(3);pc_usb_minimum();PC_BYTE(0x83a75a)=3;
  if(!pc_usb_was_owned||!pc_usb_need_rebind){pc_usb_need_rebind=1;pc_usb_attempts=pc_usb_retry_at=0;pc_usb_bound_generation=pc_usb_pcm_generation=0;}
  pc_usb_ready=0;pc_usb_recovering=1;pc_usb_error=0;pc_result=1;pc_switching=0;pc_status();return 0;
 }
 pc_switching=1;unsigned old_dsd=PC_WORD(player+0x98),old_output=PC_BYTE(0x83a75a);
 pc_usb_retire_output();
 /* Explicit USB exit retains native DAC power/format handling, with no stale
  * entry3 left behind when native idle selection only changes route7. */
 if(output==6&&PC_BYTE(0x83a764)){PC_FN(0x4e8888,void (*)(void))();PC_BYTE(0x83a764)=0;}
 PC_FN(0x4e8b94,void (*)(unsigned))(1);PC_FN(0x4536d0,void (*)(unsigned))(2);
 pc_reconcile_route(output);int result=PC_FN(0x455c58,int (*)(unsigned))(output);
 if(result||pc_route()!=output){PC_FN(0x4536d0,void (*)(unsigned))(old_dsd);pc_result=result?result:-3;if(old_output==6)PC_FN(0x4e8b94,void (*)(unsigned))(0);pc_switching=0;pc_status();return pc_result;}
 PC_BYTE(0x83a75a)=output;pc_volume_leave();PC_FN(0x44dc80,void (*)(float))(0.0f);pc_usb_guarded=0;
 if(output==6){unsigned rate=PC_WORD(player+0x1c),bits=PC_WORD(player+0x24);if(rate>=8000&&rate<=384000&&(bits==16||bits==24||bits==32))PC_FN(0x4e84f0,void (*)(unsigned,unsigned,unsigned))(rate,bits,0);PC_FN(0x4eae58,void (*)(void))();PC_FN(0x4e8b94,void (*)(unsigned))(0);}
 else PC_FN(0x4eae58,void (*)(void))();
 pc_switches++;pc_result=0;pc_switching=0;pc_status();return 0;
}
static int pc_output(void *request){pthread_mutex_lock(pc_usb_mutex);int result=pc_output_locked(request);pthread_mutex_unlock(pc_usb_mutex);return result;}
static void pc_volume_tick(void){
 if(pthread_mutex_trylock(pc_usb_mutex))return;
 unsigned t=pc_seconds();if(!pc_switching){
  if(pc_volume_active&&pc_volume_mode!=pc_workmode())pc_volume_leave();
  int owned=pc_usb_owned();if(owned)pc_volume_enter();
  pu_usb_request_poll(owned);
  pc_usb_runtime_tick(owned);
  if(!owned&&pc_volume_active){pc_volume_leave();if(pc_route()==6){PC_FN(0x44dc80,void (*)(float))(0.0f);PC_FN(0x4eae58,void (*)(void))();}}
  if(pc_volume_dirty&&t-pc_volume_changed>=2)pc_save_usb();
 }
 if(t!=pc_last_report){pc_last_report=t;pc_status();}
 pthread_mutex_unlock(pc_usb_mutex);
}
#include "../mobile-remote/native_eq_player.h"
__attribute__((noinline,visibility("hidden"))) static int pc_usleep(unsigned usec){unsigned caller=(unsigned)__builtin_return_address(0);int result=pc_real_usleep(usec);if(caller==0x4e5b9c&&usec==100000){dlna_transport_poll();if(pc_transport_tick)pc_transport_tick();dsb_host_poll();dsde_tick();d2p_tick();dop_tick();ram_host_poll();dsp_tick();vs_player_tick();rg_tick();remote_eq_player_tick();pc_volume_tick();if(pc_cover_tick)pc_cover_tick();}return result;}
static void pc_install(void){
 for(unsigned i=0;i<sizeof(pc_guards)/sizeof(pc_guards[0]);i++)if(memcmp((const void *)pc_guards[i].address,pc_guards[i].bytes,16))return;
 if(PC_WORD(0x83a5c8)!=0x4f1294)return;
 pc_real_usleep=(int (*)(unsigned))dlsym((void *)0,"usleep");if(!pc_real_usleep)return;
 pc_usb_pcm_close=(int (*)(void *))dlsym((void *)0,"pcm_close");pc_usb_pcm_is_ready=(int (*)(void *))dlsym((void *)0,"pcm_is_ready");
 pc_mixer_db=(int (*)(void *,long,int))dlsym((void *)0,"snd_mixer_selem_set_playback_dB_all");pc_mixer_switch=(int (*)(void *,int))dlsym((void *)0,"snd_mixer_selem_set_playback_switch_all");pc_mixer_set=(int (*)(void *,long))dlsym((void *)0,"snd_mixer_selem_set_playback_volume_all");
 if(!dlna_lifecycle_install())return;
 if(!dlna_transport_install())return;
 if(!lhdcv5_sink_install())return;
 PC_WORD(PLAYER_USLEEP_GOT)=(unsigned)pc_usleep;PC_WORD(0x83a5c8)=(unsigned)pc_output;dsb_host_install();dsde_install();d2p_install();dop_install();rg_install();pc_status();
 // Publish the verified process itself; manager readiness may lag a UI upgrade.
 char pidtext[32];int n=snprintf(pidtext,sizeof(pidtext),"%d\n",getpid());int f=open("/tmp/disc-runtime/bt-player.new",1|0x100|0x200,0600);
 if(f>=0){int ok=write(f,pidtext,n)==n;close(f);if(ok)rename("/tmp/disc-runtime/bt-player.new","/tmp/disc-runtime/bt-player");}
}
