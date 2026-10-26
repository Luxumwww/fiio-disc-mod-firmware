#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../ffhaas_core.h"
#include "../ffhaas_params.h"
void *disc_haas_reference_create(unsigned,const float *);
void disc_haas_reference_process(void*,const double*,double*,unsigned);
void disc_haas_reference_destroy(void*);
int main(void){unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1100,contexts=0;double in[1024],actual[1024],expected[1024];
 for(unsigned r=0;r<6;r++)for(unsigned variant=0;variant<33;variant++){
  float p[12];memcpy(p,disc_ffhaas_default,sizeof(p));if(variant>=1&&variant<=24){unsigned j=(variant-1)/2;p[j]=disc_ffhaas_range[j][(variant-1)&1];}else if(variant>=25)p[3]=variant-25;
  DiscFFHaas *d=disc_ffhaas_create(rates[r],p);void *ref=disc_haas_reference_create(rates[r],variant==0?0:p);assert(d&&ref);
  for(unsigned block=0;block<40;block++){
   for(unsigned i=0;i<512;i++){rng=rng*1664525u+1013904223u;double l=(int)rng/2147483648.;rng=rng*1664525u+1013904223u;double v=(int)rng/2147483648.;if(block<10)v=l;else if(block<20)v=-l;in[2*i]=l;in[2*i+1]=v;}
   disc_haas_reference_process(ref,in,expected,512);disc_ffhaas_process_double(d,in,actual,512);
   for(unsigned i=0;i<1024;i++){assert(isfinite(actual[i]));if(actual[i]!=expected[i]){fprintf(stderr,"mismatch rate%u variant%u index%u %.17g %.17g\n",rates[r],variant,i,actual[i],expected[i]);return 2;}}
  }disc_ffhaas_reset(d);memset(in,0,sizeof(in));disc_ffhaas_process_double(d,in,actual,512);for(unsigned i=0;i<1024;i++)assert(!actual[i]);disc_ffhaas_destroy(d);disc_haas_reference_destroy(ref);contexts++;
 }
 for(unsigned i=0;i<12;i++){float p[12];memcpy(p,disc_ffhaas_default,sizeof(p));p[i]=NAN;assert(!disc_ffhaas_create(48000,p));}
 printf("PASS %u contexts/6 rates: port equals unmodified FFmpeg n8.0 sample-for-sample in double precision; defaults, all endpoints, eight source/polarity choices, mono/antiphase/stereo, reset\n",contexts);return 0;
}
