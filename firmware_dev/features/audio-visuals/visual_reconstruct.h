/* Finite display reconstruction for 8..48k scopes. Never an audio resampler.
 * 32 samples of real context at both ends avoid synthetic endpoint chords. */
#include "visual_reconstruct_coefficients.h"
static unsigned disc_scope_padding(unsigned rate){return rate>=8000&&rate<=48000?32:0;}
static unsigned disc_visual_scope_convert(DiscVisualGain *state,const unsigned char *data,unsigned bits,unsigned channels,unsigned samples,unsigned rate,unsigned t,int normalize,short (*out)[2]){
 unsigned pad=disc_scope_padding(rate);
 if(!pad){disc_visual_convert(state,data,bits,channels,samples,disc_scope_step(rate),t,normalize,out);return samples;}
 if(samples>960)return 0;
 static float decoded[1024][2],reconstructed[3840][2];
 unsigned n=samples+64;float gain=disc_visual_gain(state,data,bits,channels,n,t,normalize),peak=0;
 for(unsigned i=0;i<n;i++)for(unsigned c=0;c<2;c++)decoded[i][c]=disc_visual_raw(data,bits,i*channels+(channels==1?0:c));
 for(unsigned i=0;i<samples;i++){
  float quarter[2]={0,0},half[2]={0,0},third[2]={0,0};
  for(unsigned j=0;j<32;j++){
   const float *lo=decoded[i+1+j],*hi=decoded[i+64-j];
   float a=disc_reconstruct_quarter_left[j],b=disc_reconstruct_quarter_right[j],h=disc_reconstruct_half[j];
   for(unsigned c=0;c<2;c++){quarter[c]+=a*lo[c]+b*hi[c];third[c]+=b*lo[c]+a*hi[c];half[c]+=h*(lo[c]+hi[c]);}
  }
  for(unsigned c=0;c<2;c++){
   reconstructed[i*4][c]=decoded[i+32][c];reconstructed[i*4+1][c]=quarter[c];reconstructed[i*4+2][c]=half[c];reconstructed[i*4+3][c]=third[c];
   for(unsigned k=0;k<4;k++){float v=reconstructed[i*4+k][c];if(v<0)v=-v;if(v>peak)peak=v;}
  }
 }
 /* One shared scale keeps overshoot from clipping either side of the shape. */
 if(peak*gain>32760.f)gain=32760.f/peak;
 for(unsigned i=0;i<samples*4;i++)for(unsigned c=0;c<2;c++)out[i][c]=(short)(reconstructed[i][c]*gain);
 return samples*4;
}
