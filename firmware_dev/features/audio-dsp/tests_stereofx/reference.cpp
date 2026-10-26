#include "StereoFX.h"
#include <stdlib.h>
extern "C" {
void *stereofx_reference_create(unsigned rate,const float *p){StereoFX *s=(StereoFX*)calloc(1,sizeof(StereoFX));s->fpdL=s->fpdR=16386;s->rate=rate;s->A=p[0];s->B=p[1];s->C=p[2];return s;}
void stereofx_reference_sample(void *v,double *io){StereoFX *s=(StereoFX*)v;double l=io[0],r=io[1],*in[2]={&l,&r},*out[2]={io,io+1};s->processDoubleReplacing(in,out,1);}
void stereofx_reference_process(void *s,const double *in,double *out,unsigned n){for(unsigned i=0;i<n;i++){out[i*2]=in[i*2];out[i*2+1]=in[i*2+1];stereofx_reference_sample(s,out+i*2);}}
void stereofx_reference_destroy(void *v){free(v);}
}
