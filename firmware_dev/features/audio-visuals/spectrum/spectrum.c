typedef unsigned size_t;
extern void *memset(void *,int,unsigned),*memcpy(void *,const void *,unsigned);
extern float log10f(float);
#include "spectrum_api.h"
#include "geometry.h"
#define PIXELS 129600u
static unsigned canvas[PIXELS],mixed[PIXELS],base[PIXELS];
static short pcm[SPECTRUM_N][2];
static float real[SPECTRUM_N],imaginary[SPECTRUM_N],power_left[2049],power_right[2049];
static unsigned count,sampling,received,updated,analyzed,generation,ready,feed_sequence,analyzed_sequence;
static int target[SPECTRUM_BANDS],smooth[SPECTRUM_BANDS],drawn[SPECTRUM_BANDS];
static int peaks[SPECTRUM_BANDS],old_peaks[SPECTRUM_BANDS],bass_target,bass_smooth;
static unsigned peak_time[SPECTRUM_BANDS],color_phase,previous_phase,ring_drawn;
static const DiscVisualImage image={5u|(360u<<10)|(360u<<21),518400,canvas},miximage={5u|(360u<<10)|(360u<<21),518400,mixed};
static void init(void){
 if(ready)return;ready=1;
 for(unsigned i=0;i<PIXELS;i++)base[i]=0xff070d12u;
 for(unsigned i=0;i<sizeof(spectrum_grid)/sizeof(spectrum_grid[0]);i++)base[spectrum_grid[i]]=0xff16272du;
 memcpy(canvas,base,sizeof(canvas));
}
static void reset(void){
 init();memcpy(canvas,base,sizeof(canvas));memset(pcm,0,sizeof(pcm));memset(target,0,sizeof(target));memset(smooth,0,sizeof(smooth));memset(drawn,0,sizeof(drawn));memset(peaks,0,sizeof(peaks));memset(old_peaks,0,sizeof(old_peaks));memset(peak_time,0,sizeof(peak_time));memset(power_left,0,sizeof(power_left));memset(power_right,0,sizeof(power_right));bass_target=bass_smooth=0;color_phase=previous_phase=ring_drawn=0;count=sampling=received=updated=analyzed=feed_sequence=analyzed_sequence=0;generation++;
}
static void feed(const short (*stereo)[2],unsigned n,unsigned rate,unsigned t){
 if(!stereo||n!=SPECTRUM_N||rate<8000||rate>768000)return;
 init();memcpy(pcm,stereo,sizeof(pcm));count=n;sampling=rate;received=t;feed_sequence++;
}
static void fft_analyze(void){
 /* One complex FFT packs L into real and R into imaginary. Recover each
  * real-channel spectrum from the conjugate bins; never average PCM channels,
  * which would incorrectly cancel an antiphase stereo signal. */
 for(unsigned i=0;i<SPECTRUM_N;i++){unsigned j=fft_reverse[i];float w=fft_window[i]*(1.0f/32768.0f);real[j]=pcm[i][0]*w;imaginary[j]=pcm[i][1]*w;}
 for(unsigned length=2;length<=SPECTRUM_N;length*=2){unsigned half=length/2,step=SPECTRUM_N/length;
  for(unsigned start=0;start<SPECTRUM_N;start+=length)for(unsigned j=0;j<half;j++){
   unsigned a=start+j,b=a+half,k=j*step;float wr=fft_cos[k],wi=fft_sin[k];
   float r=real[b]*wr-imaginary[b]*wi,q=real[b]*wi+imaginary[b]*wr,ar=real[a],ai=imaginary[a];
   real[a]=ar+r;imaginary[a]=ai+q;real[b]=ar-r;imaginary[b]=ai-q;
  }
 }
 const float scale=1.0f/(2047.5f*2047.5f); /* 4/sum(Hann)^2, then /4 for stereo unpack */
 for(unsigned k=0;k<=SPECTRUM_N/2;k++){
  unsigned mirror=(SPECTRUM_N-k)&(SPECTRUM_N-1);
  float lr=real[k]+real[mirror],li=imaginary[k]-imaginary[mirror];
  float rr=imaginary[k]+imaginary[mirror],ri=real[mirror]-real[k];
  power_left[k]=(lr*lr+li*li)*scale;power_right[k]=(rr*rr+ri*ri)*scale;
 }
 /* DC/Nyquist have only one side and use the matching half amplitude. */
 power_left[0]*=0.25f;power_right[0]*=0.25f;power_left[SPECTRUM_N/2]*=0.25f;power_right[SPECTRUM_N/2]*=0.25f;
 for(unsigned band=0;band<SPECTRUM_BANDS;band++){
  float low=spectrum_frequency[band]*SPECTRUM_N/sampling,high=spectrum_frequency[band+1]*SPECTRUM_N/sampling,peak=0;
  unsigned first=(unsigned)low+1,last=(unsigned)high;if(first<1)first=1;if(last>SPECTRUM_N/2)last=SPECTRUM_N/2;
  if(low<SPECTRUM_N/2){
   if(first<=last)for(unsigned k=first;k<=last;k++){float p=(power_left[k]+power_right[k])*0.5f;if(p>peak)peak=p;}
   else {float center=(low+high)*0.5f;unsigned k=(unsigned)center;if(k>=1&&k<SPECTRUM_N/2){float f=center-k;peak=((power_left[k]+power_right[k])*(1.0f-f)+(power_left[k+1]+power_right[k+1])*f)*0.5f;}}
  }
  float db=peak>1.0e-12f?10.0f*log10f(peak):-120.0f;
  int height=(int)((db+72.0f)*(146.0f*256.0f/72.0f));if(height<0)height=0;if(height>146*256)height=146*256;target[band]=height;
 }
 float bass=0;unsigned low=(20u*SPECTRUM_N+sampling-1)/sampling,high=180u*SPECTRUM_N/sampling;if(low<1)low=1;
 for(unsigned k=low;k<=high&&k<=SPECTRUM_N/2;k++){float v=(power_left[k]+power_right[k])*0.5f;if(v>bass)bass=v;}
 float db=bass>1.0e-12f?10.0f*log10f(bass):-120.0f;
 bass_target=(int)((db+60.0f)*(256.0f/54.0f));if(bass_target<0)bass_target=0;if(bass_target>256)bass_target=256;
}
static int overlap(int start,int end,int cell){int a=start>cell?start:cell,b=end<cell+256?end:cell+256;return b>a?b-a:0;}
static void paint_cell(unsigned band,unsigned cell){
 int body=smooth[band]-(int)cell*256;if(body<0)body=0;if(body>256)body=256;
 int cap=peaks[band]?overlap(peaks[band]+256,peaks[band]+768,(int)cell*256):0;
 unsigned color=spectrum_palette[(color_phase+band*2+cell/12)&255],level=144+cell*112/147,out_color=0;
 for(unsigned c=0;c<3;c++){unsigned v=(color>>(c*8))&255;out_color|=(v*level/256)<<(c*8);}
 if(cap>body){body=cap;unsigned pastel=0;for(unsigned c=0;c<3;c++){unsigned v=(color>>(c*8))&255;pastel|=(160+v*95/255)<<(c*8);}out_color=pastel;}
 for(unsigned j=spectrum_offsets[band][cell];j<spectrum_offsets[band][cell+1];j++){
  unsigned index=spectrum_pixels[j],alpha=(spectrum_alpha[j]*(unsigned)body+128)>>8;
  if(!alpha){canvas[index]=base[index];continue;}
  unsigned background=base[index],out=0xff000000u;
  for(unsigned c=0;c<3;c++){unsigned a=(background>>(c*8))&255,b=(out_color>>(c*8))&255;out|=((a*(255-alpha)+b*alpha+127)/255)<<(c*8);}
  canvas[index]=out;
 }
}
static void paint_range(unsigned band,int start,int end){
 if(start<0)start=0;unsigned first=(unsigned)start>>8,last=((unsigned)end+255)>>8;
 if(last>=SPECTRUM_CELLS)last=SPECTRUM_CELLS-1;for(unsigned cell=first;cell<=last;cell++)paint_cell(band,cell);
}
static void paint_bar(unsigned band,int color_changed){
 int old=drawn[band],height=smooth[band],peak=peaks[band],before=old_peaks[band];
 if(color_changed&&(height||old))paint_range(band,0,height>old?height:old);
 else if(old!=height)paint_range(band,old<height?old:height,old>height?old:height);
 if(before!=peak||color_changed){if(before)paint_range(band,before+256,before+768);if(peak)paint_range(band,peak+256,peak+768);}
 drawn[band]=height;old_peaks[band]=peak;
}
static void paint_rings(void){
 for(unsigned j=0;j<sizeof(center_pixels)/sizeof(center_pixels[0]);j++){
  int radius=center_radii[j],alpha=0;unsigned color=0;
  for(int ring=0;ring<3;ring++){
   int distance=radius-((6+ring*6)*256+bass_smooth*(ring+2));if(distance<0)distance=-distance;
   int edge=320-distance;if(edge<=0)continue;
   int a=edge*bass_smooth/320;if(a>alpha){alpha=a;color=spectrum_palette[(color_phase+ring*30+120)&255];}
  }
  if(alpha>220)alpha=220;unsigned index=center_pixels[j],background=base[index],out=0xff000000u;
  for(unsigned c=0;c<3;c++){unsigned a=(background>>(c*8))&255,b=(color>>(c*8))&255;out|=((a*(255-(unsigned)alpha)+b*(unsigned)alpha+127)/255)<<(c*8);}canvas[index]=out;
 }
}
static void tick(unsigned t){
 init();if(updated&&t-updated<16)return;unsigned dt=updated?t-updated:16;updated=t;if(dt>200)dt=200;
 if(count&&t-received<=120){if(feed_sequence!=analyzed_sequence&&(!analyzed||t-analyzed>=32)){fft_analyze();analyzed=t;analyzed_sequence=feed_sequence;}}
 else {for(unsigned band=0;band<SPECTRUM_BANDS;band++)target[band]=0;bass_target=0;}
 color_phase=(t/64)&255;int color_changed=color_phase!=previous_phase;previous_phase=color_phase;
 int delta_bass=bass_target-bass_smooth;
 if(delta_bass){int step=delta_bass*(int)dt/((delta_bass>0?50:230)+(int)dt);if(!step)step=delta_bass>0?1:-1;bass_smooth+=step;}
 int changed=0;
 for(unsigned band=0;band<SPECTRUM_BANDS;band++){
  int delta=target[band]-smooth[band],tau=delta>0?40:190;
  if(delta){int step=delta*(int)dt/(tau+(int)dt);if(!step)step=delta>0?1:-1;smooth[band]+=step;}
  int limit=smooth[band];if(limit>144*256)limit=144*256;
  if(limit>=peaks[band]){peak_time[band]=t;peaks[band]=limit;}
  else if(t-peak_time[band]>320){peaks[band]-=45*256*(int)dt/1000;if(peaks[band]<limit)peaks[band]=limit;}
  if(smooth[band]!=drawn[band]||peaks[band]!=old_peaks[band]||(color_changed&&(smooth[band]||peaks[band]))){paint_bar(band,color_changed);changed=1;}
 }
 if((unsigned)bass_smooth!=ring_drawn||(color_changed&&bass_smooth)){paint_rings();ring_drawn=(unsigned)bass_smooth;changed=1;}
 if(changed)generation++;
}
static const DiscVisualImage *compose(unsigned progress,const unsigned *main){
 progress&=65535u;if(progress>360)progress=360;init();if(progress==360)return &image;
 for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){
  if(x<progress)mixed[y*360+x]=canvas[y*360+x+360-progress];
  else mixed[y*360+x]=main?(main[(359-y)*360+359-(x-progress)]|0xff000000u):0xff070d12u;
 }return &miximage;
}
static unsigned revision(void){return generation;}
static void stats(unsigned *samples,unsigned *rate,unsigned *age,unsigned t){if(samples)*samples=count;if(rate)*rate=sampling;if(age)*age=received?t-received:0xffffffffu;}
static const DiscSpectrumApi api={DISC_SPECTRUM_ABI,sizeof(DiscSpectrumApi),reset,feed,tick,compose,revision,stats};
const DiscSpectrumApi *disc_spectrum_get_api(unsigned abi){return abi==DISC_SPECTRUM_ABI?&api:0;}
#ifdef DISC_SPECTRUM_TEST
float disc_spectrum_test_power(unsigned channel,unsigned bin){return bin<=SPECTRUM_N/2?(channel?power_right[bin]:power_left[bin]):0;}
int disc_spectrum_test_height(unsigned band){return band<SPECTRUM_BANDS?target[band]:0;}
unsigned disc_spectrum_test_analysis_sequence(void){return analyzed_sequence;}
int disc_spectrum_test_peak(unsigned band){return band<SPECTRUM_BANDS?peaks[band]:0;}
int disc_spectrum_test_smooth(unsigned band){return band<SPECTRUM_BANDS?smooth[band]:0;}
int disc_spectrum_test_bass(void){return bass_smooth;}
#endif
