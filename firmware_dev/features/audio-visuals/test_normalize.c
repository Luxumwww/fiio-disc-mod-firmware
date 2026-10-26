#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "visual_normalize.h"
static int raw[65536*2];static short out[4096][2];
int main(void){
 DiscVisualGain g={0};
 /* 40dB source-volume difference, below short precision: normalize before truncation. */
 for(int i=0;i<960;i++){raw[2*i]=(int)(sin(i*.031)*16000*65536);raw[2*i+1]=-raw[2*i]/2;}
 disc_visual_convert(&g,(void *)raw,32,2,960,1,100,1,out);short reference[960][2];for(int i=0;i<960;i++)for(int c=0;c<2;c++)reference[i][c]=out[i][c];
 for(int i=0;i<1920;i++)raw[i]/=100;g=(DiscVisualGain){0};disc_visual_convert(&g,(void *)raw,32,2,960,1,100,1,out);
 for(int i=0;i<960;i++)for(int c=0;c<2;c++)assert(abs(out[i][c]-reference[i][c])<=2);
 for(int i=0;i<960;i++)assert(abs(out[i][0]+2*out[i][1])<=2);
 g=(DiscVisualGain){0};disc_visual_convert(&g,(void *)raw,32,2,960,1,100,0,out);assert(abs(out[50][0])<161);
 for(int i=0;i<1920;i++)raw[i]=0;disc_visual_convert(&g,(void *)raw,32,2,960,1,120,1,out);for(int i=0;i<960;i++)assert(!out[i][0]&&!out[i][1]);
 /* Stereo box decimation preserves a real30Hz low-frequency tone at192k. */
 for(int i=0;i<16384;i++){raw[2*i]=(int)(sin(2*3.141592653589793*30*i/192000)*12000*65536);raw[2*i+1]=raw[2*i];}
 assert(disc_spectrum_step(192000)==4&&disc_spectrum_step(768000)==16);
 assert(disc_scope_samples(44100)==882&&disc_scope_samples(48000)==960&&disc_scope_samples(96000)==1920&&disc_scope_samples(192000)==3840&&disc_scope_samples(352800)==7056);
 disc_visual_convert(&g,(void *)raw,32,2,4096,4,200,0,out);
 for(int i=0;i<4096;i++)assert(abs(out[i][0]-(int)(sin(2*3.141592653589793*30*(i*4+1.5)/192000)*12000))<2);
 assert(disc_scope_samples(384000)==7680&&disc_spectrum_step(384000)==8);
 for(int i=0;i<7680;i++){raw[i*2]=(i&1)?2147483647:(-2147483647-1);raw[i*2+1]=raw[i*2];}
 g=(DiscVisualGain){0};disc_visual_convert(&g,(void *)raw,32,2,960,8,300,1,out);for(int i=0;i<960;i++)assert(out[i][0]==out[i][1]&&out[i][0]>=-24576&&out[i][0]<=24576);
 for(int i=0;i<7680;i++){raw[i*2]=(int)(sin(2*3.141592653589793*1000*i/384000)*32000*65536);raw[i*2+1]=(int)(cos(2*3.141592653589793*1000*i/384000)*16000*65536);}
 g=(DiscVisualGain){0};disc_visual_convert(&g,(void *)raw,32,2,960,8,400,1,out);
 for(int i=0;i<960;i++){double radius=(double)out[i][0]*out[i][0]+4.0*out[i][1]*out[i][1];assert(radius>24576.0*24576*.98&&radius<24576.0*24576*1.02);}
 puts("PASS: source volume40dB invariance, stereo phase/ratio, optional raw amplitude, silence,192k low-frequency decimation");
}
