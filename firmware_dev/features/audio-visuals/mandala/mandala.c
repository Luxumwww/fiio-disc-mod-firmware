/* A real two-dimensional DFT of the stereo phase-space density, then a
 * six-fold polar projection. No audio devices, workers, or vendor addresses. */
#include "mandala_api.h"
#include "../visual_limits.h"
#include "geometry.h"
extern void *memset(void *,int,unsigned),*memcpy(void *,const void *,unsigned);
extern float log1pf(float),sqrtf(float);
#define N 64u
#define PIXELS 129600u
static unsigned canvas[PIXELS],mixed[PIXELS];
static float density[N*N],real[N*N],imaginary[N*N],power[N*N],brightness[N*N];
static unsigned gradient[256][256];
static unsigned char target[180u*180u],smooth[180u*180u];
static short smoothing_step[511];
static unsigned smoothing_dt;
static unsigned count,rate,received,sequence,analyzed_sequence,updated,analyzed,generation,ready;
static const DiscVisualImage image={5u|(360u<<10)|(360u<<21),518400,canvas},miximage={5u|(360u<<10)|(360u<<21),518400,mixed};
static void init(void){if(ready)return;ready=1;for(unsigned hue=0;hue<256;hue++)for(unsigned v=0;v<256;v++){unsigned out=0xff000000u,color=mandala_palette[hue];for(unsigned c=0;c<3;c++){int bg=(0x050a10u>>(8*c))&255,value=(color>>(8*c))&255;out|=(unsigned)(bg+(value-bg)*(int)v/255)<<(8*c);}gradient[hue][v]=out;}for(unsigned i=0;i<PIXELS;i++)canvas[i]=0xff050a10u;}
static void reset(void){init();memset(density,0,sizeof(density));memset(power,0,sizeof(power));memset(target,0,sizeof(target));memset(smooth,0,sizeof(smooth));count=rate=received=sequence=analyzed_sequence=updated=analyzed=0;for(unsigned i=0;i<PIXELS;i++)canvas[i]=0xff050a10u;generation++;}
static void feed(const short (*pcm)[2],unsigned samples,unsigned sampling,unsigned time){
 if(!pcm||!samples||samples>DISC_SCOPE_MAX_SAMPLES||sampling<8000||sampling>768000)return;init();
 for(unsigned i=0;i<N*N;i++)density[i]*=.65f;
 float scale=1.f/samples;
 for(unsigned i=0;i<samples;i++){
  unsigned x=(unsigned)((int)pcm[i][0]+32768)>>10,y=(unsigned)((int)pcm[i][1]+32768)>>10;
  density[y*N+x]+=scale;
 }
 count=samples;rate=sampling;received=time;sequence++;
}
static void fft_line(unsigned start,unsigned stride){
 for(unsigned i=0;i<N;i++){unsigned j=reverse64[i];if(j>i){unsigned a=start+i*stride,b=start+j*stride;float t=real[a];real[a]=real[b];real[b]=t;t=imaginary[a];imaginary[a]=imaginary[b];imaginary[b]=t;}}
 for(unsigned length=2;length<=N;length*=2){unsigned half=length/2,step=N/length;
  for(unsigned s=0;s<N;s+=length)for(unsigned j=0;j<half;j++){
   unsigned a=start+(s+j)*stride,b=a+half*stride,k=j*step;
   float r=real[b]*twiddle64[k][0]-imaginary[b]*twiddle64[k][1],im=real[b]*twiddle64[k][1]+imaginary[b]*twiddle64[k][0];
   real[b]=real[a]-r;imaginary[b]=imaginary[a]-im;real[a]+=r;imaginary[a]+=im;
  }
 }
}
static void analyze(void){
 float mean=0;for(unsigned i=0;i<N*N;i++)mean+=density[i];mean/=N*N;
 for(unsigned i=0;i<N*N;i++){real[i]=density[i]-mean;imaginary[i]=0;}
 for(unsigned y=0;y<N;y++)fft_line(y*N,1);
 for(unsigned x=0;x<N;x++)fft_line(x,N);
 float peak=0;for(unsigned i=0;i<N*N;i++){power[i]=sqrtf(real[i]*real[i]+imaginary[i]*imaginary[i]);if(power[i]>peak)peak=power[i];}
 float normalization=peak>.000001f?1.f/peak:0;
 for(unsigned i=0;i<N*N;i++)brightness[i]=log1pf(24*power[i]*normalization)*79.f;
 for(unsigned y=0;y<180;y++)for(unsigned i=y*180+mandala_row_start[y],end=y*180+mandala_row_end[y];i<end;i++){
  unsigned packed=mandala_map[i];if(packed==65535){target[i]=0;continue;}
  unsigned fraction=mandala_fraction[i];float fx=(fraction&255)*(1.f/255),fy=(fraction>>8)*(1.f/255);
  float top=brightness[packed]+(brightness[packed+1]-brightness[packed])*fx,bottom=brightness[packed+64]+(brightness[packed+65]-brightness[packed+64])*fx;
  unsigned v=(unsigned)(top+(bottom-top)*fy);if(v>255)v=255;target[i]=(unsigned char)v;
 }
}
static void tick(unsigned t){
 init();if(updated&&t-updated<16)return;unsigned dt=updated?t-updated:16;updated=t;if(dt>200)dt=200;
 if(count&&t-received<=120){if(sequence!=analyzed_sequence&&(!analyzed||t-analyzed>=32)){analyze();analyzed=t;analyzed_sequence=sequence;}}
 else memset(target,0,sizeof(target));
 if(smoothing_dt!=dt){smoothing_dt=dt;for(int delta=-255;delta<=255;delta++){int step=delta*(int)dt/((delta>0?45:180)+(int)dt);if(delta&&!step)step=delta>0?1:-1;smoothing_step[delta+255]=(short)step;}}
 unsigned phase=(t/96)&255,changed=0;
 /* All four quadrants share geometry, target and temporal smoothing. */
 for(unsigned y=0;y<180;y++)for(unsigned x=mandala_row_start[y];x<mandala_row_end[y];x++){
  unsigned i=y*180+x,top=y*360,bottom=(359-y)*360;
  int delta=(int)target[i]-smooth[i];int step=smoothing_step[delta+255];
  smooth[i]=(unsigned char)((int)smooth[i]+step);unsigned out=gradient[(phase+mandala_radius[i]/2)&255][smooth[i]];
  if(out!=canvas[top+x]){canvas[top+x]=canvas[top+359-x]=canvas[bottom+x]=canvas[bottom+359-x]=out;changed=1;}
 }
 if(changed)generation++;
}
static const DiscVisualImage *compose(unsigned progress,const unsigned *main){
 progress&=65535u;if(progress>360)progress=360;init();if(progress==360)return &image;
 for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){
  if(x>=360-progress)mixed[y*360+x]=canvas[y*360+x+progress-360];
  else mixed[y*360+x]=main?(main[(359-y)*360+359-(x+progress)]|0xff000000u):0xff050a10u;
 }return &miximage;
}
static unsigned revision(void){return generation;}
static void stats(unsigned *samples,unsigned *sampling,unsigned *age,unsigned t){if(samples)*samples=count;if(sampling)*sampling=rate;if(age)*age=received?t-received:0xffffffffu;}
static const DiscMandalaApi api={DISC_MANDALA_ABI,sizeof(DiscMandalaApi),reset,feed,tick,compose,revision,stats};
const DiscMandalaApi *disc_mandala_get_api(unsigned abi){return abi==DISC_MANDALA_ABI?&api:0;}
#ifdef DISC_MANDALA_TEST
float disc_mandala_test_power(unsigned x,unsigned y){return x<N&&y<N?power[y*N+x]:0;}
unsigned disc_mandala_test_sequence(void){return analyzed_sequence;}
void disc_mandala_test_density(const float *p){memcpy(density,p,sizeof(density));analyze();}
#endif
