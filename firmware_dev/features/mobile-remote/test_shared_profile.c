#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../audio-dsp/shared_profile.h"
int main(void){DiscChain c={0},out;unsigned enabled;char text[8192];c.count=2;for(unsigned i=0;i<2;i++){c.nodes[i].id=i+1;c.nodes[i].type=12;c.nodes[i].enabled=i;memcpy(c.nodes[i].params,disc_chain_eq_default,sizeof(c.nodes[i].params));}c.nodes[1].params[1]=-3;
 for(unsigned master=0;master<2;master++){int n=disc_shared_format(&c,master,text,sizeof(text));assert(n>0);assert(disc_shared_parse(text,&out,&enabled));assert(enabled==master&&out.count==2&&out.nodes[0].id==1&&out.nodes[1].params[1]==-3);assert(!disc_shared_format(&c,master,text,8));}
 const char *bad[]={"","DSP-SHARED-1 2\n1 0\n","DSP-SHARED-1 0 extra\n1 0","DSP-SHARED-1 1\n1 0 extra","DSP-SHARED-1 1\n1 9\n"};for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(!disc_shared_parse(bad[i],&out,&enabled));
 c.nodes[1].id=1;assert(!disc_shared_format(&c,1,text,sizeof(text)));c.nodes[1].id=2;c.nodes[1].params[0]=NAN;assert(!disc_shared_format(&c,1,text,sizeof(text)));puts("PASS shared profile: atomic master/nodes format, strict malformed rejection, finite values, bounded serialization");}
