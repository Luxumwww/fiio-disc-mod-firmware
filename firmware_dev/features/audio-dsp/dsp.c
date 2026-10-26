#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#ifdef __mips_msa
#include <msa.h>
#endif
#include "dsp_api.h"
#include "parameters.h"
#include <time.h>
#ifdef DISC_DSP_PROFILE
static unsigned long long profile_time(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (unsigned long long)t.tv_sec*1000000000+t.tv_nsec;}
#endif
#include "freeverb.h"
#include "tube.h"
#include "v8.h"
#include "purest_warm.h"
#include "galactic.h"
#include "dragonfly_adapter.h"
#include "output_guard.h"
#include "creative_effects.h"
#include "chain_format.h"
#include "chain_peq.h"
#include "haas.h"
#include "ffhaas_core.h"
#include "stereofx_core.h"
#include "warmgloss.h"
#ifdef DISC_DSP_UNITY
#include "freeverb.c"
#endif
typedef struct Dsp {
 unsigned rate,mask,available,frames,clipped;
 float envelope,gain,attack,release,detect,fade,mix[17],curve[4097];
 float threshold,ratio_power,comp_makeup,comp_mix,warm_mix,warm_output;
 freeverb_t *hall;
 V8 *v8;Tube *tube;PurestWarm3State *warm;Galactic3State *galactic;void *dragonfly;OutputGuard guard;
 unsigned is_chain,is_peq,node_count,node_types[DISC_CHAIN_MAX],node_enabled[DISC_CHAIN_MAX];struct Dsp *nodes[DISC_CHAIN_MAX];float chain_mix;DiscPeq peq;
 DiscStereo stereo;DiscDelay *delay;DiscDynamic deesser,dynamic_eq;
 DiscHaas *haas;
 DiscFFHaas *ffhaas;
 DiscStereoFx *stereofx;
 DiscWarmGloss *warmgloss;
} Dsp;
static void destroy(void *);
static void *create_configured(unsigned rate,unsigned mask,const char *root,const float params[12][12]){
 if(params&&!disc_params_valid(params))return 0;
 if(rate<8000||rate>768000||mask>4095)return 0;(void)root;mask&=4093;Dsp *d=calloc(1,sizeof(*d));if(!d)return 0;
 output_guard_init(&d->guard,rate);d->rate=rate;d->mask=mask;d->gain=1;d->available=DISC_DSP_COMPRESSOR;d->fade=1.f/(rate*.05f);
 d->threshold=.177827941f;d->ratio_power=2.f/3.f;d->comp_makeup=d->comp_mix=d->warm_mix=d->warm_output=1;
 d->attack=expf(-1.f/(rate*.010f));d->release=expf(-1.f/(rate*.200f));d->detect=expf(-1.f/(rate*.001f));
 for(unsigned j=0;j<=4096;j++){float e=j/512.f;d->curve[j]=e>.177827941f?powf(.177827941f/e,2.f/3.f):1.f;}
 if((mask&DISC_DSP_WARM)&&rate<=384000){d->warm=purest_warm_create(rate);if(d->warm)d->available|=DISC_DSP_WARM;}
 if((mask&DISC_DSP_GALACTIC)&&rate<=384000){d->galactic=galactic_create(rate);if(d->galactic)d->available|=DISC_DSP_GALACTIC;}
 if((mask&DISC_DSP_DRAGONFLY)&&rate<=384000){const float *p=params&&memcmp(params[6],disc_param_default[6],sizeof(params[6]))?params[6]:0;d->dragonfly=p?disc_dragonfly_create_configured(rate,p):disc_dragonfly_create(rate);if(d->dragonfly)d->available|=DISC_DSP_DRAGONFLY;}
 if((mask&DISC_DSP_V8)&&rate<=384000){d->v8=v8_create(rate);if(d->v8)d->available|=DISC_DSP_V8;}
 if((mask&DISC_DSP_TUBE)&&rate<=384000){d->tube=tube_create(rate);if(d->tube)d->available|=DISC_DSP_TUBE;}
 if((mask&DISC_DSP_HALL)&&rate<=384000){d->hall=malloc(sizeof(freeverb_t));if(d->hall){freeverb_init(d->hall,rate);d->available|=DISC_DSP_HALL;}}
 if(params){
  if(d->hall&&memcmp(params[0],disc_param_default[0],sizeof(params[0]))){BASS_BFX_FREEVERB p=d->hall->p;p.fDryMix=params[0][0];p.fWetMix=params[0][1];p.fRoomSize=params[0][2];p.fDamp=params[0][3];p.fWidth=params[0][4];freeverb_set_params(d->hall,&p);}
  if(memcmp(params[2],disc_param_default[2],sizeof(params[2]))){const float *p=params[2];d->threshold=powf(10.f,p[0]/20.f);d->ratio_power=1.f-1.f/p[1];d->attack=expf(-1.f/(rate*p[2]*.001f));d->release=expf(-1.f/(rate*p[3]*.001f));d->comp_makeup=powf(10.f,p[4]/20.f);d->comp_mix=p[5];for(unsigned j=0;j<=4096;j++){float e=j/512.f;d->curve[j]=e>d->threshold?powf(d->threshold/e,d->ratio_power):1.f;}}
  if(d->tube&&memcmp(params[3],disc_param_default[3],sizeof(params[3])))tube_configure(d->tube,rate,params[3]);
  if(d->v8&&memcmp(params[4],disc_param_default[4],sizeof(params[4])))v8_configure(d->v8,rate,params[4]);
  if(d->warm){d->warm->asym=params[5][0];d->warm_mix=params[5][1];d->warm_output=params[5][2];}
  if(d->galactic&&memcmp(params[7],disc_param_default[7],sizeof(params[7]))){const float *p=params[7];d->galactic->A=p[0];d->galactic->B=p[1];d->galactic->C=p[2];d->galactic->D=p[3];d->galactic->E=p[4];d->galactic->F=p[5];galactic_recalculate(d->galactic,rate);}
 }
 if(rate<=384000){
  const float (*p)[12]=params?params:disc_param_default;
  if(mask&DISC_DSP_STEREO){creative_stereo_init(&d->stereo,rate,p[8]);d->available|=DISC_DSP_STEREO;}
  if(mask&DISC_DSP_DELAY){d->delay=creative_delay_create(rate,p[9]);if(!d->delay){destroy(d);return 0;}d->available|=DISC_DSP_DELAY;}
  if(mask&DISC_DSP_DEESSER){creative_dynamic_init(&d->deesser,rate,p[10],1);d->available|=DISC_DSP_DEESSER;}
  if(mask&DISC_DSP_DYNAMIC_EQ){creative_dynamic_init(&d->dynamic_eq,rate,p[11],0);d->available|=DISC_DSP_DYNAMIC_EQ;}
 }
 return d;
}
static void *create(unsigned rate,unsigned mask,const char *root){return create_configured(rate,mask,root,0);}
__attribute__((visibility("default"))) void *disc_dsp_create_configured(unsigned rate,unsigned mask,const char *root,const float params[12][12]){return create_configured(rate,mask,root,params);}
static void destroy(void *context){Dsp *d=context;if(!d)return;for(unsigned i=0;i<d->node_count;i++)destroy(d->nodes[i]);free(d->hall);free(d->tube);free(d->warm);free(d->galactic);if(d->dragonfly)disc_dragonfly_destroy(d->dragonfly);v8_destroy(d->v8);creative_delay_destroy(d->delay);disc_haas_destroy(d->haas);disc_ffhaas_destroy(d->ffhaas);disc_stereofx_destroy(d->stereofx);disc_warmgloss_destroy(d->warmgloss);free(d);}
static void set_mask(void *context,unsigned mask){
 Dsp *d=context;if(!d)return;
 if(d->is_chain){unsigned before=d->mask;d->mask=mask&DISC_DSP_CUSTOM;if(d->mask&&!before)for(unsigned i=0;i<d->node_count;i++){set_mask(d->nodes[i],0);set_mask(d->nodes[i],d->node_enabled[i]?(1u<<d->node_types[i]):0);}return;}
 if(d->is_peq){if((mask&DISC_DSP_CUSTOM)&&!d->mask)chain_peq_reset(&d->peq);d->mask=mask&DISC_DSP_CUSTOM;return;}
 if(d->warmgloss){if((mask&DISC_WARMGLOSS_MASK)&&!d->mask){disc_warmgloss_reset(d->warmgloss);d->mix[16]=0;}d->mask=mask&DISC_WARMGLOSS_MASK;return;}
 if(d->haas){if((mask&DISC_HAAS_MASK)&&!d->mask)disc_haas_reset(d->haas);d->mask=mask&DISC_HAAS_MASK;return;}
 if(d->stereofx){if((mask&DISC_STEREOFX_MASK)&&!d->mask){disc_stereofx_reset(d->stereofx);d->mix[15]=0;}d->mask=mask&DISC_STEREOFX_MASK;return;}
 if(d->ffhaas){if((mask&DISC_FFHAAS_MASK)&&!d->mask)disc_ffhaas_reset(d->ffhaas);d->mask=mask&DISC_FFHAAS_MASK;return;}
 unsigned enabled=(mask&4093)&~d->mask;
 // A new activation must not replay samples left by an earlier activation.
 if((enabled&DISC_DSP_HALL)&&d->hall)freeverb_mute(d->hall);
 if(enabled&DISC_DSP_COMPRESSOR){d->gain=1;d->envelope=0;}
 if((enabled&DISC_DSP_V8)&&d->v8)v8_reset(d->v8);
 if((enabled&DISC_DSP_TUBE)&&d->tube)tube_reset(d->tube);
 if((enabled&DISC_DSP_WARM)&&d->warm){float asym=d->warm->asym;purest_warm_reset(d->warm,d->rate);d->warm->asym=asym;}
 if((enabled&DISC_DSP_GALACTIC)&&d->galactic){float p[6]={d->galactic->A,d->galactic->B,d->galactic->C,d->galactic->D,d->galactic->E,d->galactic->F};galactic_reset(d->galactic,d->rate);d->galactic->A=p[0];d->galactic->B=p[1];d->galactic->C=p[2];d->galactic->D=p[3];d->galactic->E=p[4];d->galactic->F=p[5];galactic_recalculate(d->galactic,d->rate);}
 if((enabled&DISC_DSP_DRAGONFLY)&&d->dragonfly)disc_dragonfly_reset(d->dragonfly);
 if(enabled&DISC_DSP_STEREO){d->stereo.side_low=d->stereo.feed[0]=d->stereo.feed[1]=0;}
 if((enabled&DISC_DSP_DELAY)&&d->delay)creative_delay_reset(d->delay);
 if(enabled&DISC_DSP_DEESSER)creative_dynamic_reset(&d->deesser);
 if(enabled&DISC_DSP_DYNAMIC_EQ)creative_dynamic_reset(&d->dynamic_eq);
 d->mask=mask&4093;
}
static float ramp(Dsp *d,unsigned i,unsigned bit){float target=(d->mask&d->available&bit)?1.f:0.f;float v=d->mix[i];if(v<target){v+=d->fade;if(v>target)v=target;}else if(v>target){v-=d->fade;if(v<target)v=target;}return d->mix[i]=v;}
static void effect_sample(Dsp *d,unsigned type,float *io){if(!d)return;float mix;
 switch(type){
 case 2: mix=ramp(d,1,DISC_DSP_COMPRESSOR);
  if((d->mask&DISC_DSP_COMPRESSOR)||mix>0){
   float l=fabsf(io[0]),r=fabsf(io[1]),peak=l>r?l:r;d->envelope=d->detect*d->envelope+(1-d->detect)*peak;
   float target=1;if(d->envelope>d->threshold){float x=d->envelope*512;unsigned j=(unsigned)x;if(j<4096)target=d->curve[j]+(d->curve[j+1]-d->curve[j])*(x-j);else target=powf(d->threshold/d->envelope,d->ratio_power);}
   float a=target<d->gain?d->attack:d->release;d->gain=a*d->gain+(1-a)*target;
   float gain=1+(d->gain*d->comp_makeup-1)*mix*d->comp_mix;io[0]*=gain;io[1]*=gain;
  } break;
 case 0: mix=ramp(d,2,DISC_DSP_HALL);
  if(d->hall&&((d->mask&DISC_DSP_HALL)||mix>0)){float wet[2]={io[0],io[1]};freeverb_process(d->hall,wet,1);io[0]+=(wet[0]-io[0])*mix;io[1]+=(wet[1]-io[1])*mix;} break;
 case 4: mix=ramp(d,4,DISC_DSP_V8);
  if(d->v8&&((d->mask&DISC_DSP_V8)||mix>0)){float wet[2]={io[0],io[1]};v8_sample(d->v8,wet);io[0]+=(wet[0]-io[0])*mix;io[1]+=(wet[1]-io[1])*mix;} break;
 case 3: mix=ramp(d,3,DISC_DSP_TUBE);
  if(d->tube&&((d->mask&DISC_DSP_TUBE)||mix>0)){output_guard_sanitize(&d->guard,io);for(unsigned c=0;c<2;c++){float wet=tube_sample(d->tube,io[c],c);io[c]+=(wet-io[c])*mix;}} break;
 case 5: mix=ramp(d,5,DISC_DSP_WARM);
  if(d->warm&&((d->mask&DISC_DSP_WARM)||mix>0)){float wet[2]={io[0],io[1]};purest_warm_sample(d->warm,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]*d->warm_output-io[c])*mix*d->warm_mix;} break;
 case 6: mix=ramp(d,6,DISC_DSP_DRAGONFLY);
  if(d->dragonfly&&((d->mask&DISC_DSP_DRAGONFLY)||mix>0)){float wet[2]={io[0],io[1]};disc_dragonfly_sample(d->dragonfly,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;} break;
 case 7: mix=ramp(d,7,DISC_DSP_GALACTIC);
  if(d->galactic&&((d->mask&DISC_DSP_GALACTIC)||mix>0)){float wet[2]={io[0],io[1]};galactic_sample(d->galactic,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;} break;
 case 8: mix=ramp(d,8,DISC_DSP_STEREO);
  if((d->available&DISC_DSP_STEREO)&&((d->mask&DISC_DSP_STEREO)||mix>0)){float wet[2]={io[0],io[1]};creative_stereo(&d->stereo,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;} break;
 case 9: mix=ramp(d,9,DISC_DSP_DELAY);
  if(d->delay&&((d->mask&DISC_DSP_DELAY)||mix>0)){float wet[2]={io[0],io[1]};creative_delay(d->delay,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;} break;
 case 10: mix=ramp(d,10,DISC_DSP_DEESSER);
  if((d->available&DISC_DSP_DEESSER)&&((d->mask&DISC_DSP_DEESSER)||mix>0)){float wet[2]={io[0],io[1]};creative_dynamic(&d->deesser,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;} break;
 case 11: mix=ramp(d,11,DISC_DSP_DYNAMIC_EQ);
  if((d->available&DISC_DSP_DYNAMIC_EQ)&&((d->mask&DISC_DSP_DYNAMIC_EQ)||mix>0)){float wet[2]={io[0],io[1]};creative_dynamic(&d->dynamic_eq,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;} break;
 case 12: mix=ramp(d,12,DISC_DSP_CUSTOM);if((d->mask&DISC_DSP_CUSTOM)||mix>0){float wet[2]={io[0],io[1]};chain_peq_sample(&d->peq,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;}break;
 case DISC_HAAS_TYPE:mix=ramp(d,13,DISC_HAAS_MASK);if(d->haas&&((d->mask&DISC_HAAS_MASK)||mix>0)){float wet[2]={io[0],io[1]};disc_haas_sample(d->haas,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;}break;
 case DISC_WARMGLOSS_TYPE:mix=ramp(d,16,DISC_WARMGLOSS_MASK);if(d->warmgloss&&((d->mask&DISC_WARMGLOSS_MASK)||mix>0)){float wet[2]={io[0],io[1]};disc_warmgloss_sample(d->warmgloss,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;}break;
 case DISC_STEREOFX_TYPE:mix=ramp(d,15,DISC_STEREOFX_MASK);if(d->stereofx&&((d->mask&DISC_STEREOFX_MASK)||mix>0)){float wet[2]={io[0],io[1]};disc_stereofx_sample(d->stereofx,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;}break;
 case DISC_FFHAAS_TYPE:mix=ramp(d,14,DISC_FFHAAS_MASK);if(d->ffhaas&&((d->mask&DISC_FFHAAS_MASK)||mix>0)){float wet[2]={io[0],io[1]};disc_ffhaas_sample(d->ffhaas,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;}break;
 default:break;
 }
}
static int process(void *context,int *pcm,unsigned frames){
 Dsp *d=context;if(!d||!pcm)return 0;
 /* The saved chain is immutable during this locked block. Resolve only its
  * allocated, enabled nodes once, retaining saved order and OFF fades. */
 Dsp *active[DISC_CHAIN_MAX];unsigned active_types[DISC_CHAIN_MAX],active_count=0;
 if(d->is_chain)for(unsigned j=0;j<d->node_count;j++)if(d->node_enabled[j]&&d->nodes[j]){active[active_count]=d->nodes[j];active_types[active_count++]=d->node_types[j];}
 float chain_target=d->mask&&d->available&&active_count?1:0;
 Dsp *single_warm=d->is_chain&&active_count==1&&active_types[0]==DISC_WARMGLOSS_TYPE?active[0]:0;
 for(unsigned i=0;i<frames;i++){
  float io[2]={pcm[2*i]*(1.f/2147483648.f),pcm[2*i+1]*(1.f/2147483648.f)};
  if(d->is_chain){
   float target=chain_target;
   if(d->chain_mix<target){d->chain_mix+=d->fade;if(d->chain_mix>target)d->chain_mix=target;}else if(d->chain_mix>target){d->chain_mix-=d->fade;if(d->chain_mix<target)d->chain_mix=target;}
   if(d->chain_mix>0){
    if(single_warm){float mix=ramp(single_warm,16,DISC_WARMGLOSS_MASK);if(single_warm->warmgloss&&((single_warm->mask&DISC_WARMGLOSS_MASK)||mix>0)){float wet[2]={io[0],io[1]};disc_warmgloss_sample(single_warm->warmgloss,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;}}
    else for(unsigned j=0;j<active_count;j++)effect_sample(active[j],active_types[j],io);
   }
  }else {static const unsigned order[]={2,0,4,3,5,6,7,8,9,10,11};for(unsigned j=0;j<11;j++)effect_sample(d,order[j],io);}
  // Avoid float round trips once every effect is fully bypassed.
  float guard_mix=d->is_chain?d->chain_mix:0;if(!d->is_chain)for(unsigned j=0;j<17;j++)if(d->mix[j]>guard_mix)guard_mix=d->mix[j];
  if(guard_mix==0){if(d->guard.active)output_guard_reset(&d->guard);continue;}
  output_guard_process(&d->guard,io);
  // Crossfade the complete delayed chain to the original undelayed input
  // while toggling. Both endpoints are bounded; OFF remains bit-exact.
  if(guard_mix<1)for(unsigned c=0;c<2;c++){float dry=pcm[2*i+c]*(1.f/2147483648.f);io[c]=dry+(io[c]-dry)*guard_mix;if(io[c]>.99999f)io[c]=.99999f;if(io[c]<-.99999f)io[c]=-.99999f;}
  for(unsigned c=0;c<2;c++){
   float v=io[c];if(!isfinite(v)){v=0;d->guard.invalid++;}
   if(v>=1){pcm[2*i+c]=INT32_MAX;d->clipped++;}else if(v<=-1){pcm[2*i+c]=INT32_MIN;d->clipped++;}else pcm[2*i+c]=(int)(v*2147483648.f);
  }
 }
 d->frames+=frames;return 1;
}
__attribute__((visibility("default"))) int disc_dsp_process_float(void *context,float *pcm,unsigned frames){
 Dsp *d=context;if(!d||!pcm)return 0;
 /* The saved chain is immutable during this locked block. Resolve only its
  * allocated, enabled nodes once, retaining saved order and OFF fades. */
 Dsp *active[DISC_CHAIN_MAX];unsigned active_types[DISC_CHAIN_MAX],active_count=0;
 if(d->is_chain)for(unsigned j=0;j<d->node_count;j++)if(d->node_enabled[j]&&d->nodes[j]){active[active_count]=d->nodes[j];active_types[active_count++]=d->node_types[j];}
 float chain_target=d->mask&&d->available&&active_count?1:0;
 Dsp *single_warm=d->is_chain&&active_count==1&&active_types[0]==DISC_WARMGLOSS_TYPE?active[0]:0;
 for(unsigned i=0;i<frames;i++){
  float *io=pcm+i*2,dry[2]={io[0],io[1]};
  if(d->is_chain){
   float target=chain_target;
   if(d->chain_mix<target){d->chain_mix+=d->fade;if(d->chain_mix>target)d->chain_mix=target;}else if(d->chain_mix>target){d->chain_mix-=d->fade;if(d->chain_mix<target)d->chain_mix=target;}
   if(d->chain_mix>0){
    if(single_warm){float mix=ramp(single_warm,16,DISC_WARMGLOSS_MASK);if(single_warm->warmgloss&&((single_warm->mask&DISC_WARMGLOSS_MASK)||mix>0)){float wet[2]={io[0],io[1]};disc_warmgloss_sample(single_warm->warmgloss,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;}}
    else for(unsigned j=0;j<active_count;j++)effect_sample(active[j],active_types[j],io);
   }
  }else {static const unsigned order[]={2,0,4,3,5,6,7,8,9,10,11};for(unsigned j=0;j<11;j++)effect_sample(d,order[j],io);}
  /* SRC owns final full-rate protection, after reconstruction. Keep floating
   * headroom here so the downsampler cannot clip before nonlinear effects. */
  float guard_mix=d->is_chain?d->chain_mix:0;if(!d->is_chain)for(unsigned j=0;j<17;j++)if(d->mix[j]>guard_mix)guard_mix=d->mix[j];
  if(guard_mix<1)for(unsigned c=0;c<2;c++)io[c]=dry[c]+(io[c]-dry[c])*guard_mix;
  for(unsigned c=0;c<2;c++)if(!isfinite(io[c])){io[c]=0;d->guard.invalid++;}

 }
 d->frames+=frames;return 1;
}
static unsigned chain_error;
__attribute__((visibility("default"))) unsigned disc_dsp_chain_error(void){return chain_error;}
static unsigned context_bytes(Dsp *d){unsigned bytes=sizeof(*d);if(d->hall)bytes+=sizeof(*d->hall);if(d->tube)bytes+=sizeof(*d->tube);if(d->warm)bytes+=sizeof(*d->warm);if(d->galactic)bytes+=sizeof(*d->galactic);if(d->v8)bytes+=sizeof(*d->v8)+d->v8->bytes;if(d->delay)bytes+=sizeof(*d->delay)+d->delay->length*2*sizeof(float);if(d->haas)bytes+=sizeof(*d->haas)+d->haas->length*sizeof(float);if(d->warmgloss)bytes+=disc_warmgloss_memory(d->warmgloss);if(d->stereofx)bytes+=disc_stereofx_memory(d->stereofx);if(d->ffhaas)bytes+=disc_ffhaas_memory(d->ffhaas);if(d->dragonfly)bytes+=disc_dragonfly_memory(d->dragonfly);return bytes;}
__attribute__((visibility("default"))) void *disc_dsp_create_chain(unsigned rate,const DiscChain *spec){
 chain_error=0;if(!disc_chain_valid(spec)){chain_error=4;return 0;}if(rate<8000||rate>384000){chain_error=1;return 0;}
 Dsp *d=calloc(1,sizeof(*d));if(!d){chain_error=3;return 0;}d->rate=rate;d->is_chain=1;d->mask=DISC_DSP_CUSTOM;d->available=0;d->fade=1.f/(rate*.05f);output_guard_init(&d->guard,rate);unsigned total=sizeof(*d);
 for(unsigned i=0;i<spec->count;i++){
  const DiscChainNode *n=&spec->nodes[i];Dsp *node;
  if(!n->enabled){d->node_types[i]=n->type;d->node_enabled[i]=0;d->node_count++;continue;}
  if(n->type==12){node=calloc(1,sizeof(*node));if(node){node->is_peq=1;node->rate=rate;node->mask=node->available=DISC_DSP_CUSTOM;node->fade=d->fade;chain_peq_init(&node->peq,rate,n->params);}}
  else if(n->type==DISC_WARMGLOSS_TYPE){node=calloc(1,sizeof(*node));if(node){node->rate=rate;node->mask=node->available=DISC_WARMGLOSS_MASK;node->fade=d->fade;node->warmgloss=disc_warmgloss_create(rate,n->params);if(!node->warmgloss){destroy(node);node=0;}}}
  else if(n->type==DISC_HAAS_TYPE){node=calloc(1,sizeof(*node));if(node){node->rate=rate;node->mask=node->available=DISC_HAAS_MASK;node->fade=d->fade;node->haas=disc_haas_create(rate,n->params);if(!node->haas){destroy(node);node=0;}}}
  else if(n->type==DISC_STEREOFX_TYPE){node=calloc(1,sizeof(*node));if(node){node->rate=rate;node->mask=node->available=DISC_STEREOFX_MASK;node->fade=d->fade;node->stereofx=disc_stereofx_create(rate,n->params);if(!node->stereofx){destroy(node);node=0;}}}
  else if(n->type==DISC_FFHAAS_TYPE){node=calloc(1,sizeof(*node));if(node){node->rate=rate;node->mask=node->available=DISC_FFHAAS_MASK;node->fade=d->fade;node->ffhaas=disc_ffhaas_create(rate,n->params);if(!node->ffhaas){destroy(node);node=0;}}}
  else {float params[12][12];memcpy(params,disc_param_default,sizeof(params));memcpy(params[n->type],n->params,sizeof(n->params));node=create_configured(rate,1u<<n->type,".",params);}
  if(!node){chain_error=3;destroy(d);return 0;}d->nodes[i]=node;d->node_types[i]=n->type;d->node_enabled[i]=n->enabled;d->node_count++;d->available=DISC_DSP_CUSTOM;
  if(!(node->available&(1u<<n->type))){chain_error=3;destroy(d);return 0;}
  if(!n->enabled)set_mask(node,0);total+=context_bytes(node);if(total>16u*1024u*1024u){chain_error=2;destroy(d);return 0;}
 }
 return d;
}
static void stats(void *context,unsigned *available,unsigned *latency,unsigned *clipped){Dsp *d=context;*available=d?d->available:0;*latency=d?((d->mask&d->available)||d->guard.active?d->guard.delay:0):0;*clipped=d?d->clipped:0;

}
static void health(void *context,unsigned *underflows,unsigned *completed){(void)context;*underflows=0;*completed=0;}
static void diagnostics(void *context,unsigned *limited,unsigned *invalid,float *peak,float *gain){Dsp *d=context;*limited=d?d->guard.limited:0;*invalid=d?d->guard.invalid:0;*peak=d?d->guard.peak:0;*gain=d?d->guard.gain:1;}
static const DiscDspApi api={3,create,destroy,set_mask,process,stats,health,diagnostics};
__attribute__((visibility("default"))) const DiscDspApi *disc_dsp_get_api(void){return &api;}
