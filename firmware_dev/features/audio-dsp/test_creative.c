#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "creative_effects.h"
#include "profile_format.h"
static double rms(DiscDynamic *d,unsigned rate,double hz){double sum=0;for(unsigned i=0;i<rate;i++){float x=.2*sin(6.28318530718*hz*i/rate),p[2]={x,x};creative_dynamic(d,p);if(i>rate/2)sum+=p[0]*p[0];assert(isfinite(p[0])&&p[0]==p[1]);}return sqrt(sum/(rate/2-1));}
int main(void){
 unsigned rates[]={44100,48000,192000,384000};for(unsigned r=0;r<4;r++){
  unsigned rate=rates[r];DiscStereo s={0};creative_stereo_init(&s,rate,disc_param_default[8]);for(unsigned i=0;i<10000;i++){float x=.3*sin(i*.1),p[2]={x,x};creative_stereo(&s,p);assert(fabs(p[0]-x)<1e-6&&p[0]==p[1]);}
  DiscDynamic high={0},low={0};creative_dynamic_init(&high,rate,disc_param_default[10],1);creative_dynamic_init(&low,rate,disc_param_default[10],1);
  double h=rms(&high,rate,6500),l=rms(&low,rate,200);assert(h<.11&&l>.135);
  DiscDynamic bass={0},air={0};creative_dynamic_init(&bass,rate,disc_param_default[11],0);creative_dynamic_init(&air,rate,disc_param_default[11],0);double b=rms(&bass,rate,250),a=rms(&air,rate,6000);assert(b<.13&&a>.135);creative_dynamic_reset(&bass);float silence[2]={0,0};creative_dynamic(&bass,silence);assert(silence[0]==0&&silence[1]==0);
  DiscDelay *d=creative_delay_create(rate,disc_param_default[9]);assert(d);unsigned at=d->length;float p[2]={1,0};creative_delay(d,p);assert(fabs(p[0]-.8)<1e-6);for(unsigned i=1;i<at;i++){p[0]=p[1]=0;creative_delay(d,p);assert(p[0]==0&&p[1]==0);}p[0]=p[1]=0;creative_delay(d,p);assert(fabs(p[0]-.2)<1e-6&&p[1]==0);creative_delay_reset(d);for(unsigned i=0;i<at*2;i++){p[0]=p[1]=0;creative_delay(d,p);assert(p[0]==0&&p[1]==0);}creative_delay_destroy(d);
  printf("PASS %u: mono center preserved; deesser high=%g low=%g; dynamic bass=%g air=%g; delay timing/reset\n",rate,h,l,b,a);
 }
 float p[12][12],old[12][12];memcpy(old,disc_param_default,sizeof(old));char text[4096];int n=snprintf(text,sizeof(text),"1");for(unsigned i=0;i<8;i++)for(unsigned j=0;j<12;j++)n+=snprintf(text+n,sizeof(text)-n," %.6g",old[i][j]);assert(disc_profile_parse(text,p)&&!memcmp(p,old,sizeof(p)));assert(!disc_profile_parse("2 NaN",p));assert(!disc_profile_parse("1 0",p));puts("PASS legacy profile defaults and malformed profile rejection");return 0;
}
