#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../powersag2_core.h"
void *powersag2_reference_create(double,double);
void powersag2_reference_sample(void *,double[2]);
void powersag2_reference_destroy(void *);
static uint32_t rng=0x14579234u;
static double random_sample(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return ((double)rng/4294967295.0)*2-1;}
static double source(unsigned i,unsigned ch){
    switch((i/4096u)%8u){
    case 0:return 0;
    case 1:return 1e-25*(ch?-1:1);
    case 2:return .9*sin(.006283185307179586*(i+ch*123));
    case 3:return .1*(ch?-.5:1);
    case 4:return (i%1024u<256)?.8*sin(.015*i):0;
    case 5:return random_sample();
    case 6:return random_sample()*2;
    default:return (i%300u==0)?(ch?-1:1):0;
    }
}
int main(void){
    const double ranges[]={0,.1,.3,.55,.8,1};
    const double wets[]={0,.5,.65,1};
    unsigned contexts=0,frames=0;
    double maximum_error=0,range_one_peak=0;
    double *in=malloc(32768u*2*sizeof(double)),*out=malloc(32768u*2*sizeof(double));assert(in&&out);
    assert(!disc_powersag2_create(-.1,.5));assert(!disc_powersag2_create(1.1,.5));
    assert(!disc_powersag2_create(.5,-.1));assert(!disc_powersag2_create(.5,1.1));
    assert(!disc_powersag2_create(NAN,.5));assert(!disc_powersag2_create(.5,INFINITY));
    disc_powersag2_reset(NULL);disc_powersag2_destroy(NULL);assert(!disc_powersag2_memory(NULL));
    for(unsigned a=0;a<6;a++)for(unsigned b=0;b<4;b++){
        DiscPowerSag2 *s=disc_powersag2_create(ranges[a],wets[b]);
        void *ref=powersag2_reference_create(ranges[a],wets[b]);assert(s&&ref);
        assert(disc_powersag2_memory(s)<270000u);
        for(unsigned i=0;i<32768;i++)for(unsigned ch=0;ch<2;ch++)in[i*2+ch]=source(i,ch);
        for(unsigned i=0;i<32768;){unsigned n=(i%251u)+1;if(n>32768-i)n=32768-i;disc_powersag2_process_double(s,in+i*2,out+i*2,n);i+=n;}
        for(unsigned i=0;i<32768;i++){
            double expected[2]={in[i*2],in[i*2+1]};powersag2_reference_sample(ref,expected);
            for(unsigned ch=0;ch<2;ch++){
                double actual=out[i*2+ch],error=fabs(actual-expected[ch]);assert(isfinite(actual));
                if(error>maximum_error)maximum_error=error;
                if(memcmp(&actual,&expected[ch],sizeof(actual))){fprintf(stderr,"Mismatch A=%.9g B=%.9g frame=%u ch=%u actual=%.17g expected=%.17g\n",ranges[a],wets[b],i,ch,actual,expected[ch]);return 1;}
                if(a==5&&b==3&&fabs(actual)>range_one_peak)range_one_peak=fabs(actual);
            }
        }
        disc_powersag2_reset(s);powersag2_reference_destroy(ref);ref=powersag2_reference_create(ranges[a],wets[b]);
        for(unsigned i=0;i<4096;i++){
            double x=i<100?0:random_sample(),io[2]={x,x},expected[2]={x,x};
            disc_powersag2_sample_double(s,io);powersag2_reference_sample(ref,expected);
            assert(io[0]==io[1]&&!memcmp(io,expected,sizeof(io)));
        }
        powersag2_reference_destroy(ref);disc_powersag2_destroy(s);contexts++;frames+=32768+4096;
    }
    free(in);free(out);
    printf("PowerSag2 WinVST double oracle: contexts=%u stereo_frames=%u max_double_error=%.17g; full ring wraps/reset/in-place/mono/validation passed; max_range_full_wet_peak=%.9g\n",contexts,frames,maximum_error,range_one_peak);
    return 0;
}
