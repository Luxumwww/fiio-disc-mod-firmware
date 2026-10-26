#define DISC_DSP_UNITY
#define DISC_HALL_SCALAR
#include "dsp.c"
#include <assert.h>
static double harmonic(Tube *t,unsigned frequency,unsigned component){
 tube_reset(t);double real=0,imag=0;unsigned rate=48000;
 for(unsigned i=0;i<rate*3;i++){float y=tube_sample(t,.6f*sinf(6.28318530718f*frequency*i/rate),0);assert(isfinite(y));if(i>=rate*2){double angle=6.283185307179586*component*i/rate;real+=y*cos(angle);imag+=y*sin(angle);}}
 return hypot(real,imag)*2/rate;
}
int main(void){
 OutputGuard g;output_guard_init(&g,192000);float io[2];for(unsigned i=0;i<=g.delay;i++){io[0]=.4f;io[1]=-.2f;output_guard_process(&g,io);}assert(io[0]==.4f&&io[1]==-.2f&&g.gain==1&&g.limited==0);
 io[0]=2;io[1]=-1;output_guard_process(&g,io);for(unsigned i=0;i<g.delay;i++){io[0]=io[1]=.2f;output_guard_process(&g,io);}assert(io[0]<1&&io[0]>.9999f&&io[1]==-.5f*io[0]);float old=g.gain;
 for(unsigned i=0;i<192000;i++){io[0]=io[1]=.2f;output_guard_process(&g,io);assert(g.gain>=old&&g.gain<=1);old=g.gain;}assert(g.gain>.999f);
 io[0]=NAN;io[1]=INFINITY;output_guard_process(&g,io);for(unsigned i=0;i<g.delay;i++){io[0]=io[1]=0;output_guard_process(&g,io);}assert(io[0]==0&&io[1]==0&&g.invalid==2);
 Tube *t=tube_create(48000);assert(t);double h1=harmonic(t,1000,1000),h2=harmonic(t,1000,2000);assert(h2/h1>.03&&h2/h1<.5);
 tube_reset(t);float dc=0;for(unsigned i=0;i<48000*4;i++)dc=tube_sample(t,.5f,0);/* Internal dry signal retains DC; nonlinear wet signal is blocked. */assert(fabsf(dc-.175f)<.002f);
 tube_reset(t);for(unsigned i=0;i<10000;i++){float x=.5f*sinf(i*.03f);assert(tube_sample(t,x,0)==tube_sample(t,x,1));}free(t);
 const DiscDspApi *a=disc_dsp_get_api();assert(a->abi==3);void *d=a->create(192000,8,".");assert(d);unsigned available,latency,clipped;a->stats(d,&available,&latency,&clipped);assert((available&8)&&latency==382);
 int pcm[2048],copy[2048];for(unsigned i=0;i<2048;i++)pcm[i]=copy[i]=(int)(i*7919u+17u);a->set_mask(d,0);a->process(d,pcm,1024);assert(!memcmp(pcm,copy,sizeof(pcm)));
 a->set_mask(d,8);for(unsigned n=0;n<20;n++){for(unsigned i=0;i<2048;i++)pcm[i]=(i&1)?INT32_MIN:INT32_MAX;a->process(d,pcm,1024);}a->stats(d,&available,&latency,&clipped);assert(clipped==0);
 a->set_mask(d,0);for(unsigned n=0;n<20;n++){memcpy(pcm,copy,sizeof(pcm));a->process(d,pcm,1024);}memcpy(pcm,copy,sizeof(pcm));a->process(d,pcm,1024);assert(!memcmp(pcm,copy,sizeof(pcm)));a->destroy(d);
 d=a->create(384000,8,".");assert(d);a->stats(d,&available,&latency,&clipped);assert(available&8);a->destroy(d);d=a->create(768000,8,".");assert(d);a->stats(d,&available,&latency,&clipped);assert(!(available&8));a->destroy(d);
 int *whole=malloc(32791*8),*split=malloc(32791*8);assert(whole&&split);unsigned rng=1234567;for(unsigned i=0;i<32791*2;i++){rng=rng*1664525+1013904223;whole[i]=split[i]=(int)rng;}void *d1=a->create(384000,8,"."),*d2=a->create(384000,8,".");assert(d1&&d2);a->process(d1,whole,32791);for(unsigned pos=0;pos<32791;){unsigned n=pos%777+1;if(n>32791-pos)n=32791-pos;a->process(d2,split+pos*2,n);pos+=n;}assert(!memcmp(whole,split,32791*8));free(whole);free(split);a->destroy(d1);a->destroy(d2);
 printf("PASS: stereo-linked overload protection, unity below overload, recovery, invalid separation, tube H2/H1=%.4f, DC/stereo, ABI3, bit-exact bypass, mixed chunks, 384k gate\n",h2/h1);
 t=tube_create(48000);printf("alias_14k=%.9f alias_3k=%.9f\n",harmonic(t,17000,14000),harmonic(t,17000,3000));free(t);return 0;
}
