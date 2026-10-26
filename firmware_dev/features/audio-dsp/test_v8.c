#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "v8.h"
int main(int argc,char **argv){
 unsigned rate=argc>1?(unsigned)atoi(argv[1]):48000;V8 *v=v8_create(rate);assert(v);FILE *f=argc>2?fopen(argv[2],"wb"):0;
 double energy=0,last=0;float max=0;for(unsigned n=0;n<rate*3;n++){float a[2]={n==0?1.f:0,0};v8_sample(v,a);for(unsigned c=0;c<2;c++){assert(isfinite(a[c]));energy+=a[c]*a[c];if(n>=rate*2)last+=a[c]*a[c];if(fabsf(a[c])>max)max=fabsf(a[c]);}if(f)assert(fwrite(a,sizeof(float),2,f)==2);}
 assert(energy>.001&&energy<10&&last<energy*1e-6&&max<2);if(f)fclose(f);v8_reset(v);for(unsigned n=0;n<rate/10;n++){float a[2]={0,0};v8_sample(v,a);assert(a[0]==0&&a[1]==0);}
 printf("PASS rate=%u energy=%.9f late_energy=%.9g peak=%.6f memory=%u reset_silence=PASS\n",rate,energy,last,max,v->bytes);v8_destroy(v);return 0;
}
