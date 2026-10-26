/* Observe final tinyalsa writes: RAM only, no audio mutation or file I/O. */
static int (*dop_real_pcm_write)(void *,const void *,unsigned);
static int (*dop_real_pcm_read)(void *,void *,unsigned);
#ifndef DISC_DOP_TRACE_TEST
#include "dop_recover.h"
#else
static int dop_recovery_write(void *pcm,const void *data,unsigned bytes,int guard){(void)guard;return dop_real_pcm_write(pcm,data,bytes);}
#endif
static volatile unsigned dop_trace_enabled;
static unsigned dop_guard_dropped;
static unsigned dop_packet_bad(const unsigned char *b,unsigned bytes){
 if(bytes<8||bytes%8)return bytes?bytes-1:0;
 unsigned first=b[3];if(first!=5&&first!=250)return 0;
 for(unsigned i=0;i<bytes;i+=8){unsigned expected=(i/8)&1?first^255:first;if(b[i+3]!=expected||b[i+7]!=expected)return i;}
 return bytes;
}
static unsigned dop_trace_calls,dop_trace_errors,dop_trace_bad,dop_trace_phase_errors;
static unsigned dop_trace_last_marker,dop_trace_last_rate,dop_trace_last_class,dop_trace_have;
static unsigned dop_trace_first_bad_offset,dop_trace_last_bytes,dop_trace_gap_max,dop_trace_last_time;
static int dop_trace_last_result;
static unsigned dop_trace_short_writes,dop_trace_write_max_ms,dop_trace_written_frames;
static unsigned dop_read_calls,dop_read_errors,dop_read_max_ms;
static int dop_read_last_result;
static unsigned dop_trace_sample_ready,dop_trace_good_ready;
static unsigned char dop_trace_bad_sample[256],dop_trace_good_sample[256];
static unsigned dop_trace_time(void){long t[2];return clock_gettime(1,t)?0:(unsigned)t[0]*1000u+(unsigned)t[1]/1000000u;}
static int dop_trace_read(void *pcm,void *data,unsigned bytes){
 int observe=dop_trace_enabled&&pc_workmode()==4&&pc_route()==6;unsigned began=observe?dop_trace_time():0;
 int result=dop_real_pcm_read(pcm,data,bytes);
 if(observe){unsigned duration=dop_trace_time()-began;dop_read_calls++;dop_read_last_result=result;if(result<0)dop_read_errors++;if(duration>dop_read_max_ms)dop_read_max_ms=duration;}
 return result;
}
static void dop_trace_copy(unsigned char *to,const unsigned char *from,unsigned bytes,unsigned offset){
 if(offset+256>bytes)offset=bytes>256?bytes-256:0;
 for(unsigned i=0;i<256;i++)to[i]=offset+i<bytes?from[offset+i]:0;
}
static int dop_trace_write(void *pcm,const void *data,unsigned bytes){
 unsigned bad=bytes;int guard=data&&bytes&&dop_source()&&PC_BYTE(0x899060)==1;
 unsigned source=PC_WORD(0x83dfc4);guard=guard&&source&&PC_WORD(source+0x28)==0;
 if(guard)bad=dop_packet_bad(data,bytes);
 if(dop_trace_enabled&&pc_workmode()==4&&pc_route()==6&&data&&bytes){
  const unsigned char *b=data;unsigned p=PC_WORD(0x83dfc4),rate=p?PC_WORD(p+0xc):0,kind=p?PC_WORD(p+0x2c):0,t=dop_trace_time();
  if(dop_trace_last_time&&t-dop_trace_last_time>dop_trace_gap_max)dop_trace_gap_max=t-dop_trace_last_time;
  dop_trace_last_time=t;dop_trace_calls++;dop_trace_last_bytes=bytes;
  if(rate!=dop_trace_last_rate||kind!=dop_trace_last_class)dop_trace_have=0;
  if(kind==2&&(rate==176400||rate==352800)){
   if(!guard)bad=dop_packet_bad(b,bytes);unsigned first=bytes>=8?b[3]:0;
   if(bad<bytes){dop_trace_bad++;dop_trace_first_bad_offset=bad;if(!dop_trace_sample_ready){dop_trace_copy(dop_trace_bad_sample,b,bytes,bad);__sync_synchronize();dop_trace_sample_ready=1;}dop_trace_have=0;}
   else {
    if(dop_trace_have&&first!=(dop_trace_last_marker^255))dop_trace_phase_errors++;
    dop_trace_last_marker=b[bytes-5];dop_trace_have=1;
    if(!dop_trace_good_ready){dop_trace_copy(dop_trace_good_sample,b,bytes,0);__sync_synchronize();dop_trace_good_ready=1;}
   }
  }else dop_trace_have=0;
  dop_trace_last_rate=rate;dop_trace_last_class=kind;
 }
 /* A classified DoP stream may still have old PCM queued at the output.
  * Never send malformed packets into an active DSD DAC; consume only the
  * invalid chunk, without editing valid DSD or synthesizing a waveform. */
 if(guard&&bad<bytes){dop_guard_dropped++;return 0;}
 unsigned began=dop_trace_enabled?dop_trace_time():0;
 int result=dop_recovery_write(pcm,data,bytes,guard);
 if(dop_trace_enabled){unsigned duration=dop_trace_time()-began;if(duration>dop_trace_write_max_ms)dop_trace_write_max_ms=duration;dop_trace_last_result=result;if(result<0)dop_trace_errors++;else if(result>0&&guard){dop_trace_written_frames+=(unsigned)result;if((unsigned)result<bytes/8)dop_trace_short_writes++;}}
 return result;
}
static void dop_trace_dump(const char *path,const unsigned char *b){int f=open(path,1|0x100|0x200,0600);if(f>=0){write(f,b,256);close(f);}}
static void dop_trace_poll(void){
 dop_trace_enabled=access("/usr/data/disc-runtime/components/disc-audio-diagnostics/disabled",0)&&!access("/usr/data/disc-runtime/components/disc-audio-diagnostics/enabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
 if(!dop_trace_enabled)return;
 if(dop_trace_good_ready==1){__sync_synchronize();dop_trace_dump("/tmp/disc-runtime/dop-final-good.bin",dop_trace_good_sample);dop_trace_good_ready=2;}
 if(dop_trace_sample_ready==1){__sync_synchronize();dop_trace_dump("/tmp/disc-runtime/dop-final-bad.bin",dop_trace_bad_sample);dop_trace_sample_ready=2;}
 unsigned p=PC_WORD(0x83dfc4);char b[1024];int n=snprintf(b,sizeof(b),"point=before_tinyalsa_pcm_write\npcm_silence_calls_suppressed=%u\nlast_write_result=%d\ninvalid_chunks_discarded=%u\nwrite_calls=%u\nwrite_errors=%u\nbad_marker_chunks=%u\nphase_discontinuities=%u\nbad_offset=%u\nlast_bytes=%u\nmax_gap_ms=%u\ninput_rate=%u\noutput_rate=%u\nclass=%u\nDAC_dsd=%u\nresample_flag=%u\nplayer_state=%u\npre_output_calls=%u\npre_output_max_bytes=%u\npre_output_oversize=%u\npre_output_truncated_bytes=%u\nshort_writes=%u\nwrite_max_ms=%u\nwritten_frames=%u\ncapture_reads=%u\ncapture_errors=%u\ncapture_last_result=%d\ncapture_max_ms=%u\n",dop_quiet_bypasses,dop_trace_last_result,dop_guard_dropped,dop_trace_calls,dop_trace_errors,dop_trace_bad,dop_trace_phase_errors,dop_trace_first_bad_offset,dop_trace_last_bytes,dop_trace_gap_max,p?PC_WORD(p+0xc):0,p?PC_WORD(p+0x1c):0,p?PC_WORD(p+0x2c):0,PC_BYTE(0x899060),PC_WORD(0x839a78),p?PC_WORD(p+0x48):0,dop_pre_calls,dop_pre_max,dop_pre_oversize,dop_pre_truncated_bytes,dop_trace_short_writes,dop_trace_write_max_ms,dop_trace_written_frames,dop_read_calls,dop_read_errors,dop_read_last_result,dop_read_max_ms);
 int f=open("/tmp/disc-runtime/dop-final.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}
}
