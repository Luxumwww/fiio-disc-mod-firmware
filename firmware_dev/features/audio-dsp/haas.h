#ifndef DISC_HAAS_H
#define DISC_HAAS_H
/* Centered differential ambience (mode2) preserves the common/mid signal and
 * adds a delayed high-band side signal antisymmetrically. Mono input remains
 * identical in L/R. Legacy single-channel Haas modes0/1 remain explicit choices.
 * Allocation and coefficient math stay outside callbacks. */
#include "haas_params.h"
typedef struct {float *buffer,pole,low,wet,output,fraction;unsigned length,pos,delay,channel,mode;int bass_on;} DiscHaas;
static DiscHaas *disc_haas_create(unsigned rate,const float *p){
 if(rate<8000||rate>384000||!disc_haas_params_valid(p))return 0;
 DiscHaas *h=calloc(1,sizeof(*h));if(!h)return 0;
 float frames=rate*p[0]*.001f;h->delay=(unsigned)frames;h->fraction=frames-h->delay;h->length=h->delay+2;
 h->buffer=calloc(h->length,sizeof(float));if(!h->buffer){free(h);return 0;}
 h->wet=p[1];h->channel=p[2]==0?1:0;h->mode=(unsigned)p[2];h->bass_on=p[3]>0;
 h->pole=expf(-6.28318530718f*p[3]/rate);h->output=powf(10.f,p[4]/20.f);return h;
}
static void disc_haas_reset(DiscHaas *h){memset(h->buffer,0,h->length*sizeof(float));h->pos=0;h->low=0;}
static void disc_haas_destroy(DiscHaas *h){if(h){free(h->buffer);free(h);}}
static void disc_haas_sample(DiscHaas *h,float *io){
 float x=h->mode==2?(io[0]-io[1])*.5f:io[h->channel],low=0;
 if(h->bass_on){h->low=h->pole*h->low+(1-h->pole)*x;low=h->low;}
 float high=x-low;h->buffer[h->pos]=high;
 unsigned at=h->pos+h->length-h->delay;if(at>=h->length)at-=h->length;
 unsigned before=at?at-1:h->length-1;
 float delayed=h->buffer[at]+(h->buffer[before]-h->buffer[at])*h->fraction;
 /* Both neutral endpoints preserve the dry float sample exactly. */
 if(h->wet>0&&(h->delay||h->fraction>0)){
  if(h->mode==2){float side=h->wet*delayed;io[0]+=side;io[1]-=side;}
  else io[h->channel]=x+(delayed-high)*h->wet;
 }
 io[0]*=h->output;io[1]*=h->output;if(++h->pos==h->length)h->pos=0;
}
#endif
