#include "disc_lhdcv5.h"
#include "lhdc_dec.h"
#include "lhdc_imdct.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define MAX_FRAME 1538u
#define MAX_PACKET 8192u
struct disc_lhdcv5 {
 void *workspace;
 lhdc_decoder_t *core;
 disc_lhdcv5_stats stats;
 uint8_t pending[MAX_FRAME];
 size_t count;
 uint16_t sequence;
 uint32_t ssrc;
 int synced;
 int32_t pcm[1920];
 int16_t pcm16[1920];
};
static volatile int stream_taken;
extern disc_lhdcv5_quality_stats disc_core_quality;
void disc_lhdcv5_get_quality(disc_lhdcv5_quality_stats *out){if(out)*out=disc_core_quality;}
static unsigned be16(const uint8_t *p){return (unsigned)p[0]*256+p[1];}
static unsigned le16(const uint8_t *p){return (unsigned)p[1]*256+p[0];}
static uint32_t be32(const uint8_t *p){return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
int disc_lhdcv5_parse_config(const uint8_t *p,size_t size,disc_lhdcv5_config *out){
 static const uint8_t id[6]={0x3a,5,0,0,0x35,0x4c};
 if(!p||!out||size!=11||memcmp(p,id,6)){errno=EINVAL;return -1;}
 /* P6..P10 are the AOSP15 layout used in the user's real captures. Reject
  * untested frame durations/versions and capability masks as SetConfiguration. */
 unsigned rate=p[6]&0x35,bits=p[7]&6;
 if((p[6]&0xc0)||(p[8]&15)!=1){errno=ENOTSUP;return -1;}
 out->rate=rate==0x20?44100:rate==0x10?48000:rate==4?96000:rate==1?192000:0;
 out->bits=bits==4?16:bits==2?24:0;out->frame_ms=5;
 if(!out->rate||!out->bits){errno=EINVAL;return -1;}return 0;
}
disc_lhdcv5 *disc_lhdcv5_open(const uint8_t *p,size_t size){
 disc_lhdcv5_config cfg;if(disc_lhdcv5_parse_config(p,size,&cfg))return NULL;
 if(!__sync_bool_compare_and_swap(&stream_taken,0,1)){errno=EBUSY;return NULL;}
 memset(&disc_core_quality,0,sizeof(disc_core_quality));
 disc_lhdcv5 *s=calloc(1,sizeof(*s));
 if(!s){__sync_lock_release(&stream_taken);return NULL;}
 s->workspace=calloc(1,lhdc_dec_get_workspace_size(cfg.rate,5));
 lhdc_dec_config_t c={.sample_rate=cfg.rate,.bit_depth=cfg.bits,.frame_duration=5,.channels=2,.max_frame_bytes=1536};
 if(s->workspace)s->core=lhdc_dec_init(s->workspace,&c);
 if(!s->core){disc_lhdcv5_close(s);errno=ENOMEM;return NULL;}
 s->stats.rate=cfg.rate;s->stats.source_bits=cfg.bits;return s;
}
void disc_lhdcv5_close(disc_lhdcv5 *s){
 if(!s)return;
 if(s->core)lhdc_dec_deinit(s->core);
 lhdc_dec_free_window();lhdc_dec_free_slot_scratch();
 lhdc_imdct_free_480();lhdc_imdct_free_960();lhdc_imdct_free_1920();lhdc_imdct_free_cos();
 free(s->workspace);free(s);__sync_lock_release(&stream_taken);
}
void disc_lhdcv5_flush(disc_lhdcv5 *s){if(s){s->count=0;s->synced=0;lhdc_dec_flush(s->core);}}
static int malformed(disc_lhdcv5 *s){s->stats.malformed++;disc_lhdcv5_flush(s);errno=EPROTO;return -1;}
int disc_lhdcv5_packet(disc_lhdcv5 *s,const uint8_t *p,size_t size,disc_lhdcv5_pcm sink,void *user){
 if(!s||!p||!sink){errno=EINVAL;return -1;}
 s->stats.packets++;
 if(size<14||size>MAX_PACKET||p[0]>>6!=2||(p[1]&127)!=96)return malformed(s);
 size_t start=12+4*(p[0]&15),end=size;
 if(start>end)return malformed(s);
 if(p[0]&16){if(end-start<4)return malformed(s);size_t extra=4+4*be16(p+start+2);if(extra>end-start)return malformed(s);start+=extra;}
 if(p[0]&32){unsigned pad=p[size-1];if(!pad||pad>end-start)return malformed(s);end-=pad;}
 if(end-start<2)return malformed(s);
 uint16_t seq=(uint16_t)be16(p+2);uint32_t ssrc=be32(p+8);
 if(s->synced&&ssrc==s->ssrc){
  uint16_t delta=(uint16_t)(seq-s->sequence);
  if(delta==0||delta>=0x8000){s->stats.duplicates++;return 0;}
  if(delta!=1){s->stats.gaps+=delta-1;s->count=0;lhdc_dec_flush(s->core);}
 }else if(s->synced){s->count=0;lhdc_dec_flush(s->core);}
 s->synced=1;s->sequence=seq;s->ssrc=ssrc;start+=2;
 /* Do not trust a guessed frame-count/fragment bitfield. Real streams carry
  * self-delimiting frames; validated incomplete frames can span packet records.
  * The pending buffer is fixed-sized, cleared on any sequence discontinuity. */
 int frames=0;
 while(start<end){
  size_t need=s->count<2?2:2+2*(le16(s->pending)&1023);
  if(need<4&&s->count>=2)return malformed(s);
  if(need>sizeof(s->pending))return malformed(s);
  size_t take=need-s->count;if(take>end-start)take=end-start;
  memcpy(s->pending+s->count,p+start,take);s->count+=take;start+=take;
  if(s->count<2)continue;
  need=2+2*(le16(s->pending)&1023);
  if(need<4||need>sizeof(s->pending))return malformed(s);
  if(s->count<need)continue;
  size_t used=0;uint32_t generated=0;
  void *decoded=s->stats.source_bits==16?(void *)s->pcm16:(void *)s->pcm;
  int ret=lhdc_dec_decode_frame(s->core,s->pending,s->count,decoded,960,&used,&generated,NULL);
  s->count=0;
  if(ret||used!=need||generated>960){s->stats.decode_errors++;lhdc_dec_flush(s->core);errno=EILSEQ;return -1;}
  if(s->stats.source_bits==16)for(size_t i=0;i<generated*2;i++)s->pcm[i]=(int32_t)s->pcm16[i]*65536;
  s->stats.frames++;s->stats.pcm_frames+=generated;frames++;
  if(sink(user,s->pcm,generated)){disc_lhdcv5_flush(s);errno=EIO;return -1;}
 }
 return frames;
}
void disc_lhdcv5_get_stats(disc_lhdcv5 *s,disc_lhdcv5_stats *out){
 if(!s||!out)return;*out=s->stats;out->pending_bytes=s->count;
 lhdc_dec_latency_t timing;lhdc_dec_latency_stats(&timing,0);
 out->avg_decode_us=timing.avg_us;out->max_decode_us=timing.max_us;out->over_budget=timing.over;
}
