/* Production tag/core/editor headers: malformed inputs, exact PCM and edits. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
extern int open(const char *,int,...),close(int);
#include "replaygain_core.h"
#include "shared_edit.h"
static void near(float a,float b){assert(fabsf(a-b)<0.0001f);}
static void test_tags(const char *root){
 const char *names[]={"flac","id322","id323","id324","id3utf16","ape","mp4","vorbis","opus","bigcover","flac-id3"};
 for(unsigned i=0;i<sizeof names/sizeof names[0];i++){
  char path[512];snprintf(path,sizeof path,"%s/%s.bin",root,names[i]);DiscRgTags t=disc_rg_tags(path);
  if(t.valid!=15)fprintf(stderr,"tags: %s valid=%u\n",names[i],t.valid);
  assert(t.valid==15);near(t.track,-6);near(t.album,-3);near(t.track_peak,.8f);near(t.album_peak,.9f);
 }
 const char *bad[]={"empty","truncated","overflow","nonfinite","duplicate","budget","badogg"};
 for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++){char path[512];snprintf(path,sizeof path,"%s/%s.bin",root,bad[i]);DiscRgTags t=disc_rg_tags(path);if(t.valid)fprintf(stderr,"bad tags %s %u\n",bad[i],t.valid);assert(!t.valid);}
 /* Bounded random-file corpus checks parser arithmetic and buffer bounds. */
 for(unsigned i=0;i<256;i++){char p[512];snprintf(p,sizeof p,"%s/fuzz-%u.bin",root,i);disc_rg_tags(p);}
}
static void test_gain(void){
 DiscRgConfig c;assert(disc_rg_config_parse("1 2 -1.5 1\n",&c));assert(c.mode==2&&c.protect==1&&c.preamp==-1.5f);
 const char *bad[]={"","1 3 0 1","1 1 nan 1","1 1 13 1","1 1 0 2","1 1 0 1 extra"};for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++)assert(!disc_rg_config_parse(bad[i],&c));
 DiscRgTags t={-6,-3,.8f,.9f,15};c=(DiscRgConfig){1,1,0};DiscRgGain g=disc_rg_gain(&t,&c);near(g.gain,powf(10,-.3f));assert(g.source==1);
 c.mode=2;g=disc_rg_gain(&t,&c);near(g.db,-3);assert(g.source==2);t.valid&=~RG_ALBUM;g=disc_rg_gain(&t,&c);assert(g.source==3);near(g.db,-6);
 t.valid=0;c.preamp=12;g=disc_rg_gain(&t,&c);near(g.gain,1);assert(!g.source);
 t=(DiscRgTags){6,0,2,0,RG_TRACK|RG_TRACK_PEAK};c=(DiscRgConfig){1,1,0};g=disc_rg_gain(&t,&c);near(g.gain,.5f);assert(g.protected_peak);
 t.valid=RG_TRACK;g=disc_rg_gain(&t,&c);near(g.gain,1);assert(g.missing_peak&&g.protected_peak);c.protect=0;g=disc_rg_gain(&t,&c);assert(g.gain>1);
}
static void put(unsigned char *p,int x,unsigned bits){unsigned v=(unsigned)x;for(unsigned j=0;j<bits/8;j++)p[j]=(unsigned char)(v>>(8*j));}
static int get(const unsigned char *p,unsigned bits){unsigned v=0;for(unsigned j=0;j<bits/8;j++)v|=(unsigned)p[j]<<(8*j);return bits==16?(int)(short)v:bits==24?(int)(v<<8)>>8:(int)v;}
static void test_pcm(void){
 for(unsigned bits=16;bits<=32;bits+=8){
  unsigned width=bits/8;int high=bits==16?32767:bits==24?8388607:2147483647,low=-high-1;
  unsigned char pcm[64],original[64];int values[]={low,high,1234,-1234,0,0};for(unsigned i=0;i<6;i++)put(pcm+i*width,values[i],bits);memcpy(original,pcm,6*width);
  DiscRgRamp r={0};disc_rg_pcm_bytes(&r,pcm,6*width,192000,bits,2,1);assert(!memcmp(pcm,original,6*width));
  r=(DiscRgRamp){0};disc_rg_pcm_bytes(&r,pcm,6*width,192000,bits,2,.5f);for(unsigned i=0;i<6;i++)assert(get(pcm+i*width,bits)==(int)(values[i]*.5));
  memcpy(pcm,original,6*width);r=(DiscRgRamp){0};disc_rg_pcm_bytes(&r,pcm,6*width,192000,bits,2,4);assert(get(pcm,bits)==low&&get(pcm+width,bits)==high);
  unsigned char block[192*8];r=(DiscRgRamp){.current=1,.started=1,.target=1};
  for(unsigned k=0;k<20;k++){for(unsigned i=0;i<192*2;i++)put(block+i*width,1000,bits);disc_rg_pcm_bytes(&r,block,192*2*width,192000,bits,2,.5f);for(unsigned i=0;i<192;i++)assert(get(block+2*i*width,bits)==get(block+(2*i+1)*width,bits));if(k<19)assert(r.remaining);}
  near(r.current,.5f);assert(!r.remaining); /* 20 ms across twenty 1 ms buffers */
 }
}
static void test_edit(void){
 for(unsigned type=0;type<17;type++)if(type!=1){DiscChain c={0};unsigned id=0;const DiscNativeEffect *e=&disc_native_effects[type];assert(e->count);
  assert(disc_shared_edit(&c,&id,type,0,e->params[0].initial,e->params[0].initial)==1&&id==1&&c.count==1&&!c.nodes[0].enabled&&disc_chain_valid(&c));
  for(unsigned p=0;p<e->count;p++){float old=c.nodes[0].params[p];assert(disc_shared_edit(&c,&id,type,p,old,e->params[p].low)==1);assert(disc_shared_edit(&c,&id,type,p,e->params[p].low,e->params[p].high)==1);}
 }
 DiscChain c={0};unsigned first=0,second=0;assert(disc_shared_edit(&c,&first,16,0,0,.3f)==1);assert(disc_shared_edit(&c,&second,16,0,0,.4f)==1);c.nodes[0].enabled=1;
 DiscChainNode duplicate=c.nodes[1];assert(disc_shared_edit(&c,&first,16,1,c.nodes[0].params[1],.2f)==1);assert(!memcmp(&duplicate,&c.nodes[1],sizeof duplicate)&&c.nodes[0].enabled);
 assert(disc_shared_edit(&c,&first,16,1,0,.3f)==-1);c.nodes[0].id=7;assert(disc_shared_edit(&c,&first,16,1,.2f,.3f)==-1);
 while(c.count<16){unsigned id=0;assert(disc_shared_edit(&c,&id,16,0,0,.3f)==1);}unsigned id=0;assert(disc_shared_edit(&c,&id,16,0,0,.3f)==-3);
 char text[8192];int n=disc_shared_format(&c,0,text,sizeof text);DiscChain restored;unsigned master;assert(n&&disc_shared_parse(text,&restored,&master)&&!master&&restored.count==16);
}
static void test_reset(void){
 for(unsigned type=0;type<17;type++)if(type!=1){
  DiscChain c={0};unsigned id=0;const DiscNativeEffect *e=&disc_native_effects[type];assert(disc_shared_edit(&c,&id,type,0,0,e->params[0].low)==1);c.nodes[0].enabled=1;
  DiscChainNode expected=c.nodes[0];int result=disc_shared_reset(&c,id,type,expected.params);assert(result>=0&&disc_chain_valid(&c)&&c.count==1&&c.nodes[0].enabled);
  for(unsigned j=0;j<12;j++)assert(c.nodes[0].params[j]==(j<e->count?e->params[j].initial:0));assert(!disc_shared_reset(&c,id,type,c.nodes[0].params));
 }
 DiscChain c={0};unsigned a=0,b=0;assert(disc_shared_edit(&c,&a,16,0,0,.3f)==1&&disc_shared_edit(&c,&b,16,0,0,.4f)==1);
 DiscChainNode original=c.nodes[0];c.nodes[0].enabled=1;DiscChainNode other=c.nodes[1];assert(disc_shared_reset(&c,a,16,original.params)==1&&c.nodes[0].enabled&&!memcmp(&c.nodes[1],&other,sizeof other));
 DiscChain before=c;assert(disc_shared_reset(&c,a,16,original.params)==-1&&!memcmp(&c,&before,sizeof c));assert(disc_shared_reset(&c,999,16,original.params)==-1);assert(disc_shared_reset(&c,a,4,c.nodes[0].params)==-1);
 assert(!disc_shared_reset(&c,0,16,original.params)&&!memcmp(&c,&before,sizeof c));assert(disc_shared_reset(&c,a,1,original.params)==-2);
 puts("PASS reset: all16 schema defaults, switch/order/other-instance retained, stale/removed/wrong-type rejection, absent no-create, repeated default no-op");
}
int main(int argc,char **argv){assert(argc==2);test_tags(argv[1]);test_gain();test_pcm();test_edit();test_reset();puts("PASS tags/formats/malformed+256-fuzz gain/peak PCM16/24/32 ramp/chunks/stereo shared-edit/duplicates/stale/capacity");return 0;}
