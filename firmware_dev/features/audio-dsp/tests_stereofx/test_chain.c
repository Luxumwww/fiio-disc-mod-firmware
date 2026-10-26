#define main old_haas_checks
#include "../test_haas_190.c"
#undef main
#include "../shared_toggle.h"
#include "../../bluetooth-meter/dsp_order.h"
int main(void){unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1110,count=0;int x[1024],y[1024];
 for(unsigned r=0;r<6;r++)for(unsigned v=0;v<8;v++){
  DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=15;c.nodes[0].enabled=1;memcpy(c.nodes[0].params,disc_stereofx_default,sizeof(c.nodes[0].params));if(v>=1&&v<=6)c.nodes[0].params[(v-1)/2]=disc_stereofx_range[(v-1)/2][(v-1)&1];if(v==7)c.nodes[0].params[0]=c.nodes[0].params[2]=1,c.nodes[0].params[1]=.7f;
  Dsp *d=disc_dsp_create_chain(rates[r],&c);assert(d&&d->nodes[0]->stereofx);
  for(unsigned k=0;k<48;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=(int)rng;}process(d,x,512);assert(!d->guard.invalid&&!d->clipped&&isfinite(d->guard.gain));}
  set_mask(d,0);memset(x,0,sizeof(x));for(unsigned k=0;k<80;k++)process(d,x,512);for(unsigned i=0;i<1024;i++)x[i]=y[i]=(i&1)?INT32_MIN+(int)i:INT32_MAX-(int)i;process(d,x,512);assert(!memcmp(x,y,sizeof(x)));
  set_mask(d,4096);Dsp *fresh=disc_dsp_create_chain(rates[r],&c);for(unsigned k=0;k<32;k++){memset(x,0,sizeof(x));memset(y,0,sizeof(y));process(d,x,512);process(fresh,y,512);assert(!memcmp(x,y,sizeof(x)));}destroy(fresh);destroy(d);count++;
 }
 DiscChain c={0};unsigned on=0;assert(disc_shared_toggle_type(&c,&on,15));assert(on&&c.nodes[0].type==15&&!memcmp(c.nodes[0].params,disc_stereofx_default,sizeof(c.nodes[0].params)));char text[8192];DiscChain parsed;assert(disc_shared_format(&c,on,text,sizeof(text))&&disc_shared_parse(text,&parsed,&on));assert(disc_shared_toggle_type(&c,&on,15)&&!c.nodes[0].enabled);assert(disc_shared_toggle_type(&c,&on,14)&&c.count==2&&c.nodes[1].type==14);assert(c.nodes[0].type==15&&!c.nodes[0].enabled);
 unsigned order[DSP_ORDER_COUNT];assert(dsp_order_parse("4 6 0 5 3 2 7 13\n",order));assert(order[7]==13&&order[8]==14);for(unsigned i=0;i<10000;i++){rng=rng*1664525u+1013904223u;unsigned id=order[rng%DSP_ORDER_COUNT];dsp_order_move(order,id,(int)((rng>>8)%13)-2);unsigned mask=0;int n=0;for(unsigned j=0;j<DSP_ORDER_COUNT;j++){assert(!(mask&(1u<<order[j])));mask|=1u<<order[j];n+=snprintf(text+n,sizeof(text)-n,"%u ",order[j]);}assert(mask==24829&&dsp_order_parse(text,order));}assert(!dsp_order_parse("0 0 2 3 4 5 6 7 13",order));
 printf("PASS %u full-chain endpoint contexts/6 rates: protected finite output, exact OFF, deterministic reset; independent Haas/StereoFX toggles, shared persistence, old-order migration and10000 reorder checks\n",count);return 0;
}
