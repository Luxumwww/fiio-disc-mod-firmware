#include "stream_core.h"
#include "spectrum_tables.h"
static DiscStream stream;
static DiscPcmPacket packet;
#ifdef DISC_HOST_TEST
#include <stdio.h>
static void report(const char *s){fputs(s,stdout);}
#else
void *memset(void *destination,int value,unsigned long count){volatile unsigned char *p=destination;for(unsigned long i=0;i<count;i++)p[i]=(unsigned char)value;return destination;}
static void report(const char *s){unsigned n=0;while(s[n])n++;register long v0 __asm__("$2")=4004,a0 __asm__("$4")=1,a1 __asm__("$5")=(long)s,a2 __asm__("$6")=n;__asm__ volatile("syscall":"+r"(v0):"r"(a0),"r"(a1),"r"(a2):"$7","memory");}
#endif
static int wave(unsigned phase){phase%=1024;return phase<512?disc_cos[phase]/2:-disc_cos[phase-512]/2;}
static void make(unsigned rate,unsigned position,unsigned step,unsigned divide,unsigned right_only){
 packet=(DiscPcmPacket){DISC_PCM_MAGIC,1,2,rate,2,256,position,7,{0}};
 for(unsigned i=0;i<256;i++){
  int sample=wave((position+i)*step/divide),left=right_only?0:sample,right=-sample;
  packet.pcm[i*4]=(unsigned char)left;packet.pcm[i*4+1]=(unsigned char)(left>>8);
  packet.pcm[i*4+2]=(unsigned char)right;packet.pcm[i*4+3]=(unsigned char)(right>>8);
 }
}
static unsigned peak(unsigned channel){unsigned best=1;for(unsigned k=2;k<512;k++)if(stream.spectrum[channel].magnitude[k]>stream.spectrum[channel].magnitude[best])best=k;return best;}
int disc_native_selftest(void){
 const unsigned decimations[]={1,2,4,8,16};
 for(unsigned j=0;j<5;j++){
  unsigned d=decimations[j];disc_stream_reset(&stream);
  for(unsigned pos=0;pos<2048*d;pos+=256){make(48000*d,pos,16,d,0);if(!disc_stream_packet(&stream,&packet,1056))return 1;}
  if(!disc_stream_analyze(&stream)||peak(0)!=16||peak(1)!=16||stream.spectrum[0].magnitude[16]<2500)return 2;
  unsigned level=0;for(unsigned b=0;b<72;b++)level+=stream.levels[b];if(!level||stream.db[0]>-5||stream.db[0]<-12)return 3;
 }
 disc_stream_reset(&stream);
 for(unsigned pos=0;pos<2048;pos+=256){make(48000,pos,16,1,1);disc_stream_packet(&stream,&packet,1056);}
 if(!disc_stream_analyze(&stream)||peak(1)!=16||stream.db[0]!=-96)return 4;
 unsigned total=0;for(unsigned b=0;b<72;b++)total+=stream.levels[b];if(!total)return 5;
 /* Out-of-band 56.25kHz at 192kHz would alias to 8.25kHz without the FIR. */
 disc_stream_reset(&stream);
 for(unsigned pos=0;pos<8192;pos+=256){make(192000,pos,300,1,0);disc_stream_packet(&stream,&packet,1056);}
 if(!disc_stream_analyze(&stream))return 6;
 if(stream.spectrum[0].magnitude[peak(0)]>50)return 7;
 make(192000,8193,16,4,0);disc_stream_packet(&stream,&packet,1056);if(disc_stream_analyze(&stream)||stream.fill!=64)return 8;
 unsigned before=stream.accepted;packet.magic=0;if(disc_stream_packet(&stream,&packet,1056)||stream.accepted!=before)return 9;
 packet=(DiscPcmPacket){DISC_PCM_MAGIC,1,32,176400,2,256,0,8,{0}};
 for(unsigned i=0;i<256;i++)for(unsigned c=0;c<2;c++){
  unsigned k=(i*2+c)*3;packet.pcm[k]=0xaa;packet.pcm[k+1]=0xbb;packet.pcm[k+2]=i&1?250:5;
 }
 if(!disc_stream_packet(&stream,&packet,1568)||!stream.dop||disc_stream_analyze(&stream))return 10;
 for(unsigned pos=256;pos<256+8192;pos+=256){
  packet.position=pos;for(unsigned i=0;i<1536;i++)packet.pcm[i]=0;disc_stream_packet(&stream,&packet,1568);
 }
 if(stream.dop||!disc_stream_analyze(&stream)||stream.db[0]!=-96)return 11;
 for(unsigned b=0;b<72;b++)if(stream.levels[b])return 12;
 report("DISC_STREAM_OK rates=48/96/192/384/768k anti_phase=pass right_only=pass alias_rejection=pass gap_reset=pass DoP=filtered recovery=pass\n");return 0;
}
#ifdef DISC_HOST_TEST
int main(void){return disc_native_selftest();}
#endif
