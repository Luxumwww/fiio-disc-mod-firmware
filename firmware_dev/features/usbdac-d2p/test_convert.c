#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "convert.h"
#include "../bluetooth-meter/usbdac_volume.h"
static unsigned char *tone(unsigned carrier,unsigned frames){
 unsigned char *p=malloc(frames*8);assert(p);double error[2]={0};unsigned long bit=0;
 for(unsigned f=0;f<frames;f++){
  p[f*8]=p[f*8+4]=0;p[f*8+3]=p[f*8+7]=(f&1)?250:5;
  for(unsigned part=0;part<2;part++){
   unsigned char b[2]={0};for(unsigned j=0;j<8;j++,bit++){
    double x=0.3*sin(2*3.141592653589793*1000*bit/(carrier*16.0));
    for(unsigned c=0;c<2;c++){error[c]+=c?-x:x;int positive=error[c]>=0;error[c]-=positive?1:-1;b[c]=(unsigned char)((b[c]<<1)|positive);}
   }
   p[f*8+(part?1:2)]=b[0];p[f*8+4+(part?1:2)]=b[1];
  }
 }return p;
}
int main(void){
 for(unsigned carrier=176400;carrier<=352800;carrier*=2){
  unsigned frames=carrier/5;unsigned char *in=tone(carrier,frames);int *a=calloc(frames*2,sizeof(int)),*b=calloc(frames*2,sizeof(int));assert(a&&b);
  DiscD2P sa,sb;d2p_reset(&sa,carrier);d2p_reset(&sb,carrier);
  int n=d2p_convert(&sa,in,frames*8,a,frames);assert(n==17640);
  unsigned cursor=0,out=0;
  while(cursor<frames){unsigned count=1+cursor%191;if(count>frames-cursor)count=frames-cursor;int v=d2p_convert(&sb,in+cursor*8,count*8,b+out*2,frames-out);assert(v>=0);out+=(unsigned)v;cursor+=count;}
  assert(out==(unsigned)n&&!memcmp(a,b,out*8));
  double energy=0,cross=0;for(int i=2000;i<n;i++){double l=a[2*i]/2147483648.0,r=a[2*i+1]/2147483648.0;energy+=l*l;cross+=l*r;assert(fabs(l+r)<0.0001);}
  double rms=sqrt(energy/(n-2000));assert(rms>0.205&&rms<0.220&&cross<0);
  unsigned char malformed[8]={0,1,2,5,0,3,4,250};DiscD2P saved=sa;assert(d2p_convert(&sa,malformed,8,b,frames)==-1&&!memcmp(&sa,&saved,sizeof(sa)));
  d2p_reset(&sa,carrier);assert(d2p_convert(&sa,in,frames*8,a,1)==-2&&sa.pos==0&&sa.phase==0);
  printf("PASS DSD%u: 1000Hz RMS %.6f, exact duration, channel polarity, arbitrary chunk continuity, malformed packet rejected\n",carrier==176400?64:128,rms);
  free(a);free(b);free(in);
 }
 assert(disc_uac_volume(21,77,0)==21&&disc_uac_volume(21,77,1)==77);
 puts("PASS native UI selected volume instead of stale player cache");return 0;
}
