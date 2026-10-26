typedef unsigned size_t;
extern void *memset(void *,int,unsigned),*memcpy(void *,const void *,unsigned);
#include "polar_api.h"
#include "../visual_limits.h"
#include "geometry.h"
#define SIDE 360u
#define PIXELS 129600u
#define TRACE_POINTS 320u
static unsigned canvas[PIXELS],mixed[PIXELS],base[PIXELS];
static unsigned char left_glow[PIXELS],right_glow[PIXELS];
static unsigned palette_left[256],palette_right[256];
static short pcm[DISC_SCOPE_MAX_SAMPLES][2];
static unsigned count,sampling,received,updated,generation,ready;
static unsigned idle;
static int gain=256;
static unsigned stroke_budget;
static const DiscVisualImage image={5u|(360u<<10)|(360u<<21),518400,canvas},miximage={5u|(360u<<10)|(360u<<21),518400,mixed};
static int abs_i(int x){return x<0?-x:x;}
static void stamp(unsigned char *glow,int x,int y,unsigned value){
 if((unsigned)x>=SIDE||(unsigned)y>=SIDE)return;
 unsigned i=(unsigned)y*SIDE+(unsigned)x;int dx=x-180,dy=y-180;
 if(dx*dx+dy*dy>172*172)return;
 unsigned old=glow[i];if(value>255)value=255;
 glow[i]=(unsigned char)(old+(((255-old)*value+128)>>8));
}
static void point(unsigned char *glow,int x,int y){
 int ix=x>>8,iy=y>>8;unsigned fx=(unsigned)x&255,fy=(unsigned)y&255;
 stamp(glow,ix,iy,((256-fx)*(256-fy))>>8);stamp(glow,ix+1,iy,(fx*(256-fy))>>8);
 stamp(glow,ix,iy+1,((256-fx)*fy)>>8);stamp(glow,ix+1,iy+1,(fx*fy)>>8);
}
#define DISC_SCOPE_STAMP(c,x,y,v) stamp(c,x,y,v)
#define DISC_SCOPE_POINT(c,x,y) point(c,x,y)
#include "../scope_stroke.h"
static void segment(unsigned char *glow,int x0,int y0,int x1,int y1){
 int dx=x1-x0,dy=y1-y0,steps=(abs_i(dx)>abs_i(dy)?abs_i(dx):abs_i(dy))>>8;
 if(steps<1)steps=1;
 if(count>1024){if((unsigned)steps>stroke_budget){point(glow,x0,y0);point(glow,x1,y1);return;}stroke_budget-=(unsigned)steps;disc_scope_stroke(glow,x0,y0,x1,y1);return;}
 if(steps>136)steps=136;
 int sx=dx/steps,sy=dy/steps;for(int i=0;i<=steps;i++)point(glow,x0+i*sx,y0+i*sy);
}
static void init(void){
 if(ready)return;ready=1;
 for(unsigned i=0;i<PIXELS;i++)base[i]=polar_grid[i]==2?0xff202628u:polar_grid[i]==1?0xff121e24u:0xff070d12u;
 for(unsigned i=0;i<256;i++){
  /* Packed additions stay below 255 in every component, including the grid. */
  palette_left[i]=((24*i/255)<<16)|((150*i/255)<<8)|(140*i/255);
  palette_right[i]=((153*i/255)<<16)|((56*i/255)<<8)|(63*i/255);
 }
 memcpy(canvas,base,sizeof(canvas));
}
static void reset(void){init();memset(left_glow,0,sizeof(left_glow));memset(right_glow,0,sizeof(right_glow));memcpy(canvas,base,sizeof(canvas));count=sampling=received=updated=idle=0;gain=256;generation++;}
static void feed(const short (*stereo)[2],unsigned n,unsigned rate,unsigned t){
 if(!stereo||!n||n>DISC_SCOPE_MAX_SAMPLES||rate<8000||rate>768000)return;
 init();memcpy(pcm,stereo,n*sizeof(pcm[0]));count=n;sampling=rate;received=t;idle=0;
}
static void tick(unsigned t){
 init();if(updated&&t-updated<16)return;unsigned dt=updated?t-updated:16;updated=t;if(dt>200)dt=200;
 unsigned live=count&&t-received<=120;if(idle&&!live)return;
 unsigned decay=dt>=80?64:256-dt*2;
 unsigned lit=0;for(unsigned i=0;i<PIXELS;i++){unsigned l=left_glow[i]*decay>>8,r=right_glow[i]*decay>>8;left_glow[i]=(unsigned char)(l<3?0:l);right_glow[i]=(unsigned char)(r<3?0:r);lit|=left_glow[i]|right_glow[i];}
 if(live){
  stroke_budget=count>1024?80000u:0xffffffffu;
  unsigned peak=0;for(unsigned i=0;i<count;i++)for(unsigned c=0;c<2;c++){unsigned v=(unsigned)abs_i(pcm[i][c]);if(v>peak)peak=v;}
  int target=peak>1024?32768*256/(int)peak:2048;if(target>2048)target=2048;
  gain+=(target-gain)*(int)dt/((target<gain?60:400)+(int)dt);
  int frame_gain=gain;if(peak){int fit=32768*256/(int)peak;if(frame_gain>fit)frame_gain=fit;}
  for(unsigned c=0;c<2;c++){
   unsigned char *glow=c?right_glow:left_glow;int px=0,py=0;
   for(unsigned i=0;i<TRACE_POINTS;i++){
    /* Angle = time across the current PCM window, radius = signed amplitude.
     * One shared linear gain preserves L/R balance; no clipping/compressor. */
    unsigned position=i*(count-1)*256/(TRACE_POINTS-1),index=position>>8,fraction=position&255;
    int sample=pcm[index][c];if(index+1<count)sample+=(pcm[index+1][c]-sample)*(int)fraction/256;
    int radius=70*256+(sample*frame_gain/256)*68/128;
    int x=180*256+radius*polar_direction[i][0]/32768,y=180*256+radius*polar_direction[i][1]/32768;
    if(i)segment(glow,px,py,x,y);else point(glow,x,y);px=x;py=y;
    if(count>1024){
     /* A screen angle cannot show every high-rate oscillation. Preserve its
      * real minimum/maximum rather than selecting/averaging away transients. */
     unsigned first=i*count/TRACE_POINTS,last=(i+1)*count/TRACE_POINTS;int low=pcm[first][c],high=low;
     for(unsigned k=first+1;k<last;k++){int v=pcm[k][c];if(v<low)low=v;if(v>high)high=v;}
     int lo=70*256+(low*frame_gain/256)*68/128,hi=70*256+(high*frame_gain/256)*68/128;
     segment(glow,180*256+lo*polar_direction[i][0]/32768,180*256+lo*polar_direction[i][1]/32768,180*256+hi*polar_direction[i][0]/32768,180*256+hi*polar_direction[i][1]/32768);
    }
   }
  }
 }
 for(unsigned i=0;i<PIXELS;i++)canvas[i]=base[i]+palette_left[left_glow[i]]+palette_right[right_glow[i]];
 idle=!(live||lit);
 generation++;
}
static const DiscVisualImage *compose(unsigned progress,const unsigned *main){
 progress&=65535u;if(progress>360)progress=360;init();if(progress==360)return &image;
 /* Bottom page rises as the main page moves up. Snapshot uses the same
  * rotation-neutral, reversed storage contract as the existing XY host. */
 for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){
  if(y>=360-progress)mixed[y*360+x]=canvas[(y+progress-360)*360+x];
  else mixed[y*360+x]=main?(main[(359-(y+progress))*360+359-x]|0xff000000u):0xff070d12u;
 }
 return &miximage;
}
static unsigned revision(void){return generation;}
static void stats(unsigned *samples,unsigned *rate,unsigned *age,unsigned t){if(samples)*samples=count;if(rate)*rate=sampling;if(age)*age=received?t-received:0xffffffffu;}
static const DiscPolarApi api={DISC_POLAR_ABI,sizeof(DiscPolarApi),reset,feed,tick,compose,revision,stats};
const DiscPolarApi *disc_polar_get_api(unsigned abi){return abi==DISC_POLAR_ABI?&api:0;}
