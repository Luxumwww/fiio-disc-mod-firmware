#include <assert.h>
#include <stdio.h>
#include "usbdac_format.h"
int main(void){
 unsigned h[40]={0},rate,bits,ch;h[3]=96000;h[4]=2;h[5]=24;
 assert(disc_uac_format(h,4,1,&rate,&bits,&ch)&&rate==96000&&bits==24&&ch==2);
 assert(!disc_uac_format(h,2,1,&rate,&bits,&ch));assert(!disc_uac_format(h,4,2,&rate,&bits,&ch));
 h[6]=1;assert(!disc_uac_format(h,4,1,&rate,&bits,&ch));h[6]=0;
 h[11]=2;assert(!disc_uac_format(h,4,1,&rate,&bits,&ch));h[11]=0;
 h[38]=3;assert(disc_uac_format(h,4,1,&rate,&bits,&ch));h[38]=0;
 for(unsigned r=8000;r<=192000;r*=2)for(unsigned c=1;c<=2;c++)for(unsigned b=16;b<=32;b+=8){h[3]=r;h[4]=c;h[5]=b;assert(disc_uac_format(h,4,1,&rate,&bits,&ch));}
 h[3]=7999;assert(!disc_uac_format(h,4,1,&rate,&bits,&ch));h[3]=96000;h[4]=3;assert(!disc_uac_format(h,4,1,&rate,&bits,&ch));
 const unsigned char packed[]={0xff,0xff,0x7f,0,0,0x80,0,0,0,0,1,0,0,0xff,0xff};
 assert(disc_visual_sample(packed,24,0)==32767);assert(disc_visual_sample(packed,24,1)==-32768);assert(disc_visual_sample(packed,24,2)==0);assert(disc_visual_sample(packed,24,3)==1);assert(disc_visual_sample(packed,24,4)==-1);
 const int wide[]={0x7fffffff,(int)0x80000000,0x10000,-0x10000};assert(disc_visual_sample((const void *)wide,32,0)==32767);assert(disc_visual_sample((const void *)wide,32,1)==-32768);assert(disc_visual_sample((const void *)wide,32,2)==1);assert(disc_visual_sample((const void *)wide,32,3)==-1);
 const short narrow[]={32767,-32768,1,-1};for(unsigned i=0;i<4;i++)assert(disc_visual_sample((const void *)narrow,16,i)==narrow[i]);
 unsigned char dop[16*8]={0};for(unsigned i=0;i<16;i++)for(unsigned c=0;c<2;c++)dop[(i*2+c)*4+3]=i&1?250:5;
 assert(disc_uac_is_dop(dop,32,2,16));dop[15*8+3]=7;assert(!disc_uac_is_dop(dop,32,2,16));assert(!disc_uac_is_dop(dop,16,2,16));
 h[4]=2;for(unsigned r=384000;r<=768000;r*=2){h[3]=r;assert(disc_uac_format(h,4,1,&rate,&bits,&ch));}h[3]=768001;assert(!disc_uac_format(h,4,1,&rate,&bits,&ch));
 for(unsigned mode=0;mode<6;mode++){assert(disc_visual_mode(mode,0)==(mode==3));assert(disc_visual_mode(mode,1)==(mode==1||mode==3));}
 puts("PASS: scoped mode gate, PCM16/packed24/32 signed extrema, USB format rates/channels and DoP/DSD/idle guards");return 0;
}
