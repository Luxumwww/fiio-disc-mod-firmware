#ifndef DISC_WARMGLOSS_H
#define DISC_WARMGLOSS_H
/* Project voicing around an unmodified Airwindows Tube2 double core.
 * Parallel, low-passed coloration leaves the original treble intact. The
 * shelves implement the requested bass reduction and treble gloss; they are
 * project tuning, not a claim to model a particular tube circuit.
 * All coefficients and the core are prepared outside the audio callback. */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tube2_core_inline.h"
#include "powersag2_core.h"
#include "warmgloss_params.h"
typedef struct {double b0,b1,b2,a1,a2,z1[2],z2[2];} DiscWarmShelf;
typedef struct {
 DiscTube2 *tube;
 DiscPowerSag2 *sag;
 DiscWarmShelf bass,gloss;
 double body[2][2],dc_x[2],dc_y[2];
 double body_pole,dc_pole,amount,normalize,output;
 double sag_low[2],sag_pole,sag_amount,sag_gain,sag_target,sag_smooth,sag_hold;
 unsigned rate,sag_phase;
 unsigned bass_on,gloss_on;
} DiscWarmGloss;
static void warmgloss_shelf(DiscWarmShelf *f,unsigned rate,double hz,double db,int high){
 if(hz>rate*.40)hz=rate*.40;
 double w=6.283185307179586*hz/rate,c=cos(w),s=sin(w);
 double a=pow(10,db/40),root=sqrt(a),alpha=s*.7071067811865476;
 double b0,b1,b2,a0,a1,a2;
 if(!high){b0=a*((a+1)-(a-1)*c+2*root*alpha);b1=2*a*((a-1)-(a+1)*c);b2=a*((a+1)-(a-1)*c-2*root*alpha);a0=(a+1)+(a-1)*c+2*root*alpha;a1=-2*((a-1)+(a+1)*c);a2=(a+1)+(a-1)*c-2*root*alpha;}
 else {b0=a*((a+1)+(a-1)*c+2*root*alpha);b1=-2*a*((a-1)+(a+1)*c);b2=a*((a+1)+(a-1)*c-2*root*alpha);a0=(a+1)-(a-1)*c+2*root*alpha;a1=2*((a-1)-(a+1)*c);a2=(a+1)-(a-1)*c-2*root*alpha;}
 f->b0=b0/a0;f->b1=b1/a0;f->b2=b2/a0;f->a1=a1/a0;f->a2=a2/a0;
}
static double warmgloss_filter(DiscWarmShelf *f,double x,unsigned ch){
 double y=f->b0*x+f->z1[ch];f->z1[ch]=f->b1*x-f->a1*y+f->z2[ch];f->z2[ch]=f->b2*x-f->a2*y;return y;
}
static DiscWarmGloss *disc_warmgloss_create(unsigned rate,const float *p){
 if(rate<8000||rate>384000||!disc_warmgloss_params_valid(p))return 0;
 DiscWarmGloss *s=calloc(1,sizeof(*s));if(!s)return 0;
 s->tube=disc_tube2_create(rate,.5,p[1]);if(!s->tube){free(s);return 0;}
 s->rate=rate;s->sag_amount=p[6];s->sag_gain=s->sag_target=1;
 s->sag_pole=exp(-6.283185307179586*180/rate);
 s->sag_smooth=1-exp(-1.0/(rate*.002));
 if(s->sag_amount>0){
  /* The original history runs at a fixed control rate. Music PCM is never
   * resampled. This makes 20..150ms windows consistent even at 384kHz. */
  double window=48000*(.020+.130*p[7]);
  double range=pow((window-1)/16383.,.25);
  s->sag=disc_powersag2_create(range,1);
  if(!s->sag){disc_tube2_destroy(s->tube);free(s);return 0;}
 }
 s->amount=p[0];s->body_pole=exp(-6.283185307179586*1800/rate);
 s->dc_pole=exp(-6.283185307179586*8/rate);s->output=pow(10,p[5]/20.);
 /* Normalize the original Tube2 small-signal gain. A=.5 and its
  * power-dependent output scale are included. The double-path high-rate
  * averages all have unity DC gain (unlike an older float-path branch). */
 unsigned power=(unsigned)(9.0*(1.0-(double)p[1])+1.0);
 double linear=.5*(1.0+1.0/power)*1.923076923076923;
 s->normalize=1.0/linear;
 s->bass_on=p[3]!=0;s->gloss_on=p[2]!=0;
 warmgloss_shelf(&s->bass,rate,p[4],p[3],0);
 warmgloss_shelf(&s->gloss,rate,4500,p[2],1);
 return s;
}
static void disc_warmgloss_destroy(DiscWarmGloss *s){if(s){disc_tube2_destroy(s->tube);disc_powersag2_destroy(s->sag);free(s);}}
static void disc_warmgloss_reset(DiscWarmGloss *s){
 if(!s)return;disc_tube2_reset(s->tube);
 if(s->sag)disc_powersag2_reset(s->sag);
 s->sag_low[0]=s->sag_low[1]=s->sag_hold=0;s->sag_phase=0;s->sag_gain=s->sag_target=1;
 memset(s->body,0,sizeof(s->body));memset(s->dc_x,0,sizeof(s->dc_x));memset(s->dc_y,0,sizeof(s->dc_y));
 memset(s->bass.z1,0,sizeof(s->bass.z1));memset(s->bass.z2,0,sizeof(s->bass.z2));
 memset(s->gloss.z1,0,sizeof(s->gloss.z1));memset(s->gloss.z2,0,sizeof(s->gloss.z2));
}
static unsigned disc_warmgloss_memory(const DiscWarmGloss *s){return s?(unsigned)sizeof(*s)+disc_tube2_memory(s->tube)+disc_powersag2_memory(s->sag):0;}
static void disc_warmgloss_sample(DiscWarmGloss *s,float *io){
 double dry[2],body[2],wet[2];
 for(unsigned c=0;c<2;c++){
  double x=isfinite(io[c])?io[c]:0;
  if(s->bass_on)x=warmgloss_filter(&s->bass,x,c);dry[c]=x;
 }
 if(s->sag){
  double detect=0;
  for(unsigned c=0;c<2;c++){
   s->sag_low[c]=s->sag_pole*s->sag_low[c]+(1-s->sag_pole)*dry[c];
   double level=fabs(s->sag_low[c])*.12;if(level>detect)detect=level;
  }
  if(detect>s->sag_hold)s->sag_hold=detect;
  s->sag_phase+=48000;
  if(s->sag_phase>=s->rate){
   double held=s->sag_hold;if(held<1e-12)held=1e-12;
   do{s->sag_target=disc_powersag2_linked_gain(s->sag,held);s->sag_phase-=s->rate;}while(s->sag_phase>=s->rate);
   s->sag_hold=0;
  }
  if(s->sag_target<0)s->sag_target=0;if(s->sag_target>1)s->sag_target=1;
  s->sag_gain+=s->sag_smooth*(s->sag_target-s->sag_gain);
  /* One detector and gain for both channels: heavy bass must not pan. */
  double gain=1+s->sag_amount*(s->sag_gain-1);
  for(unsigned c=0;c<2;c++)dry[c]+=(gain-1)*s->sag_low[c];
 }
 for(unsigned c=0;c<2;c++){
  double x=dry[c];
  s->body[c][0]=s->body_pole*s->body[c][0]+(1-s->body_pole)*x;
  s->body[c][1]=s->body_pole*s->body[c][1]+(1-s->body_pole)*s->body[c][0];
  wet[c]=body[c]=s->body[c][1];
 }
 if(s->amount>0)disc_tube2_sample_double_inline(s->tube,wet);
 for(unsigned c=0;c<2;c++){
  /* Do not inject the upstream denormal-prevention noise into a silent
   * channel. The core, including its RNG sequence, remains unmodified. */
  double delta=s->amount>0&&fabs(body[c])>=1.18e-23?wet[c]*s->normalize-body[c]:0;
  double ac=delta-s->dc_x[c]+s->dc_pole*s->dc_y[c];s->dc_x[c]=delta;s->dc_y[c]=ac;
  double y=dry[c]+s->amount*ac;if(s->gloss_on)y=warmgloss_filter(&s->gloss,y,c);
  io[c]=(float)(y*s->output);
 }
}
#endif
