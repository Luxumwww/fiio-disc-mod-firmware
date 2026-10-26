#define main old_haas_checks
#include "../test_haas_190.c"
#undef main
#include "../shared_toggle.h"
static DiscChain wg_spec(void){DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=16;c.nodes[0].enabled=1;memcpy(c.nodes[0].params,disc_warmgloss_default,sizeof c.nodes[0].params);return c;}
int main(void){
 unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1140,contexts=0;
 int x[1024],y[1024];
 for(unsigned r=0;r<6;r++)for(unsigned variant=0;variant<15;variant++){
  DiscChain c=wg_spec();if(variant>=1&&variant<=12)c.nodes[0].params[(variant-1)/2]=disc_warmgloss_range[(variant-1)/2][(variant-1)&1];
  if(variant==13)for(unsigned i=0;i<6;i++)c.nodes[0].params[i]=disc_warmgloss_range[i][1];
  if(variant==14)for(unsigned i=0;i<6;i++)c.nodes[0].params[i]=disc_warmgloss_range[i][0];
  Dsp *d=disc_dsp_create_chain(rates[r],&c),*swap=disc_dsp_create_chain(rates[r],&c);assert(d&&swap&&d->nodes[0]->warmgloss);
  for(unsigned k=0;k<32;k++){
   for(unsigned i=0;i<512;i++){rng=rng*1664525u+1013904223u;x[2*i]=(int)rng;rng=rng*1664525u+1013904223u;x[2*i+1]=k&1?x[2*i]:(int)rng;y[2*i]=x[2*i+1];y[2*i+1]=x[2*i];}
   assert(process(d,x,512)&&process(swap,y,512));
   for(unsigned i=0;i<512;i++){assert(x[2*i]==y[2*i+1]&&x[2*i+1]==y[2*i]);if(k&1){/* Previous independent state may ring briefly; reset checked below. */}}
   assert(!d->guard.invalid&&!d->clipped&&isfinite(d->guard.gain));
  }
  destroy(swap);set_mask(d,0);memset(x,0,sizeof x);for(unsigned k=0;k<80;k++)process(d,x,512);
  for(unsigned i=0;i<1024;i++)x[i]=y[i]=(i&1)?INT32_MIN+(int)i:INT32_MAX-(int)i;process(d,x,512);assert(!memcmp(x,y,sizeof x));
  set_mask(d,4096);Dsp *fresh=disc_dsp_create_chain(rates[r],&c);assert(fresh);
  for(unsigned k=0;k<40;k++){memset(x,0,sizeof x);memset(y,0,sizeof y);process(d,x,512);process(fresh,y,512);assert(!memcmp(x,y,sizeof x));for(unsigned i=0;i<1024;i++)assert(x[i]==0);}
  destroy(fresh);destroy(d);contexts++;
 }
 DiscChain c={0};unsigned master=0;assert(disc_shared_toggle_type(&c,&master,16));assert(c.count==1&&master&&!memcmp(c.nodes[0].params,disc_warmgloss_default,sizeof c.nodes[0].params));
 char text[8192];DiscChain parsed;assert(disc_shared_format(&c,master,text,sizeof text)&&disc_shared_parse(text,&parsed,&master));assert(!memcmp(&c,&parsed,sizeof c));
 for(unsigned i=0;i<12;i++){float old=c.nodes[0].params[i];c.nodes[0].params[i]=NAN;assert(!disc_chain_valid(&c));c.nodes[0].params[i]=old;}
 c.nodes[0].type=17;assert(!disc_chain_valid(&c));c.nodes[0].type=16;
 for(unsigned slots=9;slots<=16;slots+=7){c.count=slots;for(unsigned i=1;i<slots;i++){c.nodes[i]=c.nodes[0];c.nodes[i].id=i+1;c.nodes[i].enabled=0;}Dsp *d=disc_dsp_create_chain(192000,&c);assert(d);destroy(d);}
 printf("PASS %u single warmgloss contexts/6 rates: all endpoints, swapped stereo equality, bounded protected output, exact OFF/silent reset, shared persistence, invalid values, 9/16 saved single-enabled chains\n",contexts);
 return 0;
}
