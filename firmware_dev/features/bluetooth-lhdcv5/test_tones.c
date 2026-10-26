#include "disc_lhdcv5.h"
#include "lhdcv5_enc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static uint64_t samples;static uint32_t peak;
static int pcm(void *u,const int32_t *p,size_t n){(void)u;samples+=n;for(size_t i=0;i<n*2;i++){uint32_t a=p[i]<0?-(int64_t)p[i]:p[i];if(a>peak)peak=a;}return 0;}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL tone line %u: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){for(unsigned bits=16;bits<=24;bits+=8){for(unsigned rate=48000;rate<=192000;rate*=2){
 samples=peak=0;uint8_t config[]={0x3a,5,0,0,0x35,0x4c,rate==48000?0x10:rate==96000?4:1,bits==16?0x84:0x82,0x11,0,0};
 disc_lhdcv5 *d=disc_lhdcv5_open(config,11);CHECK(d);lhdcv5_enc_t *e=lhdcv5_enc_new(1);CHECK(e);
 CHECK(lhdcv5_enc_init_encoder(e,rate,bits,LHDC_QUALITY_LOW,1023,20)==0);uint32_t block=0;CHECK(!lhdcv5_enc_get_block_size(e,&block)&&block<=960);
 uint8_t raw[960*6],packet[8192];uint16_t seq=0;
 for(unsigned f=0;f<160;f++){
  for(unsigned n=0;n<block;n++)for(unsigned ch=0;ch<2;ch++){
   int32_t v=(int32_t)(sin(2*3.141592653589793*(ch?1500:1000)*(f*block+n)/rate)*(bits==16?12000:3000000));
   for(unsigned b=0;b<bits/8;b++)raw[(n*2+ch)*(bits/8)+b]=(uint32_t)v>>(8*b);
  }
  uint32_t written=0,frames=0;CHECK(!lhdcv5_enc_encode(e,raw,block*2*(bits/8),packet+14,sizeof(packet)-14,&written,&frames));
  if(!written)continue;
  memset(packet,0,14);packet[0]=0x80;packet[1]=0x60;packet[2]=seq>>8;packet[3]=seq++;packet[11]=1;packet[12]=frames<<2;
  CHECK(disc_lhdcv5_packet(d,packet,written+14,pcm,NULL)>=0);
 }
 disc_lhdcv5_stats st;disc_lhdcv5_get_stats(d,&st);CHECK(samples>0&&peak>1000000&&st.decode_errors==0&&st.malformed==0&&st.pending_bytes==0);
 printf("PASS generated stereo tone %u Hz/%u bit: %llu samples peak=%u\n",rate,bits,(unsigned long long)samples,peak);
 lhdcv5_enc_free(e);disc_lhdcv5_close(d);
}}return 0;}
