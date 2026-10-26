#define DISC_DSP_UNITY
#define DISC_HALL_SCALAR
#include "dsp.c"
#include <assert.h>
int main(void){
 const DiscDspApi *a=disc_dsp_get_api();for(unsigned rate=48000;rate<=384000;rate*=2){
  void *d=a->create(rate,16,".");assert(d);unsigned available,latency,clipped;a->stats(d,&available,&latency,&clipped);assert((available&16)&&latency==2*((rate+999)/1000-1));
  int pcm[15360]={0},reference[15360];for(unsigned n=0;n<rate/7680+2;n++)a->process(d,pcm,7680);
  double energy=0;pcm[0]=1000000000;for(unsigned n=0;n<rate*2/7680+2;n++){a->process(d,pcm,7680);for(unsigned j=0;j<15360;j++)energy+=(double)pcm[j]*pcm[j];memset(pcm,0,sizeof(pcm));}assert(energy>1e14);
  a->set_mask(d,0);for(unsigned n=0;n<rate/7680+2;n++)a->process(d,pcm,7680);for(unsigned j=0;j<15360;j++)pcm[j]=reference[j]=(j&1)?INT32_MIN+j:INT32_MAX-j;a->process(d,pcm,7680);assert(!memcmp(pcm,reference,sizeof(pcm)));
  a->set_mask(d,16);memset(pcm,0,sizeof(pcm));for(unsigned n=0;n<rate/7680+2;n++){a->process(d,pcm,7680);for(unsigned j=0;j<15360;j++)assert(!pcm[j]);}a->stats(d,&available,&latency,&clipped);assert(!clipped);a->destroy(d);
  unsigned count=32791;int *whole=malloc(count*8),*parts=malloc(count*8);assert(whole&&parts);unsigned seed=17;for(unsigned j=0;j<count*2;j++){seed=seed*1664525+1013904223;whole[j]=parts[j]=(int)seed;}void *x=a->create(rate,16,"."),*y=a->create(rate,16,".");assert(x&&y);a->process(x,whole,count);for(unsigned pos=0;pos<count;){unsigned n=pos%777+1;if(n>count-pos)n=count-pos;a->process(y,parts+pos*2,n);pos+=n;}assert(!memcmp(whole,parts,count*8));free(whole);free(parts);a->destroy(x);a->destroy(y);
 }
 void *d=a->create(768000,16,".");assert(d);unsigned av,lat,clip;a->stats(d,&av,&lat,&clip);assert(!(av&16));a->destroy(d);
 puts("PASS: V8 48/96/192/384k availability, impulse tail, reported ~2ms lookahead latency, OFF exact, reset silence, mixed chunks, 768k gate");return 0;
}
