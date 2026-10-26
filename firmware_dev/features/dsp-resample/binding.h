#include "resample_api.h"
#include "rules.h"
static const DiscResampleApi *dsp_src_api;
static DiscResampleProcessFloat dsp_float_process;
static void *dsp_src_context;
static unsigned dsp_processing_rate,dsp_src_config_error,dsp_src_seen,dsp_src_last_attempt_rate;
static DiscSrcRules dsp_src_rules;
static void dsp_src_poll(void){
 if(!dsp_src_seen){dsp_src_rules=disc_src_default();dsp_src_seen=1;}
 char b[128];int f=open(DISC_SRC_CONFIG,0),n=f<0?0:read(f,b,sizeof(b)-1);if(f>=0)close(f);
 if(f<0&&access(DISC_SRC_CONFIG,0)){dsp_src_rules=disc_src_default();dsp_src_config_error=0;return;}
 DiscSrcRules next;if(n<=0||n>=(int)sizeof(b)-1){dsp_src_config_error=1;dsp_src_rules=disc_src_default();return;}
 b[n]=0;if(!disc_src_parse(b,&next)){dsp_src_config_error=1;dsp_src_rules=disc_src_default();return;}
 dsp_src_rules=next;dsp_src_config_error=0;
}
/* Only the audio worker advances ramps. Control publishes transitions after
 * the zero-gain acknowledgement, so no audio callback waits for allocation. */
static volatile unsigned dsp_src_fade_mode,dsp_src_fade_epoch,dsp_src_wait;
static float dsp_src_fade_gain=1;
static void dsp_src_fade_down(void){__sync_synchronize();dsp_src_fade_mode=1;}
static void dsp_src_fade_up(unsigned epoch,unsigned wait){dsp_src_fade_gain=0;dsp_src_fade_epoch=epoch;dsp_src_wait=wait;__sync_synchronize();dsp_src_fade_mode=3;}
static void dsp_src_fade_clear(void){dsp_src_wait=0;__sync_synchronize();dsp_src_fade_mode=0;}
static void dsp_src_fade_apply(void *data,int bytes,int rate,unsigned epoch){
 unsigned mode=dsp_src_fade_mode;if(!mode){dsp_src_fade_gain=1;return;}
 unsigned p=PC_WORD(0x83dfc4);if(!p||PC_WORD(p+0x20)!=2||PC_WORD(p+0x24)!=32||!data||bytes<=0||(bytes&7)||rate<8000||rate>768000)return;
 int *pcm=data;unsigned frames=(unsigned)bytes/8;
 if(mode==2||(mode==3&&epoch!=dsp_src_fade_epoch)){memset(data,0,(unsigned)bytes);return;}
 float step=1.f/(rate*.020f);
 for(unsigned i=0;i<frames;i++){
  if(mode==1){dsp_src_fade_gain-=step;if(dsp_src_fade_gain<0)dsp_src_fade_gain=0;}
  else if(mode==3){if(dsp_src_wait)dsp_src_wait--;else {dsp_src_fade_gain+=step;if(dsp_src_fade_gain>1)dsp_src_fade_gain=1;}}
  for(unsigned c=0;c<2;c++)pcm[i*2+c]=(int)((double)pcm[i*2+c]*dsp_src_fade_gain);
 }
 if(mode==1&&dsp_src_fade_gain==0){__sync_synchronize();dsp_src_fade_mode=2;}
 else if(mode==3&&dsp_src_fade_gain==1&&!dsp_src_wait)dsp_src_fade_mode=0;
}
typedef struct {unsigned available,latency,clipped,limited,invalid,src_errors,src_underflows;float peak,gain;} DiscSrcSnapshot;
static volatile unsigned dsp_src_snapshot_sequence;
static DiscSrcSnapshot dsp_src_snapshot;
static void dsp_src_capture_stats(void){
 DiscSrcSnapshot next={0};next.gain=1;dsp_api->stats(dsp_context,&next.available,&next.latency,&next.clipped);
 dsp_api->diagnostics(dsp_context,&next.limited,&next.invalid,&next.peak,&next.gain);
 unsigned latency=0,invalid=0;dsp_src_api->stats(dsp_src_context,&latency,&next.src_errors,&next.src_underflows,&invalid);next.invalid+=invalid;next.latency=latency;
 unsigned limited;float peak,gain;dsp_src_api->diagnostics(dsp_src_context,&limited,&peak,&gain);next.limited+=limited;next.peak=peak;next.gain=gain;
 dsp_src_snapshot_sequence++;__sync_synchronize();dsp_src_snapshot=next;__sync_synchronize();dsp_src_snapshot_sequence++;
}
static int dsp_src_read_stats(DiscSrcSnapshot *out){unsigned sequence=dsp_src_snapshot_sequence;if(!sequence||(sequence&1))return 0;__sync_synchronize();*out=dsp_src_snapshot;__sync_synchronize();return sequence==dsp_src_snapshot_sequence;}
