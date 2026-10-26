#include "spectrum_core.h"
#include "spectrum_tables.h"
static DiscSpectrumCore core;
static short samples[DISC_FFT_SIZE];
#ifdef DISC_HOST_TEST
#include <stdio.h>
static void report(const char *s){fputs(s,stdout);}
#else
static void report(const char *s){
  unsigned n=0;while(s[n])n++;
  register long v0 __asm__("$2")=4004,a0 __asm__("$4")=1,a1 __asm__("$5")=(long)s,a2 __asm__("$6")=n;
  __asm__ volatile("syscall":"+r"(v0):"r"(a0),"r"(a1),"r"(a2):"$7","memory");
}
#endif
int disc_native_selftest(void){
  for(unsigned i=0;i<DISC_FFT_SIZE;i++) {
    unsigned k=(i*16)%1024;samples[i]=k<512?disc_cos[k]/2:-disc_cos[k-512]/2;
  }
  disc_spectrum_analyze(&core,samples,48000);
  unsigned peak=1;
  for(unsigned k=2;k<512;k++)if(core.magnitude[k]>core.magnitude[peak])peak=k;
  if(peak!=16||core.magnitude[peak]<3000)return 1;
  for(unsigned i=0;i<DISC_FFT_SIZE;i++)samples[i]=0;
  for(unsigned j=0;j<100;j++)disc_spectrum_analyze(&core,samples,48000);
  for(unsigned j=0;j<DISC_BANDS;j++)if(core.levels[j])return 2;
  const unsigned char s16[]={0,0x40,0,0x40},s24[]={0,0,0x40,0xaa,0,0,0x40,0xbb},s24p[]={0,0,0x40,0,0,0x40},s32[]={0,0,0,0x40,0,0,0,0x40};short out;
  if(disc_pcm_mono(s16,2,2,&out)!=4||out!=16384)return 3;
  if(disc_pcm_mono(s24,2,6,&out)!=8||out!=16384)return 4;
  if(disc_pcm_mono(s24p,2,32,&out)!=6||out!=16384)return 5;
  if(disc_pcm_mono(s32,2,10,&out)!=8||out!=16384)return 6;
  if(disc_pcm_mono(s32,3,10,&out)||disc_pcm_mono(s32,2,48,&out))return 7;
  disc_spectrum_analyze(&core,samples,0);
  report("DISC_NATIVE_CORE_OK fft=750Hz bin=16 silence=pass pcm=S16/S24/S24_3/S32 float=none\n");return 0;
}
#ifdef DISC_HOST_TEST
int main(void){return disc_native_selftest();}
#endif
