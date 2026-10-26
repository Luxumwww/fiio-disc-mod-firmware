/* Visual-only stereo normalization. Never modifies the source/audio ring.
 * Read full container precision before scaling; one gain preserves L/R phase. */
typedef struct {float peak;unsigned stamp;} DiscVisualGain;
#include "visual_limits.h"
static unsigned disc_spectrum_step(unsigned rate){return rate>48000?(rate+47999)/48000:1;}
static unsigned disc_scope_step(unsigned rate){return rate>DISC_SCOPE_MAX_RATE?(rate+DISC_SCOPE_MAX_RATE-1)/DISC_SCOPE_MAX_RATE:1;}
static unsigned disc_scope_samples(unsigned rate){return rate/(50*disc_scope_step(rate));}
static float disc_visual_raw(const unsigned char *p,unsigned bits,unsigned index){
 if(bits==32)return ((const int *)(const void *)p)[index]*(1.0f/65536.0f);
 if(bits==24){unsigned j=index*3,v=p[j]|((unsigned)p[j+1]<<8)|((unsigned)p[j+2]<<16);return ((int)(v<<8)>>8)*(1.0f/256.0f);}
 return ((const short *)(const void *)p)[index];
}
static float disc_visual_gain(DiscVisualGain *state,const unsigned char *data,unsigned bits,unsigned channels,unsigned frames,unsigned t,int normalize){
 float peak=0;
 if(normalize)for(unsigned i=0;i<frames*channels;i++){float v=disc_visual_raw(data,bits,i);if(v<0)v=-v;if(v>peak)peak=v;}
 float gain=1;
 if(normalize&&peak>=0.5f){
  unsigned dt=state->stamp?t-state->stamp:0;if(dt>2000)dt=2000;
  if(!state->stamp||dt>=2000||peak>=state->peak)state->peak=peak;
  else state->peak+=(peak-state->peak)*(float)dt/(180.0f+dt);
  gain=24576.0f/state->peak;if(gain>4096.0f)gain=4096.0f;
 }else state->peak=0;
 state->stamp=t;
 return gain;
}
static void disc_visual_convert(DiscVisualGain *state,const unsigned char *data,unsigned bits,unsigned channels,unsigned samples,unsigned step,unsigned t,int normalize,short (*out)[2]){
 float gain=disc_visual_gain(state,data,bits,channels,samples*step,t,normalize);
 for(unsigned i=0;i<samples;i++)for(unsigned c=0;c<2;c++){
  float v=0;unsigned ch=channels==1?0:c;
  for(unsigned j=0;j<step;j++)v+=disc_visual_raw(data,bits,(i*step+j)*channels+ch);
  v=v*gain/(float)step;if(v>32767)v=32767;if(v<-32768)v=-32768;
  out[i][c]=(short)v;
 }
}
