/* Real production input path: native paired scopes and display-only FFT FIR. */
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "visual_normalize.h"
#include "visual_spectrum_input.h"
static int raw[65536][2];
static short output[DISC_SCOPE_MAX_SAMPLES][2];
static void tone(unsigned rate,unsigned frames,double frequency){for(unsigned i=0;i<frames;i++){int v=(int)(12000*sin(6.283185307179586*frequency*i/rate));raw[i][0]=v*65536;raw[i][1]=-v*65536;}}
static double amplitude(double f,double rate){double ss=0,cc=0,sc=0,ys=0,yc=0;for(unsigned i=128;i<4096-128;i++){double s=sin(6.283185307179586*f*i/rate),c=cos(6.283185307179586*f*i/rate),v=output[i][0];ss+=s*s;cc+=c*c;sc+=s*c;ys+=v*s;yc+=v*c;}double d=ss*cc-sc*sc,a=(ys*cc-yc*sc)/d,b=(yc*ss-ys*sc)/d;return sqrt(a*a+b*b);}
static double cpu(void){struct timespec t;clock_gettime(2,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(void){
 for(unsigned ri=0;ri<4;ri++){
  unsigned rate=(unsigned[]){96000,192000,352800,384000}[ri],n=disc_scope_samples(rate);assert(disc_scope_step(rate)==1&&n==rate/50&&n<=DISC_SCOPE_MAX_SAMPLES);
  for(unsigned i=0;i<n;i++){raw[i][0]=((int)(i*137%60000)-30000)*65536;raw[i][1]=((int)(i*257%60000)-30000)*65536;}
  for(unsigned i=0;i<DISC_SCOPE_MAX_SAMPLES;i++)output[i][0]=output[i][1]=1234;
  DiscVisualGain g={0};disc_visual_convert(&g,(void *)raw,32,2,n,1,1000,0,output);
  for(unsigned i=0;i<n;i++)for(unsigned c=0;c<2;c++)assert(output[i][c]==raw[i][c]/65536);
  for(unsigned i=n;i<DISC_SCOPE_MAX_SAMPLES;i++)assert(output[i][0]==1234&&output[i][1]==1234);
 }
 for(unsigned ri=0;ri<4;ri++){
  unsigned rate=(unsigned[]){176400,192000,352800,384000}[ri],step=disc_spectrum_step(rate),frames=4096*step;
  for(unsigned fi=0;fi<5;fi++){
   double f=(double[]){30,1000,10000,18000,20000}[fi];tone(rate,frames,f);DiscVisualGain g={0};disc_visual_spectrum_convert(&g,(void *)raw,32,2,4096,step,1000,0,output);
   double a=amplitude(f,rate/step);assert(a>11880&&a<12120);
   for(unsigned i=128;i<4096-128;i++)assert(output[i][0]==-output[i][1]);
  }
  double effective=(double)rate/step;tone(rate,frames,effective-20000);DiscVisualGain g={0};disc_visual_spectrum_convert(&g,(void *)raw,32,2,4096,step,2000,0,output);
  assert(amplitude(20000,effective)<12000*.0018); /* >54.9dB alias rejection at20k. */
 }
 for(unsigned rate=192000;rate<=384000;rate*=2){
  unsigned step=disc_spectrum_step(rate);tone(rate,4096*step,18000);DiscVisualGain g={0};double begin=cpu();
  for(unsigned k=0;k<4;k++)disc_visual_spectrum_convert(&g,(void *)raw,32,2,4096,step,3000+k*40,1,output);
  printf("filter_rate=%u average_cpu_ms=%.3f\n",rate,(cpu()-begin)*250);
 }
 puts("PASS native192/384k pairs and20ms window, bounds,30Hz..20kHz amplitude, stereo antiphase, display FFT alias rejection");fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
