#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mandala_api.h"
const DiscMandalaApi *disc_mandala_baseline(unsigned);
int main(void){
 const DiscMandalaApi *a=disc_mandala_baseline(1),*b=disc_mandala_get_api(1);
 short pcm[3840][2];unsigned noise=12345,t=1000,frames=0;
 static unsigned background[129600];
 for(unsigned run=0;run<3;run++){
  a->reset();b->reset();
  for(unsigned f=0;f<180;f++){
   t+=f%3==0?23:f%3==1?16:7;
   unsigned n=f%2?960:3840,rate=f%2?48000:192000;
   for(unsigned i=0;i<n;i++)for(unsigned c=0;c<2;c++){noise=noise*1664525u+1013904223u;pcm[i][c]=(short)(noise>>16);}
   if(f<120&&f%4){a->feed(pcm,n,rate,t);b->feed(pcm,n,rate,t);}
   a->tick(t);b->tick(t);
   const DiscVisualImage *x=a->compose(360,0),*y=b->compose(360,0);
   assert(x->size==y->size&&!memcmp(x->data,y->data,x->size));
   assert(a->revision()==b->revision());
   if(f%30==0){x=a->compose(f*2,background);y=b->compose(f*2,background);assert(!memcmp(x->data,y->data,x->size));}
   frames++;
  }
 }
 printf("PASS %u frames: bit-identical full canvas, revision, transition, stale decay and reset\n",frames);
}
