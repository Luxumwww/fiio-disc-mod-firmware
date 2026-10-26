#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shared_toggle.h"
#include "../bluetooth-meter/dsp_order.h"
int main(void){unsigned order[8],next[8],seen=0;dsp_order_default(order);assert(order[7]==13);
 const char *old[]={"7 1 3 0 6 2 5 4","7 3 0 6 2 5 4","0 9 2 1 4 10 7 3 8 6 5"};
 for(unsigned i=0;i<3;i++){assert(dsp_order_parse(old[i],order));assert(order[7]==13);}
 for(unsigned i=0;i<8;i++)for(int j=-1;j<=8;j++){memcpy(next,order,sizeof(next));unsigned id=order[i];dsp_order_move(next,id,j);seen=0;for(unsigned k=0;k<8;k++){assert(!(seen&(1u<<next[k])));seen|=1u<<next[k];}assert(seen==8445&&next[j<0?0:j>7?7:j]==id);char text[128];snprintf(text,sizeof(text),"%u %u %u %u %u %u %u %u",next[0],next[1],next[2],next[3],next[4],next[5],next[6],next[7]);assert(dsp_order_parse(text,next));}
 assert(!dsp_order_parse("0 2 3 4 5 6 7 12",order));assert(!dsp_order_parse("0 2 3 4 5 6 13 13",order));
 DiscChain c={0},out;unsigned master=0;assert(disc_shared_toggle_type(&c,&master,13));assert(c.count==1&&master&&c.nodes[0].enabled&&!memcmp(c.nodes[0].params,disc_haas_default,sizeof(c.nodes[0].params)));
 c.nodes[0].params[0]=7.3f;c.nodes[0].params[2]=1;assert(disc_shared_toggle_type(&c,&master,13));assert(!c.nodes[0].enabled&&c.nodes[0].params[0]==7.3f&&c.nodes[0].params[2]==1);
 assert(disc_shared_toggle_type(&c,&master,13));c.nodes[1]=c.nodes[0];c.nodes[1].id=2;c.nodes[1].params[1]=.2f;c.count=2;
 assert(disc_shared_toggle_type(&c,&master,13));assert(!c.nodes[0].enabled&&!c.nodes[1].enabled&&c.nodes[1].params[1]==.2f);
 master=0;assert(disc_shared_toggle_type(&c,&master,13));assert(master&&c.nodes[0].enabled&&c.nodes[1].enabled);
 char text[8192];assert(disc_shared_format(&c,master,text,sizeof(text)));assert(disc_shared_parse(text,&out,&master)&&out.nodes[1].params[1]==.2f);
 for(unsigned i=2;i<8;i++){c.nodes[i]=c.nodes[0];c.nodes[i].id=i+1;}c.count=8;out=c;assert(!disc_shared_toggle_type(&c,&master,5)&&!memcmp(&out,&c,sizeof(c)));
 assert(!disc_shared_toggle_type(&c,&master,12));puts("PASS native Haas: legacy order migration, row dragging, shared default/toggle, duplicate parameter preservation, bounds, offline shared serialization");return 0;}
