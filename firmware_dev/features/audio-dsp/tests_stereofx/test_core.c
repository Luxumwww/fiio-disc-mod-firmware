#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "../stereofx_core.h"
#include "../stereofx_params.h"
void *stereofx_reference_create(unsigned,const float *);
void stereofx_reference_sample(void *,double *);
void stereofx_reference_destroy(void *);
int main(void){unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1110,count=0;
 for(unsigned r=0;r<6;r++)for(unsigned v=0;v<8;v++){
  float p[12];memcpy(p,disc_stereofx_default,sizeof(p));if(v>=1&&v<=6)p[(v-1)/2]=disc_stereofx_range[(v-1)/2][(v-1)&1];if(v==7)p[0]=p[2]=1,p[1]=.7f;
  DiscStereoFx *s=disc_stereofx_create(rates[r],p);void *ref=stereofx_reference_create(rates[r],p);assert(s&&ref);
  for(unsigned i=0;i<16000;i++){
   rng=rng*1664525u+1013904223u;double l=(int)rng/2147483648.;rng=rng*1664525u+1013904223u;double right=(int)rng/2147483648.;
   if(i<2000)right=l;if(i>=2000&&i<4000)right=-l;if(i>=4000&&i<6000)l=right=0;if(i>=6000&&i<8000)l*=4,right*=4;
   double in[2]={l,right},out[2],expected[2]={l,right};disc_stereofx_process_double(s,in,out,1);stereofx_reference_sample(ref,expected);
   for(unsigned c=0;c<2;c++){if(out[c]!=expected[c]){fprintf(stderr,"FAIL rate%u case%u frame%u ch%u %.17g %.17g\n",rates[r],v,i,c,out[c],expected[c]);return 2;}assert(isfinite(out[c]));}
  }
  disc_stereofx_reset(s);DiscStereoFx *fresh=disc_stereofx_create(rates[r],p);for(unsigned i=0;i<128;i++){double in[2]={0,0},a[2],b[2];disc_stereofx_process_double(s,in,a,1);disc_stereofx_process_double(fresh,in,b,1);assert(!memcmp(a,b,sizeof(a)));}disc_stereofx_destroy(fresh);disc_stereofx_destroy(s);stereofx_reference_destroy(ref);count++;
 }
 for(unsigned r=0;r<6;r++){
  DiscStereoFx *a=disc_stereofx_create(rates[r],disc_stereofx_default),*b=disc_stereofx_create(rates[r],disc_stereofx_default);double side_in=0,side_out=0;
  for(unsigned i=0;i<16000;i++){double l=.02*sin(i*.071),rr=.017*sin(i*.037),in[2]={l,rr},sw[2]={rr,l},out[2],rev[2];disc_stereofx_process_double(a,in,out,1);disc_stereofx_process_double(b,sw,rev,1);assert(out[0]==rev[1]&&out[1]==rev[0]);if(fabs(l)>=1.18e-23&&fabs(rr)>=1.18e-23)assert(fabs((out[0]+out[1])-(l+rr))<1e-15);side_in+=(l-rr)*(l-rr);side_out+=(out[0]-out[1])*(out[0]-out[1]);}assert(side_out>side_in*1.8);printf("rate=%u default_side_rms_ratio=%.6f mid_preserved\n",rates[r],sqrt(side_out/side_in));disc_stereofx_destroy(a);disc_stereofx_destroy(b);
  a=disc_stereofx_create(rates[r],disc_stereofx_default);for(unsigned i=0;i<16000;i++){double in[2]={.1*sin(i*.071),.1*sin(i*.071)},out[2];disc_stereofx_process_double(a,in,out,1);assert(out[0]==out[1]);if(fabs(in[0])>=1.18e-23)assert(out[0]==in[0]);}disc_stereofx_destroy(a);
 }
 for(unsigned j=0;j<12;j++){float p[12];memcpy(p,disc_stereofx_default,sizeof(p));p[j]=NAN;assert(!disc_stereofx_create(48000,p));}assert(!disc_stereofx_create(768000,disc_stereofx_default));
 printf("PASS %u contexts/6 rates: exact double equality to unmodified Airwindows StereoFXProc.cpp; endpoints/mono/antiphase/silence/overload/reset; default mid preserved, swap symmetric, measurable side expansion\n",count);return 0;
}
