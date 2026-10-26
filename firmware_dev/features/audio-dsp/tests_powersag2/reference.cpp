#include "PowerSag2.h"
extern "C" {
void *powersag2_reference_create(double range,double invwet){
    PowerSag2 *s=new PowerSag2;s->A=(float)range;s->B=(float)invwet;return s;
}
void powersag2_reference_sample(void *p,double io[2]){
    PowerSag2 *s=(PowerSag2*)p;
    double l=io[0],r=io[1],*in[2]={&l,&r},*out[2]={io,io+1};
    s->processDoubleReplacing(in,out,1);
}
void powersag2_reference_destroy(void *p){delete (PowerSag2*)p;}
double powersag2_reference_clamp(void *p,double detector){
    PowerSag2 *s=(PowerSag2*)p;
    int offset=(int)(pow(s->A,4)*16383)+1;
    double burst=detector*(s->controlL/sqrt(offset));
    double clamp=detector/((burst==0.0)?1.0:burst);
    if(clamp>1.0)clamp=1.0;
    if(clamp<0.0)clamp=0.0;
    return clamp;
}
}
