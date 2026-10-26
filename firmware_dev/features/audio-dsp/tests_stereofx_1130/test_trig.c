#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../stereofx_bounded_trig.h"
static uint64_t bits(double x){uint64_t b;memcpy(&b,&x,8);return b;}
int main(void){double max_abs=0;uint64_t max_ulp=0;
 for(unsigned i=0;i<=1000000;i++){
  double x=1.57079633*((double)i/1000000),a[2]={disc_sf_sin(x),disc_sf_cos(x)},b[2]={sin(x),cos(x)};
  for(unsigned c=0;c<2;c++){double e=fabs(a[c]-b[c]);if(e>max_abs)max_abs=e;uint64_t aa=bits(a[c]),bb=bits(b[c]);uint64_t u=aa>bb?aa-bb:bb-aa;if(u>max_ulp)max_ulp=u;if(e>=5e-16||u>3){printf("BAD x=%.17g c=%u a=%.17g b=%.17g e=%.17g ulp=%llu\n",x,c,a[c],b[c],e,(unsigned long long)u);return 2;}}
 }
 double special[]={0.,-0.,0x1p-1022,0x1p-1074,0x1p-27,nextafter(0x1p-27,0),0.78539816339744830962,nextafter(0.78539816339744830962,1.),1.57079632679489661923,1.57079633};
 for(unsigned i=0;i<sizeof special/sizeof special[0];i++){double x=special[i];assert(fabs(disc_sf_sin(x)-sin(x))<5e-16);assert(fabs(disc_sf_cos(x)-cos(x))<5e-16);}
 assert(isnan(disc_sf_sin(NAN))&&isnan(disc_sf_cos(NAN)));assert(isnan(disc_sf_sin(INFINITY)));assert(disc_sf_sin(-1)==sin(-1));assert(disc_sf_cos(100)==cos(100));
 printf("PASS 1000001 bounded angles and edge/tiny/NaN/fallback cases: max_abs=%.17g max_ulp=%llu\n",max_abs,(unsigned long long)max_ulp);return 0;
}
