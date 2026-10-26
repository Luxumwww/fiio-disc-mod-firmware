#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define DISC_DSP_UNITY
#include "dsp.c"
// Old reverb adapter is not exercised by these single-new-effect host checks.
void *disc_dragonfly_create(unsigned r){(void)r;return 0;}
void *disc_dragonfly_create_configured(unsigned r,const float *p){(void)r;(void)p;return 0;}
void disc_dragonfly_destroy(void *p){(void)p;}
void disc_dragonfly_reset(void *p){(void)p;}
unsigned disc_dragonfly_memory(void *p){(void)p;return 0;}
void disc_dragonfly_sample(void *p,float *x){(void)p;(void)x;}
int main(void){unsigned rates[]={44100,48000,192000,384000};unsigned rng=1047,count=0;int data[1024],original[1024];
 for(unsigned r=0;r<4;r++)for(unsigned fx=8;fx<12;fx++)for(unsigned j=0;j<disc_param_count[fx];j++)for(unsigned end=0;end<2;end++){
  float p[12][12];memcpy(p,disc_param_default,sizeof(p));p[fx][j]=disc_param_range[fx][j][end];Dsp *d=create_configured(rates[r],1u<<fx,".",p);assert(d&&(d->available&(1u<<fx)));
  for(unsigned k=0;k<128;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525+1013904223;data[i]=(int)rng;}process(d,data,512);assert(!d->clipped&&!d->guard.invalid&&isfinite(d->guard.gain));}
  set_mask(d,0);memset(data,0,sizeof(data));for(unsigned k=0;k<80;k++)process(d,data,512);
  for(unsigned i=0;i<1024;i++)data[i]=original[i]=(i&1)?INT32_MIN+i:INT32_MAX-i;process(d,data,512);assert(!memcmp(data,original,sizeof(data)));
  set_mask(d,1u<<fx);memset(data,0,sizeof(data));for(unsigned k=0;k<32;k++){process(d,data,512);for(unsigned i=0;i<1024;i++)assert(!data[i]);}destroy(d);count++;
 }
 float invalid[12][12];memcpy(invalid,disc_param_default,sizeof(invalid));invalid[10][0]=NAN;assert(!create_configured(48000,1024,".",invalid));invalid[10][0]=18000;assert(!create_configured(48000,1024,".",invalid));printf("PASS %u new single-effect endpoint contexts: finite, protected, OFF bit-exact, reset silence, invalid rejected\n",count);return 0;
}
