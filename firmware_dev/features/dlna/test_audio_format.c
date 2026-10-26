#include <assert.h>
#include <stdio.h>
#include "audio_format.h"
#include "../audio-dsp/mode_preferences.h"
int main(void){
 unsigned h[40]={0},rate,bits,channels,masks[3];h[3]=192000;h[4]=2;h[5]=32;
 assert(disc_dlna_format(h,5,&rate,&bits,&channels)&&rate==192000&&bits==32&&channels==2);
 for(unsigned mode=0;mode<17;mode++)if(mode!=5)assert(!disc_dlna_format(h,mode,&rate,&bits,&channels));
 const unsigned rates[]={8000,44100,48000,88200,96000,176400,192000,384000,768000};
 for(unsigned i=0;i<sizeof(rates)/sizeof(rates[0]);i++)for(unsigned b=16;b<=32;b+=8)for(unsigned c=1;c<=2;c++){
  h[3]=rates[i];h[4]=c;h[5]=b;assert(disc_dlna_format(h,5,&rate,&bits,&channels)&&rate==h[3]&&bits==b&&channels==c);
 }
 h[11]=2;assert(!disc_dlna_format(h,5,&rate,&bits,&channels));h[11]=0;h[6]=1;assert(!disc_dlna_format(h,5,&rate,&bits,&channels));h[6]=0;
 h[3]=7999;assert(!disc_dlna_format(h,5,&rate,&bits,&channels));h[3]=768001;assert(!disc_dlna_format(h,5,&rate,&bits,&channels));h[3]=48000;
 h[5]=8;assert(!disc_dlna_format(h,5,&rate,&bits,&channels));h[5]=32;h[4]=3;assert(!disc_dlna_format(h,5,&rate,&bits,&channels));
 disc_dsp_read_masks("8 16\n",masks);assert(masks[0]==8&&masks[1]==16&&!masks[2]);
 disc_dsp_read_masks("8 16 31\n",masks);assert(masks[0]==8&&masks[1]==16&&masks[2]==29);
 disc_dsp_read_masks("32 64 128\n",masks);assert(masks[0]==32&&masks[1]==64&&masks[2]==128);
 disc_dsp_read_masks("255 255 255\n",masks);assert(masks[0]==253&&masks[1]==253&&masks[2]==253);
 disc_dsp_read_masks("8 16 256\n",masks);assert(!masks[0]&&!masks[1]&&!masks[2]);
 disc_dsp_read_masks("8\n",masks);assert(!masks[0]&&!masks[1]&&!masks[2]);
 puts("PASS: native DLNA PCM formats, source-bit distinction, DSD/invalid rejection, legacy and independent three-bank DSP preferences");
}
