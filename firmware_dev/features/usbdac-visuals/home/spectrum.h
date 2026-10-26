/* USB DAC home-only, source-neutral 14-band analysis.  UI thread ownership.
 * Reuses the existing DISC packed stereo FFT and fixed 4096-point tables.
 * No audio handle, thread, allocation, volume write, fake signal or DPU lease.
 * Caller owns PCM ring consistency / raw DoP detection / input conversion.
 * Any unsupported / DoP / DSD input must be passed as non-PCM before feeding.
 */
#ifndef DISC_DAC_HOME_SPECTRUM_H
#define DISC_DAC_HOME_SPECTRUM_H
#include "spectrum_tables.h"
extern float log10f(float);

#define DISC_DAC_HOME_SPECTRUM_SAMPLES 4096u
#define DISC_DAC_HOME_SPECTRUM_BANDS 14u
#define DISC_DAC_HOME_SPECTRUM_HEIGHT 103u
#define DISC_DAC_HOME_SPECTRUM_PERIOD_MS 40u
#define DISC_DAC_HOME_SPECTRUM_PCM 1u
#define DISC_DAC_HOME_SPECTRUM_NO_INPUT 0u
#define DISC_DAC_HOME_SPECTRUM_DOP 2u
#define DISC_DAC_HOME_SPECTRUM_DSD 3u

typedef struct {
 float real[DISC_DAC_HOME_SPECTRUM_SAMPLES];
 float imaginary[DISC_DAC_HOME_SPECTRUM_SAMPLES];
 int target[DISC_DAC_HOME_SPECTRUM_BANDS]; /* 8 fractional bits, pixels. */
 int smooth[DISC_DAC_HOME_SPECTRUM_BANDS];
 unsigned char height[DISC_DAC_HOME_SPECTRUM_BANDS]; /* 0..103. */
 unsigned active,input_kind,has_input,has_analysis,has_tick;
 unsigned received,analyzed,updated,rate,analyses,revision;
} DiscDacHomeSpectrum;

/* Read-only geometry reference.  Panel is (149,120,169,130); 14 * 8px
 * bars with alternated 2/3px gaps occupy x=160..306 inclusive.  Baseline
 * is screen y=237, i.e. local y=117, with maximum height 103px.  Native
 * rectangles should use y = 117-height, width 8 and the returned height.
 * A separate 1px baseline can stay visible while heights are all zero. */
static unsigned disc_dac_home_spectrum_bar_x(unsigned band){
 return band<DISC_DAC_HOME_SPECTRUM_BANDS?11u+band*139u/13u:0u;
}

static void disc_dac_home_spectrum_reset(DiscDacHomeSpectrum *s){
 if(!s)return;
 for(unsigned i=0;i<DISC_DAC_HOME_SPECTRUM_BANDS;i++)s->target[i]=s->smooth[i]=s->height[i]=0;
 s->active=s->input_kind=s->has_input=s->has_analysis=s->has_tick=0;
 s->received=s->analyzed=s->updated=s->rate=s->analyses=0;s->revision=1;
}

/* Call before reading/converting PCM.  Off-home, display-off, shade and
 * native volume overlays must pass active=0.  This clears any held bars
 * immediately; re-entry requires fresh PCM and does not reuse a snapshot. */
static int disc_dac_home_spectrum_gate(DiscDacHomeSpectrum *s,int active,unsigned input_kind,unsigned t){
 if(!s)return 0;
 unsigned permitted=active&&input_kind==DISC_DAC_HOME_SPECTRUM_PCM;
 s->active=permitted;s->input_kind=input_kind;
 if(permitted)return 0;
 int changed=0;
 for(unsigned i=0;i<DISC_DAC_HOME_SPECTRUM_BANDS;i++){
  if(s->height[i])changed=1;
  s->target[i]=s->smooth[i]=s->height[i]=0;
 }
 s->has_input=s->has_analysis=0;s->received=s->analyzed=0;s->rate=0;s->has_tick=1;s->updated=t;
 if(changed)s->revision++;
 return changed;
}

static int disc_dac_home_spectrum_due(const DiscDacHomeSpectrum *s,unsigned t){
 return s&&s->active&&s->input_kind==DISC_DAC_HOME_SPECTRUM_PCM&&
        (!s->has_analysis||t-s->analyzed>=DISC_DAC_HOME_SPECTRUM_PERIOD_MS);
}

/* Exactly 4096 same-time signed16 stereo frames, typically produced with
 * disc_visual_convert() from the existing ring.  Use the effective rate
 * after the existing averaging / disc_spectrum_step() downsampling.
 * PCM is read only.  Packed L/R recovery preserves antiphase stereo energy.
 * Returns 1 only if an actual FFT ran, otherwise performs no FFT work. */
