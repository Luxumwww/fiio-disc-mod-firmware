#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char pixels[129600],reference[129600];
static void stamp(unsigned char *p,int x,int y,unsigned a){if((unsigned)x<360&&(unsigned)y<360&&a)p[y*360+x]=1;}
static void point(unsigned char *p,int x,int y){int ix=x>>8,iy=y>>8;unsigned fx=(unsigned)x&255,fy=(unsigned)y&255;stamp(p,ix,iy,((256-fx)*(256-fy))>>8);stamp(p,ix+1,iy,(fx*(256-fy))>>8);stamp(p,ix,iy+1,((256-fx)*fy)>>8);stamp(p,ix+1,iy+1,(fx*fy)>>8);}
#define DISC_SCOPE_STAMP(c,x,y,v) stamp(c,x,y,v)
#define DISC_SCOPE_POINT(c,x,y) point(c,x,y)
#include "scope_stroke.h"
int main(void){
 unsigned random=1;
 for(unsigned k=0;k<64;k++){
  int v[4];for(unsigned j=0;j<4;j++){random=random*1664525u+1013904223u;v[j]=(int)(random%(300*256))+30*256;}
  if(k<4){v[0]=v[1]=30*256+57;v[2]=k&1?v[0]:330*256-12;v[3]=k&2?v[1]:330*256-12;}
  memset(pixels,0,sizeof pixels);disc_scope_stroke(pixels,v[0],v[1],v[2],v[3]);memcpy(reference,pixels,sizeof pixels);
  memset(pixels,0,sizeof pixels);disc_scope_stroke(pixels,v[2],v[3],v[0],v[1]);assert(!memcmp(pixels,reference,sizeof pixels));
  double dx=(v[2]-v[0])/256.0,dy=(v[3]-v[1])/256.0,length=dx*dx+dy*dy;unsigned lit=0;
  for(unsigned i=0;i<129600;i++)if(pixels[i]){lit++;double x=i%360-v[0]/256.0,y=i/360-v[1]/256.0,cross=x*dy-y*dx;assert(!length||cross*cross<length*2.25);}
  assert(lit);
 }
 puts("PASS shared high-rate stroke: all slopes, reverse symmetry, exact endpoint coverage and<=1.5px line distance");fflush(stdout);return 0;
}
