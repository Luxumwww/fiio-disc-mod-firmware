/* Diagnostic source substitution and explicitly armed receiver format test.
 * No PCM open/close/prepare, threshold changes, payload format conversion,
 * TF logger or DMA replacement. No DAC changes unless format test armed. */
#ifdef DISC_RAM_TEST
#include <stddef.h>
#else
typedef unsigned size_t;
#endif
extern int open(const char *,int,...),close(int),read(int,void *,size_t),write(int,const void *,size_t),access(const char *,int),unlink(const char *),getpid(void),usleep(unsigned),clock_gettime(int,void *),snprintf(char *,size_t,const char *,...),prctl(int,...);
extern void *malloc(size_t),*dlsym(void *,const char *);extern void free(void *);
extern int strcmp(const char *,const char *);
extern int pthread_mutex_lock(void *),pthread_mutex_unlock(void *),pthread_create(void *,const void *,void *(*)(void *),void *),pthread_detach(unsigned long);
extern void *mmap(void *,size_t,int,int,int,long);
extern int munmap(void *,size_t),mprotect(void *,size_t,int),cacheflush(void *,size_t,int);
#define W(a) (*(volatile unsigned *)(unsigned long)(a))
#define B(a) (*(volatile unsigned char *)(unsigned long)(a))
#include "packet.h"
enum {IDLE,WAIT_DSD,BASELINE,REPLAY,DONE,FAILED};
static volatile unsigned phase,enabled,installed;
static unsigned char *clip;
static unsigned clip_bytes,clip_rate,position,marker=5;
static unsigned start_at,wait_deadline,baseline_seconds,replay_seconds,baseline_calls,baseline_frames,frames,write_calls,errors,partial,source_suppressed,config_losses,last_error,reported;
static unsigned got;
static unsigned board_checks;
static unsigned last_available,last_stamp_result,last_cached_state,max_available;
static unsigned char board[2][8];
static unsigned char buffer[4096];
static int (*native_write)(void *,const void *,unsigned),(*stamp)(void *,unsigned *,long *),(*native_ioctl)(int,unsigned,void *);
static const unsigned *(*config)(void *);
static int (*stock_output)(void *,int,int)=(int (*)(void *,int,int))(unsigned long)0x465718;
static unsigned seconds(void){long t[2];return clock_gettime(1,t)?0:(unsigned)t[0];}
static unsigned scope(unsigned hz){
 unsigned p=W(0x83dfc4),mode=W(0x83dfc0),sink=W(0x83dfe4);
 return p&&mode&&W(mode)==4&&sink&&W(sink+0x58)==6&&B(0x83a75a)==6&&W(0x839a74)==1&&
  W(p+0xc)==hz&&W(p+0x10)==2&&W(p+0x14)==32&&W(p+0x18)==0&&W(p+0x2c)==2&&W(p+0x58)==1&&
  W(p+0x1c)==hz&&W(p+0x20)==2&&W(p+0x24)==32&&B(0x899060)==1&&W(0x899084)==hz;
}
static void *current_pcm(unsigned sink){return (void *)(unsigned long)W(sink+6*0x54+0x74);}
static int layout(void *pcm){
 if(!pcm)return 0;const unsigned *c=config(pcm);
 return c==(const unsigned *)((unsigned char *)pcm+0x98)&&(int)W((unsigned long)pcm)>=0&&
  !(W((unsigned long)pcm+4)&0x10000003u)&&W((unsigned long)pcm+0xb8)&&c[0]==2&&c[1]==clip_rate&&c[4]==7&&c[2]&&c[3]>=2;
}
static unsigned pcm_state(void *pcm){return W(W((unsigned long)pcm+0xb8));}
#include "output_capture.h"
#include "control_trace.h"
#include "format_profile.h"
#include "sync_probe.h"
static unsigned sync_attempts,sync_completed;
static int sync_result;
static struct SyncRecord sync_record;
static void sync_probe_poll(void){
 static unsigned checked;unsigned now=seconds();if(checked==now)return;checked=now;
 if(access("/tmp/disc-runtime/dop-sync-only-start",0))return;
 unlink("/tmp/disc-runtime/dop-sync-only-start");sync_attempts++;
 unsigned p=W(0x83dfc4),sink=W(0x83dfe4),hz=p?W(p+0xc):0;
 if(!enabled||phase==REPLAY||phase==BASELINE||phase==WAIT_DSD||capture_active||control_active||!sink||
    (hz!=176400&&hz!=352800)||!scope(hz)){sync_result=6;return;}
 pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));
 void *pcm=current_pcm(sink);const unsigned *c=pcm?config(pcm):0;
 if(!scope(hz)||!pcm||c!=(const unsigned *)((unsigned char *)pcm+0x98)||
    (int)W((unsigned long)pcm)<0||W((unsigned long)pcm+4)||c[0]!=2||c[1]!=hz||c[4]!=7)sync_result=7;
 else sync_result=sync_read_pair((int)W((unsigned long)pcm),native_ioctl,&sync_record);
 pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));
 if(!sync_result)sync_completed++;
}
#include "split_write.h"
static unsigned split_selected,split_bypassed;
static unsigned split_scope(void *pcm,const void *data,unsigned bytes){
 if(!enabled||split_stats.fault||phase==WAIT_DSD||phase==BASELINE||phase==REPLAY||
    !data||!bytes||(bytes&7)||bytes>262144)return 0;
 unsigned p=W(0x83dfc4),sink=W(0x83dfe4),hz=p?W(p+0xc):0;
 if((hz!=176400&&hz!=352800)||!scope(hz)||!sink||pcm!=current_pcm(sink))return 0;
 const unsigned *c=config(pcm);
 /* Prepare/autorecovery has ambiguous zero-success semantics in this
  * exact stock tinyalsa. Split only an already prepared/running stream. */
 return c==(const unsigned *)((unsigned char *)pcm+0x98)&&
  (int)W((unsigned long)pcm)>=0&&!(W((unsigned long)pcm+4)&0x10000003u)&&
  (W((unsigned long)pcm+8)&3)==3&&c[0]==2&&c[1]==hz&&c[4]==7&&c[2]&&c[3]>=2;
}
static int wrapped_write(void *pcm,const void *data,unsigned bytes){
 if(phase==REPLAY&&scope(clip_rate)){
  unsigned sink=W(0x83dfe4);
  if(sink&&pcm==current_pcm(sink)){source_suppressed++;return 0;}
 }
 int observation=capture_begin(pcm,data,bytes);
 int rc;
 if(split_scope(pcm,data,bytes)){split_selected++;rc=split_write_block(pcm,data,bytes,native_write);}
 else{split_bypassed++;rc=native_write(pcm,data,bytes);}
 capture_end(observation,pcm,rc);
 if(phase==BASELINE&&scope(clip_rate)&&data&&bytes>=8&&(bytes&7)==0){
  const unsigned char *p=data;unsigned last=p[bytes-5];
  if((last==5||last==250)&&p[bytes-1]==last){marker=last^255;baseline_calls++;baseline_frames+=bytes/8;}
 }
 return rc;
}
static void snapshot(const char *name){
 char b[768];int f=open("/proc/asound/card0/pcm3p/sub0/status",0),n=f<0?0:read(f,b,sizeof(b));if(f>=0)close(f);
 if(n>0){f=open(name,1|0x100|0x200,0600);if(f>=0){write(f,b,(unsigned)n);close(f);}}
}
struct WriteI {int result;const void *data;unsigned frames;};
static void fail(unsigned code){last_error=code;errors++;__sync_synchronize();phase=FAILED;snapshot("/tmp/disc-runtime/ram-dop-after.state");}
static int load_clip(void);
struct Message {unsigned short address,flags,length;unsigned char *data;};
struct Transfer {struct Message *messages;unsigned count;};
static int read_board(void){
 const char *paths[2]={"/sys/bus/i2c/devices/3-0030/name","/sys/bus/i2c/devices/3-0032/name"};
 const char *names[2]={"cs43131\n","cs43131c\n"};
 for(unsigned i=0;i<2;i++){char b[24];int f=open(paths[i],0),n=f<0?0:read(f,b,23);if(f>=0)close(f);if(n<=0)return 0;b[n]=0;if(strcmp(b,names[i]))return 0;}
 int fd=open("/dev/i2c-3",2);if(fd<0)return 0;int ok=1;
 const unsigned regs[8]={0x10000,0x10001,0x10002,0x1000b,0x70004,0x40014,0x40016,0x20000};
 for(unsigned c=0;c<2&&ok;c++)for(unsigned i=0;i<8&&ok;i++){
  unsigned r=regs[i];unsigned char address[4]={r>>16,r>>8,r,0};
  struct Message m[2]={{0x30+2*c,0,4,address},{0x30+2*c,1,1,&board[c][i]}};struct Transfer t={m,2};
  ok=native_ioctl(fd,0x707,&t)==2;
 }
 close(fd);board_checks++;
 for(unsigned c=0;c<2&&ok;c++)ok=board[c][0]==0x43&&board[c][1]==0x13&&board[c][2]==0x10&&
  (board[c][3]&15)==7&&(board[c][4]&0x7c)==0x54&&!(board[c][7]&0x40);
 return ok;
}
static void send_step(unsigned sink){
 pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));void *pcm=current_pcm(sink);
 if(phase==REPLAY){
  if(!scope(clip_rate)||!layout(pcm)){config_losses++;fail(5);}
  else{
   unsigned avail=0;long ts[2]={0};
   int rc=stamp(pcm,&avail,ts); /* SYNC_PTR refresh before reading cached state. */
   last_available=avail;last_stamp_result=(unsigned)rc;last_cached_state=pcm_state(pcm);if(avail>max_available)max_available=avail;
   if(pcm_state(pcm)!=3){config_losses++;fail(5);}
   else if(rc<0||avail>W((unsigned long)pcm+0x10))fail(6);
   else if(avail){
    unsigned count=avail>512?512:avail,remaining=clip_bytes/8-frames;if(count>remaining)count=remaining;
    ram_packet_copy(buffer,clip,clip_bytes,position,count,marker);
    struct WriteI io={0,buffer,count};int rc=native_ioctl((int)W((unsigned long)pcm),0x800c4150u,&io);write_calls++;
    if(rc<0||io.result<0||(unsigned)io.result>count)fail(7);
    else if(io.result){unsigned n=(unsigned)io.result;if(n<count)partial++;
     position+=n*8;if(n&1)marker^=255;frames+=n;
    }
   }
  }
 }
 pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));
}
static void *worker(void *unused){
 (void)unused;prctl(15,"disc-ram-dop",0,0,0);
 if(!load_clip()){fail(11);return 0;}
 for(;;){
  capture_flush();
  control_flush();
  format_poll();
  sync_probe_poll();
  unsigned before_frames=frames;
  unsigned now=seconds();
  if(phase==WAIT_DSD){
   if(now>=wait_deadline)fail(1);
   else if(scope(clip_rate)){if(!read_board())fail(10);else{start_at=now;phase=BASELINE;}}
  }else if(phase==BASELINE){
   if(!scope(clip_rate))fail(2);
   else if(now-start_at>=baseline_seconds){
    unsigned sink=W(0x83dfe4);pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));
    void *pcm=current_pcm(sink);
    unsigned avail=0;long ts[2]={0};
    if(!scope(clip_rate)||!layout(pcm)||stamp(pcm,&avail,ts)<0||pcm_state(pcm)!=3||!baseline_calls)fail(3);
    else{snapshot("/tmp/disc-runtime/ram-dop-before.state");position=0;start_at=seconds();__sync_synchronize();phase=REPLAY;}
    pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));
   }
  }else if(phase==REPLAY){
   unsigned sink=W(0x83dfe4);
   if(!scope(clip_rate)||!sink){config_losses++;fail(4);}
   else if(frames==clip_bytes/8){snapshot("/tmp/disc-runtime/ram-dop-after.state");phase=DONE;}
   else if(now-start_at>=replay_seconds+3){snapshot("/tmp/disc-runtime/ram-dop-after.state");fail(9);}
   else{
    send_step(sink);
   }
  }
  /* Fill measured available space promptly; only sleep when the buffer is
   * full. A fixed delay after every small write can itself cause underrun. */
  if(phase!=REPLAY)usleep(20000);else if(frames==before_frames)usleep(1000);
 }
 return 0;
}
static int load_clip(void){
 const char *path="/usr/data/disc-runtime/module-data/disc-usbdac-ram-test/clip.bin";
 int f=open(path,0);unsigned h[8];if(f<0)return 0;
 if(read(f,h,sizeof(h))!=(int)sizeof(h)||h[0]!=0x314d4152||h[1]!=352800||h[2]!=14112000){close(f);return 0;}
 unsigned char *b=malloc(h[2]);if(!b){close(f);return 0;}unsigned done=0;
 while(done<h[2]){int n=read(f,b+done,h[2]-done);if(n<=0){free(b);close(f);return 0;}done+=(unsigned)n;}
 unsigned char extra;int trailing=read(f,&extra,1);close(f);
 if(trailing!=0||ram_packet_hash(b,done)!=h[3]||!ram_packet_valid(b,done)){free(b);return 0;}
 clip=b;clip_bytes=done;clip_rate=h[1];return 1;
}
static int output_filter(void *data,int bytes,int quiet){
 if(phase==REPLAY&&scope(clip_rate)){source_suppressed++;usleep(5000);return 0;}
 return stock_output(data,bytes,quiet);
}
static int bind_output_calls(void){
 const unsigned sites[7]={0x465970,0x465af8,0x465d64,0x465ec0,0x466338,0x466420,0x466980};
 const unsigned delay[7]={0x34058000,0x02202025,0x34058000,0x00402825,0xafa20040,0x34058000,0x00402825};
 for(unsigned i=0;i<7;i++)if(W(sites[i])!=0x0c1195c6u||W(sites[i]+4)!=delay[i])return 0;
 unsigned *stub=mmap((void *)0x07300000,4096,3,0x802,-1,0);
 unsigned address=(unsigned)(unsigned long)stub,target=(unsigned)(unsigned long)output_filter;
 if(address<0x10000||address>=0x10000000u||(address&4095)){if(stub!=(void *)-1)munmap(stub,4096);return 0;}
 stub[0]=0x3c190000u|(target>>16);stub[1]=0x37390000u|(target&65535);stub[2]=0x03200008u;stub[3]=0;
 if(cacheflush(stub,16,3)||mprotect(stub,4096,5)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x465000,8192,7)){munmap(stub,4096);return 0;}
 for(unsigned i=0;i<7;i++)W(sites[i])=0x0c000000u|(address>>2);
 __sync_synchronize();int rc=cacheflush((void *)0x465000,8192,3);
 if(rc){for(unsigned i=0;i<7;i++)W(sites[i])=0x0c1195c6u;cacheflush((void *)0x465000,8192,3);}
 int protected=mprotect((void *)0x465000,8192,5);return !rc&&!protected;
}
int disc_ram_dop_install(unsigned write_got,unsigned ioctl_got){
 if(installed)return 1;
 if(!access("/usr/data/disc-runtime/components/disc-usbdac-ram-test/disabled",0))return 0;
 const char *off[]={"disc-usbdac-dsd-backend","disc-usbdac-dsd-engine","disc-usbdac-d2p"};
 for(unsigned i=0;i<3;i++){char b[160];snprintf(b,sizeof(b),"/usr/data/disc-runtime/components/%s/disabled",off[i]);if(access(b,0))return 0;}
 if(access("/usr/data/disc-runtime/components/disc-usbdac-dop/disabled",0)){
  char b[32];int f=open("/usr/data/disc-runtime/components/disc-usbdac-dop/current/FEATURES",0),n=f<0?0:read(f,b,31);if(f>=0)close(f);if(n<0)n=0;b[n]=0;
  if(strcmp(b,"usbdac-stock-abi1\n"))return 0;
 }
 native_write=dlsym((void *)0,"pcm_write");stamp=dlsym((void *)0,"pcm_get_htimestamp");config=dlsym((void *)0,"pcm_get_config");native_ioctl=dlsym((void *)0,"ioctl");
 if(!native_write||!stamp||!config||!native_ioctl||W(write_got)!=(unsigned)(unsigned long)native_write||W(ioctl_got)!=(unsigned)(unsigned long)native_ioctl)return 0;
 if(!bind_output_calls())return 0;
 got=write_got;unsigned long thread=0;
 if(pthread_create(&thread,0,worker,0))return 0;pthread_detach(thread);
 W(got)=(unsigned)(unsigned long)wrapped_write;W(ioctl_got)=(unsigned)(unsigned long)observed_ioctl;__sync_synchronize();enabled=installed=1;return 1;
}
void disc_ram_dop_tick(void){
 if(!installed)return;
 enabled=access("/usr/data/disc-runtime/components/disc-usbdac-ram-test/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
 capture_poll();
 control_poll();
 unsigned now=seconds();
 if(!enabled){if(phase==REPLAY||phase==BASELINE||phase==WAIT_DSD)fail(8);}
 if(!access("/tmp/disc-runtime/ram-dop-stop",0)){unlink("/tmp/disc-runtime/ram-dop-stop");phase=IDLE;}
 if(enabled&&clip&&phase!=REPLAY&&phase!=BASELINE&&!access("/tmp/disc-runtime/ram-dop-start",0)){
  unlink("/tmp/disc-runtime/ram-dop-start");baseline_calls=baseline_frames=frames=write_calls=errors=partial=source_suppressed=config_losses=last_error=0;last_available=last_stamp_result=last_cached_state=max_available=0;
  baseline_seconds=1;replay_seconds=5;wait_deadline=now+60;phase=WAIT_DSD;
 }
 if(now==reported)return;reported=now;char b[3072];
 int n=snprintf(b,sizeof(b),"version=1.0.7\npid=%d\ninstalled=%u\nenabled=%u\nphase=%u\nrate=%u\nclip_bytes=%u\nbaseline_seconds=%u\nreplay_seconds=%u\nbaseline_calls=%u\nbaseline_frames=%u\nreplay_frames=%u\nwrite_calls=%u\npartial_writes=%u\nerrors=%u\nerror_code=%u\nconfig_losses=%u\nsource_writes_suppressed=%u\nDSD_conversion=0\nDAC_writes=%u\nPCM_open_close_prepare=0\n",getpid(),installed,enabled,phase,clip_rate,clip_bytes,baseline_seconds,replay_seconds,baseline_calls,baseline_frames,frames,write_calls,partial,errors,last_error,config_losses,source_suppressed,format_writes);
 n+=snprintf(b+n,sizeof(b)-(unsigned)n,"board_checks=%u\nleft_rate=%u\nright_rate=%u\nleft_DSD=%u\nright_DSD=%u\nleft_half_frame=%u\nright_half_frame=%u\nleft_frame=%u\nright_frame=%u\nrepeat=0\n",board_checks,board[0][3],board[1][3],board[0][4],board[1][4],board[0][5],board[1][5],board[0][6],board[1][6]);
 n+=snprintf(b+n,sizeof(b)-(unsigned)n,"last_available=%u\nmax_available=%u\nlast_stamp_result=%d\nlast_cached_state=%u\n",last_available,max_available,(int)last_stamp_result,last_cached_state);
 n+=snprintf(b+n,sizeof(b)-(unsigned)n,"capture_active=%u\ncapture_pending=%u\ncapture_saved=%u\ncapture_bytes=%u\ncapture_events=%u\ncapture_error=%u\n",capture_active,capture_pending,capture_saved,capture_bytes,capture_count,capture_error);
 n+=snprintf(b+n,sizeof(b)-(unsigned)n,"split_selected=%u\nsplit_bypassed=%u\nsplit_blocks=%u\nsplit_calls=%u\nsplit_frames=%u\nsplit_partials=%u\nsplit_zero_results=%u\nsplit_errors=%u\nsplit_invalid_results=%u\nsplit_incomplete_frames=%u\nsplit_max_request_frames=%u\nsplit_fault=%u\nsplit_last_result=%d\n",split_selected,split_bypassed,split_stats.blocks,split_stats.calls,split_stats.frames,split_stats.partials,split_stats.zero_results,split_stats.errors,split_stats.invalid_results,split_stats.incomplete_frames,split_stats.max_request_frames,split_stats.fault,split_stats.last_result);
 n+=snprintf(b+n,sizeof(b)-(unsigned)n,"control_observer=1\ncontrol_active=%u\ncontrol_events=%u\ncontrol_pending=%u\ncontrol_saved=%u\ncontrol_overflow=%u\ncontrol_rejected=%u\n",control_active,control_count,control_pending,control_saved,control_overflow,control_rejected);
 n+=snprintf(b+n,sizeof(b)-(unsigned)n,"format_attempts=%u\nformat_applied=%u\nformat_failed=%u\nformat_result=%d\nformat_compensated=%u\nformat_mute_result=%d\nformat_unmute_result=%d\n",format_attempts,format_applied,format_failed,format_result,format_compensated,format_mute_result,format_unmute_result);
 n+=snprintf(b+n,sizeof(b)-(unsigned)n,"sync_attempts=%u\nsync_completed=%u\nsync_result=%d\nsync_before_state=%u\nsync_after_state=%u\nsync_before_hw=%u\nsync_after_hw=%u\nsync_before_app=%u\nsync_after_app=%u\nsync_before_min=%u\nsync_after_min=%u\n",sync_attempts,sync_completed,sync_result,sync_record.before_state,sync_record.after_state,sync_record.before_hw,sync_record.after_hw,sync_record.before_app,sync_record.after_app,sync_record.before_min,sync_record.after_min);
 int f=open("/tmp/disc-runtime/ram-dop.state",1|0x100|0x200,0600);if(f>=0){write(f,b,(unsigned)n);close(f);}
}
