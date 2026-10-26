typedef unsigned size_t;
extern void *memset(void *,int,unsigned),*memcpy(void *,const void *,unsigned);
#include "visual_api.h"
#include "../visual_limits.h"
#include "labels.h"
#define SIDE 360
#define LEFT 0
#define TOP 0
static unsigned canvas[129600],mixed[129600],base[129600];
static unsigned char phosphor[SIDE*SIDE];
static unsigned palette[3][256];
static short points[DISC_SCOPE_MAX_SAMPLES][2];static unsigned count,sampling,received,updated,generation,ready;
static int gain=256;
static unsigned stroke_budget;
static const DiscVisualImage image={5u|(360u<<10)|(360u<<21),518400,canvas},miximage={5u|(360u<<10)|(360u<<21),518400,mixed};
static int abs_i(int x){return x<0?-x:x;}
/* Ceiling integer square root: one calculation per block, not per point. */
static unsigned radius_ceil(unsigned n){unsigned root=0,bit=1u<<30;while(bit>n)bit>>=2;while(bit){if(n>=root+bit){n-=root+bit;root=(root>>1)+bit;}else root>>=1;bit>>=2;}return root+(n!=0);}
static void stamp(int x,int y,unsigned value){if((unsigned)x>=SIDE||(unsigned)y>=SIDE)return;unsigned i=y*SIDE+x;if(value>255)value=255;unsigned old=phosphor[i];phosphor[i]=(unsigned char)(old+(((255-old)*value+128)>>8));}
static void point(int x,int y){int ix=x>>8,iy=y>>8;unsigned fx=(unsigned)x&255,fy=(unsigned)y&255;
 stamp(ix,iy,((256-fx)*(256-fy))>>8);stamp(ix+1,iy,(fx*(256-fy))>>8);stamp(ix,iy+1,((256-fx)*fy)>>8);stamp(ix+1,iy+1,(fx*fy)>>8);
}
#define DISC_SCOPE_STAMP(c,x,y,v) stamp(x,y,v)
#define DISC_SCOPE_POINT(c,x,y) point(x,y)
#include "../scope_stroke.h"
static void segment(int x0,int y0,int x1,int y1){int dx=x1-x0,dy=y1-y0,steps=(abs_i(dx)>abs_i(dy)?abs_i(dx):abs_i(dy))>>8;if(steps<1)steps=1;
 /* Keep every native pair. Dense high-rate paths need not redraw unlimited
  * overlapping chords; after the stroke budget, retain their real dots. */
 if(count>1024){if((unsigned)steps>stroke_budget){point(x1,y1);return;}stroke_budget-=(unsigned)steps;disc_scope_stroke(0,x0,y0,x1,y1);return;}
 if(steps>128)steps=128;
 int sx=dx/steps,sy=dy/steps;for(int i=0;i<=steps;i++)point(x0+i*sx,y0+i*sy);
}
static void paint_labels(unsigned *target){ for(unsigned i=0;i<sizeof(labels)/sizeof(labels[0]);i++){const Label *l=&labels[i];for(unsigned y=0;y<l->h;y++)for(unsigned x=0;x<l->w;x++){unsigned alpha=l->alpha[y*l->w+x];if(!alpha)continue;unsigned ix=(l->y+y)*360+l->x+x;unsigned back=target[ix],out=0xff000000u;for(unsigned c=0;c<3;c++){unsigned a=(back>>(c*8))&255,b=(l->rgb>>(c*8))&255;out|=(a+(int)(b-a)*(int)alpha/255)<<(c*8);}target[ix]=out;}}
}
static void init(void){if(ready)return;ready=1;
 for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++)base[y*360+x]=0xff070c0fu;
 for(unsigned y=0;y<SIDE;y++)for(unsigned x=0;x<SIDE;x++){
  unsigned grid=(x==180||y==180)?2:(((x>0&&x<359&&x%30==0)||(y>0&&y<359&&y%30==0))?1:0);
  base[(y+TOP)*360+x+LEFT]=grid==2?0xff29424au:grid==1?0xff17272eu:0xff091116u;
 }
 for(unsigned g=0;g<3;g++)for(unsigned v=0;v<256;v++){
  unsigned b=g==2?74:g==1?46:22,r=g==2?41:g==1?23:9,green=g==2?66:g==1?39:17;
  r+=(74-r)*v/255;green+=(246-green)*v/255;b+=(191-b)*v/255;palette[g][v]=0xff000000u|(r<<16)|(green<<8)|b;
 }
 paint_labels(base);
 memcpy(canvas,base,sizeof(canvas));
}
static void reset(void){init();memset(phosphor,0,sizeof(phosphor));memcpy(canvas,base,sizeof(canvas));count=sampling=received=updated=0;gain=256;generation++;}
static void feed(const short (*stereo)[2],unsigned n,unsigned rate,unsigned t){if(!stereo||!n||n>DISC_SCOPE_MAX_SAMPLES||rate<8000||rate>768000)return;init();memcpy(points,stereo,n*sizeof(points[0]));count=n;sampling=rate;received=t;}
static void tick(unsigned t){init();if(updated&&t-updated<16)return;unsigned dt=updated?t-updated:16;updated=t;if(dt>200)dt=200;
 unsigned decay=dt>=80?128:256-dt*3/2;
 for(unsigned i=0;i<SIDE*SIDE;i++){unsigned v=phosphor[i]*decay>>8;phosphor[i]=(unsigned char)(v<3?0:v);}
 if(count&&t-received<=120){
  stroke_budget=count>1024?64000u:0xffffffffu;
  unsigned radial_peak=0;for(unsigned i=0;i<count;i++){int x=points[i][0],y=points[i][1];unsigned square=(unsigned)(x*x)+(unsigned)(y*y);if(square>radial_peak)radial_peak=square;}
  unsigned radius=radius_ceil(radial_peak);
  int target=radius>1024?32700*256/(int)radius:2048;if(target<128)target=128;if(target>2048)target=2048;
  gain+=(target-gain)*(int)dt/((target<gain?60:400)+(int)dt);
  /* A single linear scale for every paired sample preserves the XY shape.
     Fit sudden full-scale inputs immediately; all interpolation stays in
     the convex safe disk, with room for antialiasing around the trace. */
  int frame_gain=gain;if(radius){int fit=32700*256/(int)radius;if(frame_gain>fit)frame_gain=fit;}
  int px=0,py=0;
  for(unsigned i=0;i<count;i++){
   int l=points[i][0]*frame_gain/256,r=points[i][1]*frame_gain/256;
   /* Audio vectorscope: positive L goes upper-left, positive R upper-right.
      The equal 181/256 (~1/sqrt(2)) scale preserves the original radial
      fit/shape: mono is vertical, opposite polarity is horizontal. */
   int x=(r-l)*181/256,y=(l+r)*181/256;
   x=180*256+x*170/128;y=180*256-y*170/128;
   if(i)segment(px,py,x,y);else point(x,y);px=x;py=y;
  }
 }
 for(unsigned y=0;y<SIDE;y++)for(unsigned x=0;x<SIDE;x++){
  unsigned g=(x==180||y==180)?2:(((x>0&&x<359&&x%30==0)||(y>0&&y<359&&y%30==0))?1:0);
  canvas[(y+TOP)*360+x+LEFT]=palette[g][phosphor[y*SIDE+x]];
 }paint_labels(canvas);generation++;
}
static const DiscVisualImage *compose(unsigned progress,const unsigned *main){
 unsigned from_right=progress&DISC_XY_FROM_RIGHT;progress&=65535u;if(progress>360)progress=360;init();if(progress==360){return &image;}
 for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){
  if(from_right){if(x>=360-progress)mixed[y*360+x]=canvas[y*360+x+progress-360];else mixed[y*360+x]=main?(main[(359-y)*360+359-(x+progress)]|0xff000000u):0xff070c0fu;}
  else if(x<progress)mixed[y*360+x]=canvas[y*360+x+360-progress];
  else mixed[y*360+x]=main?(main[(359-y)*360+359-(x-progress)]|0xff000000u):0xff070c0fu;
 }return &miximage;
}
static unsigned revision(void){return generation;}
static void stats(unsigned *samples,unsigned *rate,unsigned *age,unsigned t){if(samples)*samples=count;if(rate)*rate=sampling;if(age)*age=received?t-received:0xffffffffu;}
static const DiscXyApi api={DISC_XY_ABI,sizeof(DiscXyApi),reset,feed,tick,compose,revision,stats};
const DiscXyApi *disc_xy_get_api(unsigned abi){return abi==DISC_XY_ABI?&api:0;}
