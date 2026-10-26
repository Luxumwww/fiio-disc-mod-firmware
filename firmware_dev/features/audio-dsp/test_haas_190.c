#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define DISC_DSP_UNITY
#include "dsp.c"
void *disc_dragonfly_create(unsigned r){(void)r;return 0;}
void *disc_dragonfly_create_configured(unsigned r,const float *p){(void)r;(void)p;return 0;}
void disc_dragonfly_destroy(void *p){(void)p;}
void disc_dragonfly_reset(void *p){(void)p;}
unsigned disc_dragonfly_memory(void *p){(void)p;return 0;}
void disc_dragonfly_sample(void *p,float *x){(void)p;(void)x;}
static DiscChain spec(void){DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=13;c.nodes[0].enabled=1;memcpy(c.nodes[0].params,disc_haas_default,sizeof(c.nodes[0].params));return c;}
static void impulse(unsigned rate,unsigned direction){
 float p[12];memcpy(p,disc_haas_default,sizeof(p));p[0]=3.1f;p[1]=1;p[2]=direction;p[3]=0;
 DiscHaas *h=disc_haas_create(rate,p);assert(h);unsigned ch=direction?0:1;
 double frames=(double)(rate*p[0]*.001f);unsigned k=(unsigned)frames;float fraction=(float)(frames-k);
 for(unsigned i=0;i<k+20;i++){float x[2]={i==0?.25f:0,i==0?.5f:0};float amplitude=ch?.5f:.25f;disc_haas_sample(h,x);
  float expected=i==k?amplitude*(1-fraction):i==k+1?amplitude*fraction:0;
  assert(fabsf(x[ch]-expected)<1e-6f);assert(x[1-ch]==(i==0?(ch?.25f:.5f):0));
 }disc_haas_destroy(h);
 for(unsigned neutral=0;neutral<2;neutral++){p[0]=neutral?20:0;p[1]=neutral?0:1;p[3]=150;h=disc_haas_create(rate,p);assert(h);
  for(unsigned i=0;i<10000;i++){float x[2]={sinf(i*.1f)*.9f,cosf(i*.13f)*.9f},dry[2];memcpy(dry,x,sizeof(x));disc_haas_sample(h,x);assert(!memcmp(x,dry,sizeof(x)));}disc_haas_destroy(h);
 }
 p[0]=20;p[1]=1;p[3]=150;h=disc_haas_create(rate,p);assert(h);
 for(unsigned i=0;i<rate;i++){float x[2]={.25f,.25f};disc_haas_sample(h,x);if(i>rate/2)assert(fabsf(x[ch]-.25f)<1e-5f);}disc_haas_destroy(h);
}
int main(void){unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=190,count=0;int x[1024],dry[1024];
 for(unsigned r=0;r<6;r++){
  for(unsigned direction=0;direction<2;direction++)impulse(rates[r],direction);
  for(unsigned param=0;param<5;param++)for(unsigned end=0;end<2;end++){
   DiscChain c=spec();c.nodes[0].params[param]=disc_haas_range[param][end];Dsp *d=disc_dsp_create_chain(rates[r],&c);assert(d&&d->nodes[0]->haas);
   for(unsigned k=0;k<48;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=(int)rng;}process(d,x,512);assert(!d->clipped&&!d->guard.invalid&&isfinite(d->guard.gain));}
   set_mask(d,0);memset(x,0,sizeof(x));for(unsigned k=0;k<80;k++)process(d,x,512);
   for(unsigned i=0;i<1024;i++)x[i]=dry[i]=(i&1)?INT32_MIN+(int)i:INT32_MAX-(int)i;process(d,x,512);assert(!memcmp(x,dry,sizeof(x)));
   set_mask(d,4096);memset(x,0,sizeof(x));for(unsigned k=0;k<80;k++){process(d,x,512);for(unsigned i=0;i<1024;i++)assert(x[i]==0);}destroy(d);count++;
  }
  DiscChain c=spec();c.nodes[0].enabled=0;Dsp *d=disc_dsp_create_chain(rates[r],&c);assert(d&&!d->nodes[0]);for(unsigned i=0;i<1024;i++)x[i]=dry[i]=(int)(i*571119u);process(d,x,512);assert(!memcmp(x,dry,sizeof(x)));destroy(d);
 }
 DiscChain c=spec();Dsp *a=disc_dsp_create_chain(192000,&c),*b=disc_dsp_create_chain(192000,&c);assert(a&&b);
 for(unsigned block=0;block<100;block++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=dry[i]=(int)rng/4;}process(a,x,512);for(unsigned i=0;i<512;i+=16)process(b,dry+2*i,16);assert(!memcmp(x,dry,sizeof(x)));}destroy(a);destroy(b);
 for(unsigned j=0;j<12;j++){c=spec();c.nodes[0].params[j]=NAN;assert(!disc_dsp_create_chain(48000,&c));c.nodes[0].params[j]=disc_haas_range[j][1]+1;assert(!disc_chain_valid(&c));}
 c=spec();c.nodes[0].params[2]=.5f;assert(!disc_chain_valid(&c));c=spec();assert(!disc_dsp_create_chain(768000,&c));
 assert(!disc_chain_parse("1 1\n1 13 1 3 .35 .5 150 0 0 0 0 0 0 0 0\n",&c));
 assert(disc_chain_parse("1 1\n1 13 0 3 .35 1 150 0 0 0 0 0 0 0 0\n",&c));
 printf("PASS Haas: %u single-effect endpoints, 6 rates, fractional impulses/direction, neutral identity, DC protection, OFF exact, reset silence, chunk invariance, validation\n",count);return 0;
}
