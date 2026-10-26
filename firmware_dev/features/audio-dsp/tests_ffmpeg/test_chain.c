#define main old_haas_checks
#include "../test_haas_190.c"
#undef main
#include "../shared_toggle.h"
int main(void){unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1100,count=0;int x[1024],y[1024];
 for(unsigned r=0;r<6;r++)for(unsigned v=0;v<25;v++){
  DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=14;c.nodes[0].enabled=1;memcpy(c.nodes[0].params,disc_ffhaas_default,sizeof(c.nodes[0].params));if(v){unsigned j=(v-1)/2;c.nodes[0].params[j]=disc_ffhaas_range[j][(v-1)&1];}Dsp *d=disc_dsp_create_chain(rates[r],&c);assert(d&&d->nodes[0]->ffhaas);
  for(unsigned k=0;k<64;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=(int)rng;}process(d,x,512);assert(!d->guard.invalid&&!d->clipped&&isfinite(d->guard.gain));}
  set_mask(d,0);memset(x,0,sizeof(x));for(unsigned k=0;k<80;k++)process(d,x,512);for(unsigned i=0;i<1024;i++)x[i]=y[i]=(i&1)?INT32_MIN+(int)i:INT32_MAX-(int)i;process(d,x,512);assert(!memcmp(x,y,sizeof(x)));
  set_mask(d,4096);memset(x,0,sizeof(x));for(unsigned k=0;k<80;k++){process(d,x,512);for(unsigned i=0;i<1024;i++)assert(!x[i]);}destroy(d);count++;
 }
 DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=14;c.nodes[0].enabled=1;memcpy(c.nodes[0].params,disc_ffhaas_default,sizeof(c.nodes[0].params));Dsp *d=disc_dsp_create_chain(48000,&c);assert(d);double delta=0;
 for(unsigned k=0;k<64;k++){for(unsigned i=0;i<512;i++){rng=rng*1664525u+1013904223u;x[2*i]=x[2*i+1]=(int)rng/8;}process(d,x,512);if(k>20)for(unsigned i=0;i<512;i++){double a=(x[2*i]/2147483648.)-(x[2*i+1]/2147483648.);delta+=a*a;}}
 assert(delta>1);destroy(d);unsigned on=0;c.count=0;assert(disc_shared_toggle_type(&c,&on,14));assert(on&&c.nodes[0].type==14&&!memcmp(c.nodes[0].params,disc_ffhaas_default,sizeof(c.nodes[0].params)));char b[8192];DiscChain parsed;assert(disc_shared_format(&c,on,b,sizeof(b))&&disc_shared_parse(b,&parsed,&on)&&parsed.nodes[0].type==14);
 printf("PASS %u full-chain FFmpeg single-effect contexts: finite protected output, OFF exact, reset silence; mono produces actual stereo difference; native defaults/shared format\n",count);return 0;}
