#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../powersag2_core.h"
void *powersag2_reference_create(double,double);
void powersag2_reference_sample(void *,double[2]);
void powersag2_reference_destroy(void *);
double powersag2_reference_clamp(void *,double);
static uint32_t state=0x15864279u;
static double detector(unsigned i){
    state^=state<<13;state^=state>>17;state^=state<<5;
    switch((i/8192u)%8u){
    case 0:return 1e-12;
    case 1:return .09*fabs(sin(.01*i))+1e-12;
    case 2:return .5;
    case 3:return .005;
    case 4:return i%1500u<1000u?.08:1e-12;
    case 5:return (double)state/4294967295.0;
    case 6:return 2*(double)state/4294967295.0;
    default:return 1e-12;
    }
}
int main(void){
    const double ranges[]={0,.1,.3,.55,.8,.852,1};
    double max_clamp_error=0,max_ratio_error=0;
    unsigned contexts=0,frames=0;
    assert(disc_powersag2_linked_gain(NULL,.1)==1);
    for(unsigned j=0;j<sizeof(ranges)/sizeof(ranges[0]);j++){
        DiscPowerSag2 *s=disc_powersag2_create(ranges[j],1);void *reference=powersag2_reference_create(ranges[j],1);assert(s&&reference);
        for(unsigned pass=0;pass<2;pass++){
            if(pass){disc_powersag2_reset(s);powersag2_reference_destroy(reference);reference=powersag2_reference_create(ranges[j],1);}
            for(unsigned i=0;i<65536;i++){
                double d=detector(i),io[2]={d,d};
                double gain=disc_powersag2_linked_gain(s,d);
                powersag2_reference_sample(reference,io);
                double expected=powersag2_reference_clamp(reference,d),ratio=io[0]/d;
                double error=fabs(gain-expected),ratio_error=fabs(gain-ratio);
                if(error>max_clamp_error)max_clamp_error=error;
                if(ratio_error>max_ratio_error)max_ratio_error=ratio_error;
                assert(isfinite(gain)&&gain>=0&&gain<=1);
                if(memcmp(&gain,&expected,sizeof(gain))||ratio_error>4.5e-16){fprintf(stderr,"Linked mismatch range=%.9g frame=%u gain=%.17g clamp=%.17g ratio=%.17g\n",ranges[j],i,gain,expected,ratio);return 1;}
                frames++;
            }
            contexts++;
        }
        disc_powersag2_destroy(s);powersag2_reference_destroy(reference);
    }
    printf("PowerSag2 linked detector: contexts=%u detector_frames=%u max_original_clamp_error=%.17g (bitwise exact) max_original_output_ratio_error=%.17g; ring/reset/bounds ASan/UBSan passed\n",contexts,frames,max_clamp_error,max_ratio_error);
    return 0;
}
