#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "resample_api.h"
#include "third_party/speexdsp/include/speex/speex_resampler.h"
#include "../audio-dsp/output_guard.h"
#define DISC_SRC_CHUNK 1024u
typedef struct {
 unsigned input_rate,processing_rate,ratio,latency,errors,underflows;
 SpeexResamplerState *down,*up;
 OutputGuard guard;
 float in[DISC_SRC_CHUNK*2],low[DISC_SRC_CHUNK*2],out[(DISC_SRC_CHUNK+32)*2];
 float pending[32][2];unsigned pending_count;
} DiscSrc;
static void src_destroy(void *context){DiscSrc *s=context;if(!s)return;if(s->down)speex_resampler_destroy(s->down);if(s->up)speex_resampler_destroy(s->up);free(s);}
static void *src_create(unsigned input,unsigned processing){
 if(input>384000||processing<44100||input%processing)return 0;
 unsigned ratio=input/processing;if(ratio!=2&&ratio!=4&&ratio!=8)return 0;
 if((input%48000==0)!=(processing%48000==0))return 0;
 DiscSrc *s=calloc(1,sizeof(*s));if(!s)return 0;int error=0;
 s->input_rate=input;s->processing_rate=processing;s->ratio=ratio;
 s->down=speex_resampler_init(2,input,processing,10,&error);if(!s->down||error){src_destroy(s);return 0;}
 s->up=speex_resampler_init(2,processing,input,10,&error);if(!s->up||error){src_destroy(s);return 0;}
 output_guard_init(&s->guard,input);
 s->latency=speex_resampler_get_input_latency(s->down)+speex_resampler_get_output_latency(s->up)+s->guard.delay;
 return s;
}
static int src_process(void *context,int *pcm,unsigned frames,DiscResampleProcessFloat process,void *dsp){
 DiscSrc *s=context;if(!s||!pcm||!process||frames>1048576)return 0;
 unsigned pos=0;
 while(pos<frames){
  unsigned n=frames-pos;if(n>DISC_SRC_CHUNK)n=DISC_SRC_CHUNK;
  for(unsigned i=0;i<n*2;i++)s->in[i]=pcm[pos*2+i]*(1.f/2147483648.f);
  spx_uint32_t consumed=n,low_frames=DISC_SRC_CHUNK;
  if(speex_resampler_process_interleaved_float(s->down,s->in,&consumed,s->low,&low_frames)||consumed!=n){s->errors++;return 0;}
  if(low_frames&&!process(dsp,s->low,low_frames)){s->errors++;return 0;}
  spx_uint32_t used=low_frames,out_frames=DISC_SRC_CHUNK+32;
  if(speex_resampler_process_interleaved_float(s->up,s->low,&used,s->out,&out_frames)||used!=low_frames){s->errors++;return 0;}
  unsigned available=s->pending_count+out_frames;if(available<n){s->underflows++;s->errors++;return 0;}
  for(unsigned i=0;i<n;i++){
   float io[2];for(unsigned c=0;c<2;c++)io[c]=i<s->pending_count?s->pending[i][c]:s->out[(i-s->pending_count)*2+c];
   output_guard_process(&s->guard,io);
   for(unsigned c=0;c<2;c++){float v=io[c];pcm[(pos+i)*2+c]=v>=1?INT32_MAX:v<=-1?INT32_MIN:(int)(v*2147483648.f);}
  }
  unsigned keep=available-n;if(keep>32){s->errors++;return 0;}
  for(unsigned i=0;i<keep;i++)for(unsigned c=0;c<2;c++){unsigned at=n+i;s->pending[i][c]=at<s->pending_count?s->pending[at][c]:s->out[(at-s->pending_count)*2+c];}
  s->pending_count=keep;pos+=n;
 }
 return 1;
}
static void src_stats(void *context,unsigned *latency,unsigned *errors,unsigned *underflows,unsigned *invalid){DiscSrc *s=context;*latency=s?s->latency:0;*errors=s?s->errors:0;*underflows=s?s->underflows:0;*invalid=s?s->guard.invalid:0;}
static const DiscResampleApi api={1,src_create,src_destroy,src_process,src_stats};
__attribute__((visibility("default"))) const DiscResampleApi *disc_resample_get_api(void){return &api;}
