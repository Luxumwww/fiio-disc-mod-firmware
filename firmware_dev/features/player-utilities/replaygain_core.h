#ifndef DISC_REPLAYGAIN_CORE_H
#define DISC_REPLAYGAIN_CORE_H
#include "replaygain_tags.h"
extern float powf(float,float),log10f(float);
typedef struct {unsigned mode,protect;float preamp;} DiscRgConfig;
typedef struct {float gain,db;unsigned source,protected_peak,missing_peak;} DiscRgGain;
typedef struct {float current;unsigned started;float target,delta;unsigned remaining;} DiscRgRamp;
static int disc_rg_config_parse(const char *s,DiscRgConfig *out){
 DiscRgConfig c;unsigned version;int n=0;
 if(sscanf(s,"%u %u %f %u %n",&version,&c.mode,&c.preamp,&c.protect,&n)!=4||s[n]||version!=1||c.mode>2||c.protect>1||!isfinite(c.preamp)||c.preamp< -12||c.preamp>12)return 0;
 *out=c;return 1;
}
static DiscRgGain disc_rg_gain(const DiscRgTags *t,const DiscRgConfig *c){
 DiscRgGain g={1,0,0,0,0};if(!c->mode)return g;float peak=0;
 if(c->mode==2&&(t->valid&RG_ALBUM)){g.db=t->album;g.source=2;if(t->valid&RG_ALBUM_PEAK)peak=t->album_peak;}
 else if(t->valid&RG_TRACK){g.db=t->track;g.source=c->mode==2?3:1;if(t->valid&RG_TRACK_PEAK)peak=t->track_peak;}
 else return g;
 g.db+=c->preamp;g.gain=powf(10.0f,g.db*.05f);
 if(c->protect){float maximum=peak>0?1.0f/peak:1.0f;g.missing_peak=!(peak>0);if(g.gain>maximum){g.gain=maximum;g.db=20.0f*log10f(g.gain);g.protected_peak=1;}}
 return g;
}
/* Signed 16/24/32-bit interleaved PCM in the native decoder buffer. This
 * static gain preserves dynamics; the optional peak cap uses metadata only,
 * and cannot promise protection against subsequent EQ/DSP boosting. */
static void disc_rg_pcm_bytes(DiscRgRamp *r,void *data,unsigned bytes,unsigned rate,unsigned bits,unsigned channels,float target){
 if((bits!=16&&bits!=24&&bits!=32)||!channels||channels>8)return;
 unsigned sample_bytes=bits/8,frame_bytes=sample_bytes*channels;if(bytes%frame_bytes)return;unsigned frames=bytes/frame_bytes;unsigned char *pcm=data;
 if(!rate||!isfinite(target)||target<0||target>6400)return;
 if(!r->started){r->started=1;r->current=r->target=target;r->remaining=0;}
 if(r->target!=target){r->target=target;r->remaining=rate/50;if(!r->remaining)r->remaining=1;r->delta=(target-r->current)/(float)r->remaining;}
 if(r->current==1.0f&&target==1.0f)return;
 for(unsigned i=0;i<frames;i++){
  if(r->remaining){r->current+=r->delta;if(!--r->remaining)r->current=target;}float gain=r->current;
  for(unsigned c=0;c<channels;c++){
   unsigned char *p=pcm+(i*channels+c)*sample_bytes;unsigned raw=(unsigned)p[0]|(unsigned)p[1]<<8;if(bits>=24)raw|=(unsigned)p[2]<<16;if(bits==32)raw|=(unsigned)p[3]<<24;
   int value=bits==16?(int)(short)raw:bits==24?(int)(raw<<8)>>8:(int)raw;double x=(double)value*(double)gain;
   int high=bits==16?32767:bits==24?8388607:2147483647,low= -high-1;
   int out=x>=high?high:x<=low?low:(int)x;unsigned result=(unsigned)out;
   p[0]=(unsigned char)result;p[1]=(unsigned char)(result>>8);if(bits>=24)p[2]=(unsigned char)(result>>16);if(bits==32)p[3]=(unsigned char)(result>>24);
  }
 }
}
#endif
