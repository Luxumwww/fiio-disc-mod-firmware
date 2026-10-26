#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../spectrum.h"

static DiscDacHomeSpectrum state;
static short pcm[DISC_DAC_HOME_SPECTRUM_SAMPLES][2];
static void tone(unsigned rate,float frequency,float amplitude,int kind){
 for(unsigned i=0;i<DISC_DAC_HOME_SPECTRUM_SAMPLES;i++){
  int sample=(int)(sinf((float)(6.2831853071795864769*frequency*i/rate))*amplitude*32767.0f);
  pcm[i][0]=(short)sample;pcm[i][1]=kind==1?(short)-sample:kind==2?0:(short)sample;
 }
}
static unsigned strongest(void){unsigned result=0;for(unsigned i=1;i<14;i++)if(state.target[i]>state.target[result])result=i;return result;}
static void ready(unsigned t){disc_dac_home_spectrum_reset(&state);disc_dac_home_spectrum_gate(&state,1,DISC_DAC_HOME_SPECTRUM_PCM,t);}
int main(void){
 ready(0);tone(48000,1500,0.7f,0);
 assert(disc_dac_home_spectrum_process(&state,pcm,4096,48000,0)==1);
 assert(state.analyses==1&&strongest()==8);
 int coherent=state.target[8];
 /* Stereo inversion must preserve power; mono-left should be ~3dB lower. */
 ready(40);tone(48000,1500,0.7f,1);assert(disc_dac_home_spectrum_process(&state,pcm,4096,48000,40));
 assert(abs(state.target[8]-coherent)<=1);
 ready(80);tone(48000,1500,0.7f,2);assert(disc_dac_home_spectrum_process(&state,pcm,4096,48000,80));
 int attenuation=coherent-state.target[8];assert(attenuation>1100&&attenuation<1300);
 /* Both sample-rate families and low-rate Nyquist exclusion. */
 ready(120);tone(44100,344.53125f,0.6f,0);assert(disc_dac_home_spectrum_process(&state,pcm,4096,44100,120));assert(strongest()==5);
 ready(160);tone(8000,1000,0.6f,0);assert(disc_dac_home_spectrum_process(&state,pcm,4096,8000,160));assert(strongest()==7);
 for(unsigned i=11;i<14;i++)assert(!state.target[i]);
 /* No processing while covered / DoP / DSD / no input. */
 for(unsigned kind=0;kind<=3;kind++)if(kind!=DISC_DAC_HOME_SPECTRUM_PCM){
  unsigned before=state.analyses;disc_dac_home_spectrum_gate(&state,1,kind,200);
  assert(!disc_dac_home_spectrum_due(&state,200));assert(!disc_dac_home_spectrum_process(&state,pcm,4096,48000,200));assert(state.analyses==before);
 }
 disc_dac_home_spectrum_gate(&state,0,DISC_DAC_HOME_SPECTRUM_PCM,240);assert(!disc_dac_home_spectrum_process(&state,pcm,4096,48000,240));
 for(unsigned i=0;i<14;i++)assert(!state.height[i]);
 /* Real frame throttling, invalid buffers, stale stream decay, wrap-safe time. */
 ready(0);tone(48000,1500,0.8f,0);assert(disc_dac_home_spectrum_process(&state,pcm,4096,48000,0));
 assert(!disc_dac_home_spectrum_process(&state,pcm,4096,48000,39));assert(state.analyses==1);
 assert(!disc_dac_home_spectrum_process(&state,0,4096,48000,40));assert(!disc_dac_home_spectrum_process(&state,pcm,1024,48000,40));
 assert(disc_dac_home_spectrum_tick(&state,0));assert(state.height[8]>0&&state.height[8]<=103);
 for(unsigned t=40;t<=5000;t+=40)disc_dac_home_spectrum_tick(&state,t);
 for(unsigned i=0;i<14;i++)assert(!state.height[i]);
 ready(0xfffffff0u);assert(disc_dac_home_spectrum_process(&state,pcm,4096,48000,0xfffffff0u));
 assert(!disc_dac_home_spectrum_due(&state,0x10u));assert(disc_dac_home_spectrum_due(&state,0x18u));
 /* Frequency sweep stays in the correct broad band and all output bounds. */
 static const float edge[15]={25,40,63,100,160,250,400,630,1000,1600,2500,4000,6300,10000,20000};
 for(unsigned band=0;band<14;band++){
  ready(100);tone(48000,sqrtf(edge[band]*edge[band+1]),0.8f,band&1);
  assert(disc_dac_home_spectrum_process(&state,pcm,4096,48000,100));
  assert(strongest()==band);disc_dac_home_spectrum_tick(&state,100);
  for(unsigned i=0;i<14;i++)assert(state.height[i]<=103);
 }
 /* Zero input stays zero; native bar geometry is inside 169x130. */
 ready(100);memset(pcm,0,sizeof(pcm));assert(disc_dac_home_spectrum_process(&state,pcm,4096,48000,100));
 assert(!disc_dac_home_spectrum_tick(&state,100));
 for(unsigned i=0;i<14;i++){assert(!state.target[i]&&!state.height[i]);assert(disc_dac_home_spectrum_bar_x(i)+8<=169);}
 printf("home spectrum: all tests passed; state=%u bytes; FFT=4096; bands=14; period=40ms\n",(unsigned)sizeof(state));
 return 0;
}
#ifdef DISC_DAC_HOME_SPECTRUM_PRELOAD
/* Test-only entry: never call the original application's main. */
int __libc_start_main(void *entry,int argc,char **argv,void *init,void *fini,void *rtld,void *stack){
 (void)entry;(void)argc;(void)argv;(void)init;(void)fini;(void)rtld;(void)stack;
 int result=main();fflush(stdout);_Exit(result);
}
#endif
