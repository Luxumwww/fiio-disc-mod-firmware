#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "resample_api.h"
#include "third_party/soxr/src/soxr.h"
#include "third_party/soxr/src/aliases.h"
#include "../audio-dsp/output_guard.h"
/* Fixed integer-ratio VHQ bridge. All setup/warming runs on the control thread.
 * The PCM worker receives prepared contexts and bounded, preallocated FIFOs. */
enum {SRC_CHUNK=1024,SRC_LOW=8192,SRC_OUT=32768};
typedef struct {
 unsigned input_rate,processing_rate,ratio,latency,errors,underflows,capacity,head,count;
 soxr_t down,up;OutputGuard guard;
 float (*queue)[2];float input[SRC_CHUNK*2],low[SRC_LOW*2],out[SRC_OUT*2];
} Src;
extern void lsx_safe_rdft(int,int,double *);
static int cache_ready;
static int src_cache_prepare(void){
 if(cache_ready)return 1;
 double *zero=calloc(65536,sizeof(*zero));if(!zero)return 0;
 /* Immutable maximum FFT tables avoid a control/audio cache-growth race. */
 lsx_safe_rdft(65536,1,zero);free(zero);cache_ready=1;return 1;
}
static void src_destroy(void *context){Src *s=context;if(!s)return;if(s->down)soxr_delete(s->down);if(s->up)soxr_delete(s->up);free(s->queue);free(s);}
static int src_pump(Src *s,unsigned n,DiscResampleProcessFloat process,void *dsp,unsigned *produced){
 size_t used=0,low=0,up_used=0,high=0;
 soxr_error_t err=soxr_process(s->down,s->input,n,&used,s->low,SRC_LOW,&low);
 if(err||used!=n)return 0;
 if(low&&process&&!process(dsp,s->low,(unsigned)low))return 0;
 err=soxr_process(s->up,s->low,low,&up_used,s->out,SRC_OUT,&high);
 if(err||up_used!=low)return 0;*produced=(unsigned)high;return 1;
}
static void *src_create(unsigned input,unsigned processing){
 if(input>384000||processing<44100||input%processing)return 0;
 unsigned ratio=input/processing;if(ratio!=2&&ratio!=4&&ratio!=8)return 0;
 if((input%48000==0)!=(processing%48000==0)||!src_cache_prepare())return 0;
 Src *s=calloc(1,sizeof(*s));if(!s)return 0;s->input_rate=input;s->processing_rate=processing;s->ratio=ratio;
 soxr_quality_spec_t quality=soxr_quality_spec(SOXR_VHQ,SOXR_DOUBLE_PRECISION);quality.passband_end=.95;
 soxr_io_spec_t io=soxr_io_spec(SOXR_FLOAT32_I,SOXR_FLOAT32_I);soxr_runtime_spec_t runtime=soxr_runtime_spec(1);runtime.log2_min_dft_size=8;runtime.log2_large_dft_size=12;
 soxr_error_t err=0;s->down=soxr_create(input,processing,2,&err,&io,&quality,&runtime);if(!s->down||err){src_destroy(s);return 0;}
 s->up=soxr_create(processing,input,2,&err,&io,&quality,&runtime);if(!s->up||err){src_destroy(s);return 0;}
 unsigned delay=ratio*4096;s->capacity=delay+SRC_OUT+SRC_CHUNK;s->queue=calloc(s->capacity,sizeof(*s->queue));if(!s->queue){src_destroy(s);return 0;}
 /* Warm private FFT/FIFO working memory and leave a zero history. Never
  * clear the engine afterwards: that could discard prepared allocations. */
 for(unsigned i=0;i<delay*4;i+=SRC_CHUNK){unsigned out;if(!src_pump(s,SRC_CHUNK,0,0,&out)){src_destroy(s);return 0;}}
 double held=soxr_delay(s->down)*ratio+soxr_delay(s->up);unsigned buffered=(unsigned)(held+.5);
 if(buffered>delay||fabs(held-buffered)>.001){src_destroy(s);return 0;}
 s->count=delay-buffered;output_guard_init(&s->guard,input);s->latency=delay+s->guard.delay;
 return s;
}
static int src_process(void *context,int *pcm,unsigned frames,DiscResampleProcessFloat process,void *dsp){
 Src *s=context;if(!s||!pcm||!process||frames>1048576)return 0;
 for(unsigned at=0;at<frames;){
  unsigned n=frames-at;if(n>SRC_CHUNK)n=SRC_CHUNK;
  for(unsigned i=0;i<n*2;i++)s->input[i]=pcm[at*2+i]*(1.f/2147483648.f);
  unsigned produced;if(!src_pump(s,n,process,dsp,&produced)){s->errors++;return 0;}
  if(produced>s->capacity-s->count){s->errors++;return 0;}
  unsigned tail=(s->head+s->count)%s->capacity,first=s->capacity-tail;if(first>produced)first=produced;
  memcpy(s->queue+tail,s->out,first*8);if(first<produced)memcpy(s->queue,s->out+first*2,(produced-first)*8);s->count+=produced;
  if(s->count<n){s->underflows++;s->errors++;return 0;}
  for(unsigned i=0;i<n;i++){
   float io[2]={s->queue[s->head][0],s->queue[s->head][1]};if(++s->head==s->capacity)s->head=0;
   output_guard_process(&s->guard,io);
   for(unsigned c=0;c<2;c++){float value=io[c];pcm[(at+i)*2+c]=value>=1?INT32_MAX:value<=-1?INT32_MIN:(int)(value*2147483648.f);}
  }
  s->count-=n;at+=n;
 }
 return 1;
}
static void src_stats(void *context,unsigned *latency,unsigned *errors,unsigned *underflows,unsigned *invalid){Src *s=context;*latency=s?s->latency:0;*errors=s?s->errors:0;*underflows=s?s->underflows:0;*invalid=s?s->guard.invalid:0;}
static void src_diagnostics(void *context,unsigned *limited,float *peak,float *gain){Src *s=context;*limited=s?s->guard.limited:0;*peak=s?s->guard.peak:0;*gain=s?s->guard.gain:1;}
static const DiscResampleApi api={2,src_create,src_destroy,src_process,src_stats,src_diagnostics};
__attribute__((visibility("default"))) const DiscResampleApi *disc_resample_get_api(void){return &api;}
