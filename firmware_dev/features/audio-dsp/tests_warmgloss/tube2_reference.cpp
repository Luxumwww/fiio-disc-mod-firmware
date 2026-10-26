#include "Tube2.h"
extern "C" {
void *tube2_reference_create(unsigned rate,double input_pad,double tube) {
    Tube2 *s=new Tube2;
    s->rate=rate;s->A=(float)input_pad;s->B=(float)tube;
    return s;
}
void tube2_reference_sample(void *p,double io[2]) {
    Tube2 *s=(Tube2*)p;
    double l=io[0],r=io[1],*in[2]={&l,&r},*out[2]={io,io+1};
    s->processDoubleReplacing(in,out,1);
}
void tube2_reference_destroy(void *p){delete (Tube2*)p;}
}
