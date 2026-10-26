#define DISC_DSP_UNITY
#define DISC_HALL_SCALAR
#include "dsp.c"
#include <assert.h>
int main(void){
 const DiscDspApi *api=disc_dsp_get_api();unsigned rates[]={8000,44100,48000,96000,192000,384000},bits[]={32,64,128};
 for(unsigned ri=0;ri<6;ri++)for(unsigned bi=0;bi<3;bi++){
  unsigned rate=rates[ri],bit=bits[bi],av,lat,clip;void *d=api->create(rate,bit,".");assert(d);api->stats(d,&av,&lat,&clip);assert((av&bit)&&lat==2*((rate+999)/1000-1));
  int pcm[2048]={0};for(unsigned i=0;i<rate/1024+1;i++)api->process(d,pcm,1024);
  pcm[0]=1200000000;double energy=0,stereo=0;
  for(unsigned i=0;i<rate*2/1024+1;i++){api->process(d,pcm,1024);for(unsigned j=0;j<1024;j++){energy+=(double)pcm[2*j]*pcm[2*j]+(double)pcm[2*j+1]*pcm[2*j+1];stereo+=(double)pcm[2*j+1]*pcm[2*j+1];}memset(pcm,0,sizeof(pcm));}
  assert(energy>1e14);if(bit!=32)assert(stereo>1e10);
  api->set_mask(d,0);for(unsigned i=0;i<rate/1024+1;i++)api->process(d,pcm,1024);
  int original[2048];for(unsigned j=0;j<2048;j++)original[j]=pcm[j]=(j&1)?INT32_MIN+j:INT32_MAX-j;api->process(d,pcm,1024);assert(!memcmp(original,pcm,sizeof(pcm)));
  api->set_mask(d,bit);memset(pcm,0,sizeof(pcm));for(unsigned i=0;i<rate/1024+1;i++){api->process(d,pcm,1024);for(unsigned j=0;j<2048;j++)assert(pcm[j]==0);}
  unsigned limited,invalid;float peak,gain;api->diagnostics(d,&limited,&invalid,&peak,&gain);assert(!invalid);api->stats(d,&av,&lat,&clip);assert(!clip);api->destroy(d);
  unsigned frames=16391;int *a=malloc(frames*8),*b=malloc(frames*8);unsigned seed=21;
  for(unsigned j=0;j<frames*2;j++){seed=1664525*seed+1013904223;a[j]=b[j]=(int)seed/2;}
  void *x=api->create(rate,bit,"."),*y=api->create(rate,bit,".");assert(x&&y);api->process(x,a,frames);
  for(unsigned pos=0;pos<frames;){unsigned n=pos%511+1;if(n>frames-pos)n=frames-pos;api->process(y,b+pos*2,n);pos+=n;}
  assert(!memcmp(a,b,frames*8));api->diagnostics(x,&limited,&invalid,&peak,&gain);assert(!invalid&&isfinite(peak));api->destroy(x);api->destroy(y);free(a);free(b);
  printf("PASS single effect=%u rate=%u: tail/stereo, OFF exact, reset silence, chunk invariance, finite bounded output\n",bit,rate);
 }
 for(unsigned bi=0;bi<3;bi++){void *d=api->create(768000,bits[bi],".");unsigned av,lat,clip;assert(d);api->stats(d,&av,&lat,&clip);assert(!(av&bits[bi]));api->destroy(d);}
 puts("PASS new effects 768k gate; original output guard parameters retained");return 0;
}
