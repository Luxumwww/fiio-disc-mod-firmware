#ifndef DISC_VISUAL_SPECTRUM_INPUT_H
#define DISC_VISUAL_SPECTRUM_INPUT_H
#include "visual_filter_coefficients.h"
static void disc_visual_filter_select(unsigned factor,const float **coeff,unsigned *pairs,float *center){if(factor>=8){*coeff=disc_visual_halfband_first;*pairs=4;*center=disc_visual_halfband_first_center;}else if(factor>=4){*coeff=disc_visual_halfband_middle;*pairs=8;*center=disc_visual_halfband_middle_center;}else{*coeff=disc_visual_halfband;*pairs=24;*center=disc_visual_halfband_center;}}
/* Both channels share indices and bounds. The interior has no endpoint checks;
 * the three fixed kernels let the compiler specialize the tap count. Keep the
 * original symmetric-pair accumulation order for identical S16 conversion. */
#define DISC_VISUAL_FILTER_STAGE(PAIRS) \
static void disc_visual_filter_stage_##PAIRS(float (*destination)[2],const float (*source)[2],unsigned n,const float *coeff,float center){ \
 unsigned next=n/2,begin=PAIRS<next?PAIRS:next,distance=PAIRS*2-1; \
 unsigned end=n>distance?(n-distance+1)/2:0;if(end<begin)end=begin; \
 for(unsigned i=0;i<begin;i++){ \
  unsigned at=i*2;float left=center*source[at][0],right=center*source[at][1]; \
  for(unsigned j=0;j<PAIRS;j++){unsigned d=j*2+1,lo=at>=d?at-d:0,hi=at+d<n?at+d:n-1;left+=coeff[j]*(source[lo][0]+source[hi][0]);right+=coeff[j]*(source[lo][1]+source[hi][1]);} \
  destination[i][0]=left;destination[i][1]=right; \
 } \
 for(unsigned i=begin;i<end;i++){ \
  unsigned at=i*2;float left=center*source[at][0],right=center*source[at][1]; \
  for(unsigned j=0;j<PAIRS;j++){unsigned d=j*2+1;left+=coeff[j]*(source[at-d][0]+source[at+d][0]);right+=coeff[j]*(source[at-d][1]+source[at+d][1]);} \
  destination[i][0]=left;destination[i][1]=right; \
 } \
 for(unsigned i=end;i<next;i++){ \
  unsigned at=i*2;float left=center*source[at][0],right=center*source[at][1]; \
  for(unsigned j=0;j<PAIRS;j++){unsigned d=j*2+1,lo=at>=d?at-d:0,hi=at+d<n?at+d:n-1;left+=coeff[j]*(source[lo][0]+source[hi][0]);right+=coeff[j]*(source[lo][1]+source[hi][1]);} \
  destination[i][0]=left;destination[i][1]=right; \
 } \
}
DISC_VISUAL_FILTER_STAGE(4)
DISC_VISUAL_FILTER_STAGE(8)
DISC_VISUAL_FILTER_STAGE(24)
#undef DISC_VISUAL_FILTER_STAGE
static void disc_visual_filter_stage(float (*destination)[2],const float (*source)[2],unsigned n,unsigned factor){
 const float *coeff;unsigned pairs;float center;disc_visual_filter_select(factor,&coeff,&pairs,&center);
 if(pairs==4)disc_visual_filter_stage_4(destination,source,n,coeff,center);
 else if(pairs==8)disc_visual_filter_stage_8(destination,source,n,coeff,center);
 else disc_visual_filter_stage_24(destination,source,n,coeff,center);
}
/* Finite snapshot conversion, never a streaming audio resampler. Symmetric
 * pairs are filtered identically. Hann FFT edges suppress endpoint padding.
 * The common44.1/48k families use cascaded2x stages; legacy non-power-of-two
 * source rates retain the previous conversion instead of mislabelling rate. */
static void disc_visual_spectrum_convert(DiscVisualGain *state,const unsigned char *data,unsigned bits,unsigned channels,unsigned samples,unsigned step,unsigned t,int normalize,short (*out)[2]){
 if(!step||step>16||samples>DISC_VISUAL_FFT_SAMPLES)return;
 if(step==1||(step&(step-1))){disc_visual_convert(state,data,bits,channels,samples,step,t,normalize,out);return;}
 static float first[32768][2],second[16384][2];
 unsigned n=samples*step,next=n/2;float gain=disc_visual_gain(state,data,bits,channels,n,t,normalize);
 float (*source)[2]=first,(*destination)[2]=second;unsigned factor=step;
 if(n<=32768){
  /* Decode each native sample once instead of once per FIR tap. This covers
   * all supported audio rates through384k without growing the workspace. */
  if(channels==1)for(unsigned i=0;i<n;i++)first[i][0]=first[i][1]=disc_visual_raw(data,bits,i);
  else for(unsigned i=0;i<n;i++){first[i][0]=disc_visual_raw(data,bits,i*channels);first[i][1]=disc_visual_raw(data,bits,i*channels+1);}
 }else{
  /* The optional16x input exceeds the full native scratch capacity. Retain
   * its bounded first stage; its half-size result fits the same workspace. */
  const float *coeff;unsigned pairs;float center;disc_visual_filter_select(step,&coeff,&pairs,&center);
  for(unsigned i=0;i<next;i++)for(unsigned c=0;c<2;c++){
   unsigned at=i*2,ch=channels==1?0:c;float sum=center*disc_visual_raw(data,bits,at*channels+ch);
   for(unsigned j=0;j<pairs;j++){unsigned d=j*2+1,lo=at>=d?at-d:0,hi=at+d<n?at+d:n-1;sum+=coeff[j]*(disc_visual_raw(data,bits,lo*channels+ch)+disc_visual_raw(data,bits,hi*channels+ch));}
   first[i][c]=sum;
  }
  n=next;factor/=2;
 }
 for(;factor>1;factor/=2){
  disc_visual_filter_stage(destination,(const float (*)[2])source,n,factor);
  float (*swap)[2]=source;source=destination;destination=swap;n/=2;
 }
 for(unsigned i=0;i<samples;i++)for(unsigned c=0;c<2;c++){float v=source[i][c]*gain;if(v>32767)v=32767;if(v<-32768)v=-32768;out[i][c]=(short)v;}
}
#endif
