/* Polar sample vectorscope: rectified M/S vectors, dots instead of a waveform.
 * Input is only the host's existing read-only PCM snapshot. No audio output. */
typedef unsigned size_t;
extern void *memset(void *,int,unsigned),*memcpy(void *,const void *,unsigned);
#include "polar_api.h"
#include "../visual_limits.h"
#include "stereo_assets.h"
static unsigned canvas[129600],mixed[129600],base[129600],palette[3][256];
static unsigned char phosphor[129600];
static short pcm[DISC_SCOPE_MAX_SAMPLES][2];
static unsigned count,sampling,received,updated,generation,ready;
static unsigned idle;
static const unsigned meter_rgb[5]={0x20323c,0x4af6bf,0x42cddd,0xec8e79,0xd4e8e6};
static unsigned meter_palette[5][256];
static int gain=256,correlation,correlation_valid;
static unsigned level_q8[2],peak_q8[2],peak_until[2];
static const DiscVisualImage image={5u|(360u<<10)|(360u<<21),518400,canvas},miximage={5u|(360u<<10)|(360u<<21),518400,mixed};
static unsigned root_ceil(unsigned n){unsigned root=0,bit=1u<<30;while(bit>n)bit>>=2;while(bit){if(n>=root+bit){n-=root+bit;root=(root>>1)+bit;}else root>>=1;bit>>=2;}return root+(n!=0);}
static void stamp(int x,int y,unsigned v){if((unsigned)x>=360||(unsigned)y>=360)return;int dx=x-180,dy=180-y;if(dy<0||dx*dx+dy*dy>177*177)return;unsigned i=(unsigned)y*360+(unsigned)x,old=phosphor[i];if(v>255)v=255;phosphor[i]=(unsigned char)(old+(((255-old)*v+128)>>8));canvas[i]=phosphor[i]?palette[stereo_grid[i]][phosphor[i]]:base[i];}
static void point(int x,int y){int ix=x>>8,iy=y>>8;unsigned fx=(unsigned)x&255,fy=(unsigned)y&255;stamp(ix,iy,((256-fx)*(256-fy))>>8);stamp(ix+1,iy,(fx*(256-fy))>>8);stamp(ix,iy+1,((256-fx)*fy)>>8);stamp(ix+1,iy+1,(fx*fy)>>8);}
static void labels(unsigned *target){for(unsigned i=0;i<sizeof(stereo_labels)/sizeof(stereo_labels[0]);i++){const StereoLabel *l=&stereo_labels[i];for(unsigned y=0;y<l->h;y++)for(unsigned x=0;x<l->w;x++){unsigned a=l->alpha[y*l->w+x];if(!a)continue;unsigned ix=(l->y+y)*360+l->x+x,b=target[ix],v=0xff000000;for(unsigned c=0;c<3;c++){int lo=(int)((b>>(c*8))&255),hi=(0x91afb3>>(c*8))&255;v|=(unsigned)(lo+(hi-lo)*(int)a/255)<<(c*8);}target[ix]=v;}}}
static void correlation_text(char text[6]){if(!correlation_valid){text[0]=text[1]='-';text[2]=0;return;}unsigned v=(unsigned)(correlation<0?-correlation:correlation);v=(v*100+512)/1024;if(v>100)v=100;text[0]=correlation<0?'-':'+';text[1]=(char)('0'+v/100);text[2]='.';text[3]=(char)('0'+v/10%10);text[4]=(char)('0'+v%10);text[5]=0;}
static const StereoGlyph *number(char ch){for(unsigned i=0;i<sizeof(stereo_numbers)/sizeof(stereo_numbers[0]);i++)if(stereo_numbers[i].ch==ch)return &stereo_numbers[i];return 0;}
static void readout(unsigned *target){char text[6];correlation_text(text);unsigned width=0,rgb=correlation_valid?(correlation<0?0xec8e79:0x4af6bf):0x91afb3;for(unsigned i=0;text[i];i++){const StereoGlyph *g=number(text[i]);if(g)width+=g->w;}unsigned left=180-width/2;
 for(unsigned i=0;text[i];i++){const StereoGlyph *g=number(text[i]);if(!g)continue;for(unsigned y=0;y<g->h;y++)for(unsigned x=0;x<g->w;x++){unsigned a=g->alpha[y*g->w+x];if(!a)continue;unsigned ix=(242+y)*360+left+x,b=target[ix],v=0xff000000;for(unsigned c=0;c<3;c++){int lo=(int)((b>>(c*8))&255),hi=(int)((rgb>>(c*8))&255);v|=(unsigned)(lo+(hi-lo)*(int)a/255)<<(c*8);}target[ix]=v;}left+=g->w;}
}
static unsigned meter_bin(float energy){unsigned lo=0,hi=STEREO_METER_BINS;while(lo<hi){unsigned mid=(lo+hi)/2;if(energy>=stereo_meter_energy[mid])lo=mid+1;else hi=mid;}return lo;}
static unsigned meter_pixel(unsigned rgb,unsigned a){unsigned v=0xff000000;for(unsigned c=0;c<3;c++){int lo=(0x091116>>(c*8))&255,hi=(int)((rgb>>(c*8))&255);v|=(unsigned)(lo+(hi-lo)*(int)a/255)<<(c*8);}return v;}
static void meter_segment(unsigned *target,unsigned channel,unsigned index,unsigned rgb){unsigned slot=0;while(slot<5&&meter_rgb[slot]!=rgb)slot++;const unsigned *colors=slot<5?meter_palette[slot]:0;const StereoSegment *s=&stereo_meter_segments[channel*STEREO_METER_BINS+index];for(unsigned j=0;j<s->count;j++){unsigned p=stereo_meter_pixels[s->start+j];target[p>>8]=colors?colors[p&255]:meter_pixel(rgb,p&255);}}
static unsigned meter_color(unsigned channel,unsigned index){return index>=250?0xec8e79:channel?0x42cddd:0x4af6bf;}
static unsigned mix_rgb(unsigned lo,unsigned hi,unsigned fraction){unsigned out=0;for(unsigned c=0;c<3;c++){int a=(int)((lo>>(c*8))&255),b=(int)((hi>>(c*8))&255);out|=(unsigned)(a+(b-a)*(int)fraction/256)<<(c*8);}return out;}
static void meter_overlay(unsigned *target,unsigned channel,unsigned index,unsigned rgb,unsigned opacity){if(index>=STEREO_METER_BINS)return;const StereoSegment *s=&stereo_meter_segments[channel*STEREO_METER_BINS+index];for(unsigned j=0;j<s->count;j++){unsigned p=stereo_meter_pixels[s->start+j],a=((p&255)*opacity+128)>>8,ix=p>>8;target[ix]=0xff000000|mix_rgb(target[ix],rgb,a);}}
static void meters(unsigned *target){for(unsigned c=0;c<2;c++){
 unsigned n=level_q8[c]>>8,fraction=level_q8[c]&255;if(n>STEREO_METER_BINS)n=STEREO_METER_BINS;
 for(unsigned i=0;i<n;i++)meter_segment(target,c,i,meter_color(c,i));
 if(n<STEREO_METER_BINS&&fraction)meter_segment(target,c,n,mix_rgb(0x20323c,meter_color(c,n),fraction));
 unsigned p=peak_q8[c]>>8,part=peak_q8[c]&255;if(p>STEREO_METER_BINS)p=STEREO_METER_BINS;
 if(peak_q8[c]){for(unsigned j=1;j<=4;j++)if(p>=j)meter_segment(target,c,p-j,0xd4e8e6);if(p>=5)meter_overlay(target,c,p-5,0xd4e8e6,256-part);if(part&&p<STEREO_METER_BINS)meter_overlay(target,c,p,0xd4e8e6,part);}
}}
static unsigned follow(unsigned old,unsigned wanted,unsigned dt,unsigned rise,unsigned fall){int delta=(int)wanted-(int)old,step=delta*(int)dt/(int)((delta>0?rise:fall)+dt);if(delta&&!step)step=delta>0?1:-1;return (unsigned)((int)old+step);}
static void meter_update(unsigned t,unsigned dt,const unsigned rms[2]){if(dt>1000)dt=1000;for(unsigned c=0;c<2;c++){
 unsigned wanted=rms[c]*256;level_q8[c]=dt>=1000?wanted:follow(level_q8[c],wanted,dt,28,180);if(level_q8[c]<8)level_q8[c]=0;
 /* The cap is the held maximum of the VISIBLE bar, not a second detector.
  * It can only rise when the bar touches it. Return remains fractional. */
 unsigned q=level_q8[c];if(dt>=1000||q>=peak_q8[c]){peak_q8[c]=q;peak_until[c]=t+300;}else if((int)(t-peak_until[c])>=0){unsigned fall=(dt*256*96+999)/1000;peak_q8[c]=peak_q8[c]>q+fall?peak_q8[c]-fall:q;}
}}
static void clear_readouts(void){
 for(unsigned j=0;j<sizeof(stereo_labels)/sizeof(stereo_labels[0]);j++){const StereoLabel *l=&stereo_labels[j];if(l->y>180)for(unsigned y=0;y<l->h;y++)memcpy(canvas+(l->y+y)*360+l->x,base+(l->y+y)*360+l->x,l->w*sizeof(unsigned));}
 for(unsigned y=242;y<273;y++)memcpy(canvas+y*360+125,base+y*360+125,110*sizeof(unsigned));
 for(unsigned y=278;y<290;y++)memcpy(canvas+y*360+97,base+y*360+97,167*sizeof(unsigned));
 for(unsigned i=0;i<sizeof(stereo_meter_pixels)/sizeof(stereo_meter_pixels[0]);i++){unsigned ix=stereo_meter_pixels[i]>>8;canvas[ix]=base[ix];}
}
static void init(void){if(ready)return;ready=1;
 for(unsigned c=0;c<5;c++)for(unsigned a=0;a<256;a++)meter_palette[c][a]=meter_pixel(meter_rgb[c],a);
 for(unsigned i=0;i<129600;i++)base[i]=stereo_grid[i]==2?0xff29424a:stereo_grid[i]==1?0xff17272e:0xff091116;
 for(unsigned g=0;g<3;g++)for(unsigned v=0;v<256;v++){unsigned r=g==2?41:g==1?23:9,green=g==2?66:g==1?39:17,b=g==2?74:g==1?46:22;r+=(74-r)*v/255;green+=(246-green)*v/255;b+=(191-b)*v/255;palette[g][v]=0xff000000|(r<<16)|(green<<8)|b;}
 for(unsigned y=282;y<=285;y++)for(unsigned x=99;x<=261;x++)base[y*360+x]=x==180?0xff63838a:0xff203039;
 for(unsigned c=0;c<2;c++)for(unsigned i=0;i<STEREO_METER_BINS;i++)meter_segment(base,c,i,0x20323c);
 labels(base);memcpy(canvas,base,sizeof canvas);readout(canvas);
}
static void reset(void){init();memset(phosphor,0,sizeof phosphor);memcpy(canvas,base,sizeof canvas);count=sampling=received=updated=idle=0;gain=256;correlation=correlation_valid=0;memset(level_q8,0,sizeof level_q8);memset(peak_q8,0,sizeof peak_q8);memset(peak_until,0,sizeof peak_until);readout(canvas);generation++;}
static void feed(const short (*stereo)[2],unsigned n,unsigned rate,unsigned t){if(!stereo||!n||n>DISC_SCOPE_MAX_SAMPLES||rate<8000||rate>768000)return;init();memcpy(pcm,stereo,n*sizeof pcm[0]);count=n;sampling=rate;received=t;idle=0;}
static void tick(unsigned t){
 init();if(updated&&t-updated<16)return;unsigned dt=updated?t-updated:16;updated=t;
 unsigned live=count&&t-received<=120;if(idle&&!live)return;
 unsigned decay=dt>=170?0:256-dt*3/2;
 /* Only the upper half can contain trace pixels. Decay and raster it in one
  * pass; stamps update their own pixels. Lower static pixels stay untouched. */
 unsigned lit=0;for(unsigned i=0;i<181*360;i++){unsigned v=phosphor[i]*decay>>8;phosphor[i]=(unsigned char)(v<3?0:v);lit|=phosphor[i];canvas[i]=phosphor[i]?palette[stereo_grid[i]][phosphor[i]]:base[i];}
 clear_readouts();
 unsigned rms_bins[2]={0,0};
 if(live){
  unsigned peak=0;float ll=0,rr=0,lr=0;
  for(unsigned i=0;i<count;i++){int l=pcm[i][0],r=pcm[i][1];unsigned p=(unsigned)(l*l)+(unsigned)(r*r);if(p>peak)peak=p;float lf=(float)l,rf=(float)r;ll+=lf*lf;rr+=rf*rf;lr+=lf*rf;}
  rms_bins[0]=meter_bin(ll/(float)count);rms_bins[1]=meter_bin(rr/(float)count);
  unsigned radius=root_ceil(peak);int target=radius>1024?32700*256/(int)radius:2048;if(target>2048)target=2048;
  unsigned smooth_dt=dt>200?200:dt;gain+=(target-gain)*(int)smooth_dt/((target<gain?60:400)+(int)smooth_dt);
  int frame_gain=gain;if(radius){int fit=32700*256/(int)radius;if(frame_gain>fit)frame_gain=fit;}
  for(unsigned i=0;i<count;i++){
   int l=pcm[i][0]*frame_gain/256,r=pcm[i][1]*frame_gain/256,m=l+r,s=r-l;
   /* Fold negative M into the upper half-plane. A single L/R pair keeps its
    * original angle/radius: mono straight up, L/R at +/-45, anti-phase wide. */
   if(m<0){m=-m;s=-s;}
   int x=s*181/256,y=m*181/256;
   point(180*256+x*175/128,180*256-y*175/128);
  }
  /* Hardware FPU retains quiet-signal correlation; no 64-bit divider. */
  float denom=__builtin_sqrtf(ll*rr);
  correlation_valid=denom>0;int target_c=denom>0?(int)(lr*1024.0f/denom):0;
  if(target_c>1024)target_c=1024;if(target_c<-1024)target_c=-1024;
  correlation+=(target_c-correlation)*(int)smooth_dt/(90+(int)smooth_dt);
 }else correlation_valid=0;
 meter_update(t,dt,rms_bins);
 labels(canvas);
 if(correlation_valid){int x=180+correlation*81/1024;for(int y=278;y<=289;y++)for(int k=-1;k<=1;k++)canvas[y*360+x+k]=correlation<0?0xffec8e79:0xff4af6bf;}
 readout(canvas);
 meters(canvas);
 idle=!(live||lit||level_q8[0]||level_q8[1]||peak_q8[0]||peak_q8[1]);
 generation++;
}
static const DiscVisualImage *compose(unsigned progress,const unsigned *main){progress&=65535;if(progress>360)progress=360;init();if(progress==360)return &image;for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++)mixed[y*360+x]=y>=360-progress?canvas[(y+progress-360)*360+x]:main?(main[(359-y-progress)*360+359-x]|0xff000000):0xff091116;return &miximage;}
static unsigned revision(void){return generation;}
static void stats(unsigned *samples,unsigned *rate,unsigned *age,unsigned t){if(samples)*samples=count;if(rate)*rate=sampling;if(age)*age=received?t-received:0xffffffff;}
static const DiscStereoApi api={DISC_STEREO_ABI,sizeof(DiscStereoApi),reset,feed,tick,compose,revision,stats};
const DiscStereoApi *disc_stereo_get_api(unsigned abi){return abi==DISC_STEREO_ABI?&api:0;}
