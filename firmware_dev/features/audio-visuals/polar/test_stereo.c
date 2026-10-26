/* Actual production renderer, host sanitizers and original MIPS32 ABI. */
#define size_t renderer_size_t
#include "stereo.c"
#undef size_t
#include <assert.h>
#include <stdio.h>
static short input[1024][2];
static unsigned main_frame[129600];
static unsigned char arc_seen[129600];
static int n,sum_x,sum_y,min_x,max_x,min_y,max_y;
static void verify_full_raster(void){
 memcpy(main_frame,canvas,sizeof canvas);
 for(unsigned i=0;i<129600;i++)canvas[i]=phosphor[i]?palette[stereo_grid[i]][phosphor[i]]:base[i];
 labels(canvas);if(correlation_valid){int x=180+correlation*81/1024;for(int y=278;y<=289;y++)for(int k=-1;k<=1;k++)canvas[y*360+x+k]=correlation<0?0xffec8e79:0xff4af6bf;}
 readout(canvas);meters(canvas);for(unsigned i=0;i<129600;i++)if(canvas[i]!=main_frame[i]){fprintf(stderr,"raster mismatch x=%u y=%u full=%x sparse=%x updated=%u\n",i%360,i/360,canvas[i],main_frame[i],updated);assert(0);}
} /* Sparse redraw must be pixel-identical to rebuilding the full frame. */
static void measure(void){
 verify_full_raster();
 n=sum_x=sum_y=0;min_x=min_y=999;max_x=max_y=-999;
 for(int y=0;y<360;y++)for(int x=0;x<360;x++)if(phosphor[y*360+x]){
  int dx=x-180,dy=180-y;assert(dy>=0&&dx*dx+dy*dy<=177*177);
  assert((2*x-359)*(2*x-359)+(2*y-359)*(2*y-359)<359*359);
  sum_x+=dx;sum_y+=dy;n++;
  if(dx<min_x)min_x=dx;if(dx>max_x)max_x=dx;if(dy<min_y)min_y=dy;if(dy>max_y)max_y=dy;
 }
}
int main(void){
#ifdef __mips__
 assert(sizeof(DiscStereoApi)==32&&sizeof(DiscVisualImage)==12);
#endif
 assert(!disc_stereo_get_api(2)&&disc_stereo_get_api(1)==&api);
 assert(stereo_grid[5*360+180]&&stereo_grid[180*360+5]&&stereo_grid[180*360+355]); /* Standard semicircle fills the top half. */
 for(unsigned j=1;j<=3;j++){const StereoLabel *l=&stereo_labels[j];for(unsigned y=0;y<l->h;y++)for(unsigned x=0;x<l->w;x++)if(l->alpha[y*l->w+x]){int dx=(int)(l->x+x)-180,dy=(int)(l->y+y)-180;assert(dy<0&&dx*dx+dy*dy<175*175);}}
 for(unsigned i=0;i<sizeof(stereo_meter_pixels)/sizeof(stereo_meter_pixels[0]);i++){unsigned p=stereo_meter_pixels[i]>>8;assert(p<129600&&!arc_seen[p]);arc_seen[p]=1;int x=(int)(p%360),y=(int)(p/360);assert(y>180&&(2*x-359)*(2*x-359)+(2*y-359)*(2*y-359)<359*359);}
 char text[6];correlation_valid=1;correlation=1024;correlation_text(text);assert(!__builtin_strcmp(text,"+1.00"));correlation=-1024;correlation_text(text);assert(!__builtin_strcmp(text,"-1.00"));correlation=0;correlation_text(text);assert(!__builtin_strcmp(text,"+0.00"));correlation_valid=0;correlation_text(text);assert(!__builtin_strcmp(text,"--"));
 for(int kind=0;kind<7;kind++){
  reset();for(int i=0;i<1024;i++){
   int l=kind<2?-30000+(i%128)*472:(kind<4?20000:-20000),r=kind==0?l:kind==1?-l:0;
   if(kind==3||kind==5){r=l;l=0;}
   if(kind==6){l=(i*7919%60000)-30000;r=(i*3911%60000)-30000;}
   input[i][0]=l;input[i][1]=r;
  }
  for(unsigned t=1000;t<2500;t+=16){feed(input,1024,768000,t);tick(t);}measure();assert(n);
  if(kind==0)assert(min_x>=-1&&max_x<=1&&max_y>170&&correlation>990);
  else if(kind==1)assert(min_y==0&&max_y<=1&&min_x<-170&&max_x>170&&correlation<-990);
  else if(kind<6){assert((sum_x<0)==(kind==2||kind==4));assert(sum_y>0);assert((sum_x<0?-sum_x:sum_x)-sum_y<n*2);assert(!correlation_valid);}
  else assert(min_x<-90&&max_x>90&&max_y>100);
  if(kind<2)assert(level_q8[0]==level_q8[1]&&peak_q8[0]==peak_q8[1]);
  if(kind==2||kind==4)assert(level_q8[0]>190*256&&!level_q8[1]&&peak_q8[0]>190*256&&!peak_q8[1]);
  if(kind==3||kind==5)assert(level_q8[1]>190*256&&!level_q8[0]&&peak_q8[1]>190*256&&!peak_q8[0]);
  for(unsigned t=2800;t<4000;t+=16)tick(t);measure();assert(!n&&!correlation_valid);
 }
 reset();feed(input,DISC_SCOPE_MAX_SAMPLES+1,48000,1);assert(!count);feed(input,1024,7999,1);assert(!count);feed(input,1024,768001,1);assert(!count);
 for(int i=0;i<1024;i++)input[i][0]=input[i][1]=(i&1)?1:-1;
 for(unsigned t=1000;t<2500;t+=16){feed(input,1024,48000,t);tick(t);}assert(correlation_valid&&correlation>990); /* Quiet mono must not quantize to silence. */
 for(int i=0;i<1024;i++)input[i][0]=input[i][1]=0;
 feed(input,1024,48000,2510);tick(2510);assert(!correlation_valid);
 for(unsigned i=0;i<129600;i++)main_frame[i]=0xff000000|(i%256);
 const unsigned *out=compose(0,main_frame)->data;for(unsigned i=0;i<129600;i++)assert(out[i]==main_frame[129599-i]);
 out=compose(180,main_frame)->data;assert(out[120*360+130]==main_frame[(359-300)*360+359-130]);assert(out[250*360+130]==canvas[70*360+130]);
 reset();for(int i=0;i<1024;i++){input[i][0]=-32768;input[i][1]=-32768;}feed(input,1024,192000,1000);tick(1000);measure();assert(n&&max_y<=177&&min_x>=-1&&max_x<=1);
 tick(2000);measure();assert(!n); /* Long invisible interval clears old trace. */
 reset();unsigned rms[2]={200,100};meter_update(1000,1000,rms);assert(peak_q8[0]==level_q8[0]&&peak_q8[1]==level_q8[1]);unsigned held=peak_q8[0];rms[0]=rms[1]=0;meter_update(1016,16,rms);assert(peak_q8[0]==held&&level_q8[0]<held);meter_update(1299,16,rms);assert(peak_q8[0]==held);meter_update(1316,16,rms);assert(peak_q8[0]<held&&peak_q8[0]>=level_q8[0]);meter_update(5000,1000,rms);assert(!peak_q8[0]&&!level_q8[0]);reset();assert(!peak_q8[0]&&!peak_q8[1]&&!level_q8[0]&&!level_q8[1]);
 for(unsigned t=1000;t<6000;t+=16){unsigned previous[2]={peak_q8[0],peak_q8[1]};for(unsigned i=0;i<1024;i++){input[i][0]=(short)(i*137+t*5);input[i][1]=(short)(i*257-t*3);}if(t%700<400)feed(input,1024,48000,t);tick(t);verify_full_raster();for(unsigned c=0;c<2;c++){assert(peak_q8[c]>=level_q8[c]);if(peak_q8[c]>previous[c])assert(peak_q8[c]==level_q8[c]);}}
 for(int marker=0;marker<2;marker++){
  reset();for(unsigned part=64;part<=192;part+=64){memcpy(canvas,base,sizeof canvas);if(marker)peak_q8[0]=210*256+part;else level_q8[0]=200*256+part;meters(canvas);if(part>64)assert(__builtin_memcmp(canvas,main_frame,sizeof canvas));memcpy(main_frame,canvas,sizeof canvas);}
 } /* Quarter-bin changes must alter pixels; no coarse rounded-position steps. */
 reset();tick(1000);assert(idle);unsigned quiet_revision=generation;
 for(unsigned t=1016;t<2000;t+=16)tick(t);assert(generation==quiet_revision);
 feed(input,DISC_SCOPE_MAX_SAMPLES+1,48000,2000);assert(idle&&generation==quiet_revision);
 for(unsigned i=0;i<1024;i++)input[i][0]=input[i][1]=10000;
 feed(input,1024,48000,2016);assert(!idle);tick(2016);assert(generation>quiet_revision);measure();assert(n);
 for(unsigned t=2200;t<9000;t+=16)tick(t);assert(idle&&!level_q8[0]&&!peak_q8[0]);quiet_revision=generation;
 for(unsigned t=9016;t<10000;t+=16)tick(t);assert(generation==quiet_revision);
 reset();assert(!idle);feed(input,1024,48000,11000);tick(11000);measure();assert(n);
 puts("PASS actual stereo renderer: mono/anti-phase, both polarities of L/R, stereo spread, common circular fit, correlation, silence decay, malformed feed and transition composition");fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