static int disc_dac_home_spectrum_process(DiscDacHomeSpectrum *s,const short (*pcm)[2],unsigned samples,unsigned rate,unsigned t){
 if(!disc_dac_home_spectrum_due(s,t)||!pcm||samples!=DISC_DAC_HOME_SPECTRUM_SAMPLES||rate<8000||rate>768000)return 0;
 if(s->has_input&&s->rate!=rate){
  for(unsigned band=0;band<DISC_DAC_HOME_SPECTRUM_BANDS;band++)s->target[band]=s->smooth[band]=s->height[band]=0;
  s->revision++;
 }
 for(unsigned i=0;i<DISC_DAC_HOME_SPECTRUM_SAMPLES;i++){
  unsigned j=dac_home_fft_reverse[i];float w=dac_home_fft_window[i]*(1.0f/32768.0f);
  s->real[j]=pcm[i][0]*w;s->imaginary[j]=pcm[i][1]*w;
 }
 for(unsigned length=2;length<=DISC_DAC_HOME_SPECTRUM_SAMPLES;length*=2){
  unsigned half=length/2,step=DISC_DAC_HOME_SPECTRUM_SAMPLES/length;
  for(unsigned start=0;start<DISC_DAC_HOME_SPECTRUM_SAMPLES;start+=length)for(unsigned j=0;j<half;j++){
   unsigned a=start+j,b=a+half,k=j*step;float wr=dac_home_fft_cos[k],wi=dac_home_fft_sin[k];
   float r=s->real[b]*wr-s->imaginary[b]*wi,q=s->real[b]*wi+s->imaginary[b]*wr,ar=s->real[a],ai=s->imaginary[a];
   s->real[a]=ar+r;s->imaginary[a]=ai+q;s->real[b]=ar-r;s->imaginary[b]=ai-q;
  }
 }
 static const float edge[15]={25.0f,40.0f,63.0f,100.0f,160.0f,250.0f,400.0f,630.0f,1000.0f,1600.0f,2500.0f,4000.0f,6300.0f,10000.0f,20000.0f};
 const float scale=0.5f/(2047.5f*2047.5f);
 for(unsigned band=0;band<DISC_DAC_HOME_SPECTRUM_BANDS;band++){
  float low=edge[band]*DISC_DAC_HOME_SPECTRUM_SAMPLES/rate,high=edge[band+1]*DISC_DAC_HOME_SPECTRUM_SAMPLES/rate,peak=0;
  unsigned first=(unsigned)low+1,last=(unsigned)high;
  if(first<1)first=1;if(last>DISC_DAC_HOME_SPECTRUM_SAMPLES/2)last=DISC_DAC_HOME_SPECTRUM_SAMPLES/2;
  if(low<DISC_DAC_HOME_SPECTRUM_SAMPLES/2&&first<=last)for(unsigned k=first;k<=last;k++){
   unsigned mirror=(DISC_DAC_HOME_SPECTRUM_SAMPLES-k)&(DISC_DAC_HOME_SPECTRUM_SAMPLES-1u);
   float lr=s->real[k]+s->real[mirror],li=s->imaginary[k]-s->imaginary[mirror];
   float rr=s->imaginary[k]+s->imaginary[mirror],ri=s->real[mirror]-s->real[k];
   float power=(lr*lr+li*li+rr*rr+ri*ri)*scale;
   if(k==DISC_DAC_HOME_SPECTRUM_SAMPLES/2)power*=0.25f;
   if(power>peak)peak=power;
  }
  float db=peak>1.0e-12f?10.0f*log10f(peak):-120.0f;
  int height=(int)((db+66.0f)*(DISC_DAC_HOME_SPECTRUM_HEIGHT*256.0f/66.0f));
  if(height<0)height=0;if(height>(int)DISC_DAC_HOME_SPECTRUM_HEIGHT*256)height=(int)DISC_DAC_HOME_SPECTRUM_HEIGHT*256;
  s->target[band]=height;
 }
 s->has_input=s->has_analysis=1;s->received=s->analyzed=t;s->rate=rate;s->analyses++;
 return 1;
}

/* 25 Hz smoothing only, no FFT here.  A PCM producer that stopped advancing
 * falls quietly to zero after 180ms.  Return 1 only on integer-pixel changes;
 * native bars need updates only then.  No peak caps / gradient / glow. */
static int disc_dac_home_spectrum_tick(DiscDacHomeSpectrum *s,unsigned t){
 if(!s||!s->active)return 0;
 if(s->has_tick&&t-s->updated<DISC_DAC_HOME_SPECTRUM_PERIOD_MS)return 0;
 unsigned dt=s->has_tick?t-s->updated:DISC_DAC_HOME_SPECTRUM_PERIOD_MS;
 if(dt>200)dt=200;s->has_tick=1;s->updated=t;
 if(!s->has_input||t-s->received>180)for(unsigned i=0;i<DISC_DAC_HOME_SPECTRUM_BANDS;i++)s->target[i]=0;
 int changed=0;
 for(unsigned i=0;i<DISC_DAC_HOME_SPECTRUM_BANDS;i++){
  int delta=s->target[i]-s->smooth[i];
  if(delta){int step=delta*(int)dt/((delta>0?34:170)+(int)dt);if(!step)step=delta>0?1:-1;s->smooth[i]+=step;}
  unsigned height=(unsigned)(s->smooth[i]+128)>>8;
  if(height>DISC_DAC_HOME_SPECTRUM_HEIGHT)height=DISC_DAC_HOME_SPECTRUM_HEIGHT;
  if(s->height[i]!=height){s->height[i]=(unsigned char)height;changed=1;}
 }
 if(changed)s->revision++;
 return changed;
}
#endif
