/* Streaming DoP -> DSD MSB -> PCM. No allocation, locks or I/O here.
 * DSD byte-filter algorithm adapted from FFmpeg n4.2 libavcodec/dsd.c,
 * Copyright 2009,2011 Sebastian Gesemann; 2014 Peter Ross, LGPL2.1+.
 * Complete upstream source/license retained in vendor/. */
#ifndef DISC_D2P_CONVERT_H
#define DISC_D2P_CONVERT_H
#include "tables.h"
typedef struct {
 unsigned char fifo[2][16];unsigned pos;
 float pcm[2][256];unsigned cursor,phase,carrier,fade;
 unsigned char marker;unsigned have_marker;
} DiscD2P;
static unsigned char d2p_reverse(unsigned char x){x=(unsigned char)((x>>4)|(x<<4));x=(unsigned char)(((x&0xcc)>>2)|((x&0x33)<<2));return (unsigned char)(((x&0xaa)>>1)|((x&0x55)<<1));}
static void d2p_reset(DiscD2P *s,unsigned carrier){
 for(unsigned c=0;c<2;c++){for(unsigned i=0;i<16;i++)s->fifo[c][i]=0x69;for(unsigned i=0;i<256;i++)s->pcm[c][i]=0;}
 s->pos=s->cursor=s->phase=s->fade=s->have_marker=0;s->marker=0;s->carrier=carrier;
}
static float d2p_byte(DiscD2P *s,unsigned ch,unsigned char value){
 unsigned pos=s->pos;unsigned char *fifo=s->fifo[ch];fifo[pos]=value;
 unsigned j=(pos-6)&15;fifo[j]=d2p_reverse(fifo[j]);
 float sum=0;for(unsigned i=0;i<6;i++)sum+=d2p_dsd_lut[i][fifo[(pos-i)&15]]+d2p_dsd_lut[i][fifo[(pos-11+i)&15]];
 return sum;
}
/* Validate the whole packet before touching filter state. Frame sequence
 * continues across arbitrary chunks; each stereo frame shares its marker. */
static int d2p_packet(const DiscD2P *s,const unsigned char *in,unsigned bytes){
 if(bytes<8||(bytes&7)||(s->carrier!=176400&&s->carrier!=352800))return 0;
 unsigned expected=s->have_marker?(s->marker^255):in[3];if(expected!=5&&expected!=250)return 0;
 for(unsigned i=0;i<bytes;i+=8){if(in[i]||in[i+4]||in[i+3]!=expected||in[i+7]!=expected)return 0;expected^=255;}
 return 1;
}
static int d2p_convert(DiscD2P *s,const unsigned char *in,unsigned bytes,int *out,unsigned frames_capacity){
 if(!d2p_packet(s,in,bytes))return -1;
 unsigned ratio=s->carrier==176400?4:8;
 unsigned count=(s->phase+bytes/4)/ratio;if(count>frames_capacity)return -2;
 const float *coeff=s->carrier==176400?d2p_fir_176400:d2p_fir_352800;
 unsigned taps=s->carrier==176400?127:255,half=taps/2,frames=0;
 for(unsigned i=0;i<bytes;i+=8){
  /* In left-justified DoP, the first DSD byte is byte2 and the
   * following byte is byte1. Never include marker or zero padding. */
  for(unsigned part=0;part<2;part++){
   unsigned off=part?1:2;
   for(unsigned c=0;c<2;c++)s->pcm[c][s->cursor]=d2p_byte(s,c,in[i+4*c+off]);
   s->pos=(s->pos+1)&15;
   if(++s->phase==ratio){
    s->phase=0;float fade=s->fade<882?(float)++s->fade/882.0f:1.0f;
    for(unsigned c=0;c<2;c++){
     float v=0;
     for(unsigned j=0;j<half;j++)v+=coeff[j]*(s->pcm[c][(s->cursor-j)&255]+s->pcm[c][(s->cursor-(taps-1)+j)&255]);
     v+=coeff[half]*s->pcm[c][(s->cursor-half)&255];v*=fade;
     if(v>0.999999f)v=0.999999f;if(v< -0.999999f)v=-0.999999f;
     out[frames*2+c]=(int)(v*2147483647.0f);
    }frames++;
   }
   s->cursor=(s->cursor+1)&255;
  }
 }
 s->marker=in[bytes-5];s->have_marker=1;return (int)frames;
}
#endif
