#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../tube2_core.h"
void *tube2_reference_create(unsigned,double,double);
void tube2_reference_sample(void *,double[2]);
void tube2_reference_destroy(void *);
static uint32_t rng=123456789u;
static double random_sample(void) {
    rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
    return ((double)rng/4294967295.0)*2.0-1.0;
}
static double input(unsigned i,unsigned ch,unsigned rate) {
    switch((i/1024u)%8u) {
    case 0:return 0;
    case 1:return 1e-25*(ch? -1:1);
    case 2:return .1*sin(6.283185307179586*(ch? 1513:997)*i/rate);
    case 3:return .8*sin(6.283185307179586*43*i/rate);
    case 4:return (ch? -.4:.4)+.2*random_sample();
    case 5:return 3*random_sample();
    case 6:return (i%127u==0)? (ch? -1:1):0;
    default:return random_sample();
    }
}
int main(void) {
    const unsigned rates[]={8000,44100,48000,88200,96000,176400,192000,384000};
    const double params[][2]={{0,.5},{.5,0},{.5,.5},{1,1},{.7,.85},{.2,.12}};
    double input_buffer[16384],output_buffer[16384],maximum_error=0;
    unsigned contexts=0,frames=0;
    assert(!disc_tube2_create(0,.5,.5));
    assert(!disc_tube2_create(384001,.5,.5));
    assert(!disc_tube2_create(48000,NAN,.5));
    assert(!disc_tube2_create(48000,.5,INFINITY));
    assert(!disc_tube2_create(48000,-.1,.5));
    assert(!disc_tube2_create(48000,.5,1.1));
    disc_tube2_reset(NULL);disc_tube2_destroy(NULL);
    assert(!disc_tube2_memory(NULL));
    for(unsigned ri=0;ri<sizeof(rates)/sizeof(rates[0]);ri++)
    for(unsigned pi=0;pi<sizeof(params)/sizeof(params[0]);pi++) {
        unsigned rate=rates[ri];
        DiscTube2 *s=disc_tube2_create(rate,params[pi][0],params[pi][1]);
        void *reference=tube2_reference_create(rate,params[pi][0],params[pi][1]);
        assert(s&&reference&&disc_tube2_memory(s)<1024);
        for(unsigned i=0;i<8192;i++)for(unsigned ch=0;ch<2;ch++)
            input_buffer[i*2+ch]=input(i,ch,rate);
        /* Uneven blocks prove coefficient caching and pointer adaptation. */
        unsigned offset=0;
        while(offset<8192) {
            unsigned sizes[]={1,7,16,257},n=sizes[offset%4];
            if(n>8192-offset)n=8192-offset;
            disc_tube2_process_double(s,input_buffer+offset*2,output_buffer+offset*2,(int)n);
            offset+=n;
        }
        for(unsigned i=0;i<8192;i++) {
            double expected[2]={input_buffer[i*2],input_buffer[i*2+1]};
            tube2_reference_sample(reference,expected);
            for(unsigned ch=0;ch<2;ch++) {
                double actual=output_buffer[i*2+ch];
                assert(isfinite(actual));
                double error=fabs(actual-expected[ch]);
                if(error>maximum_error)maximum_error=error;
                if(memcmp(&actual,&expected[ch],sizeof(actual))) {
                    fprintf(stderr,"Mismatch rate=%u A=%.9g B=%.9g frame=%u ch=%u actual=%.17g expected=%.17g\n",rate,params[pi][0],params[pi][1],i,ch,actual,expected[ch]);
                    return 1;
                }
            }
        }
        disc_tube2_reset(s);
        tube2_reference_destroy(reference);
        reference=tube2_reference_create(rate,params[pi][0],params[pi][1]);
        /* Reset, in-place operation and mono symmetry cover silence/RNG too. */
        for(unsigned i=0;i<2048;i++) {
            double x=(i<100)? 0:random_sample(),io[2]={x,x},expected[2]={x,x};
            disc_tube2_sample_double(s,io);
            tube2_reference_sample(reference,expected);
            assert(io[0]==io[1]);
            assert(!memcmp(io,expected,sizeof(io)));
        }
        tube2_reference_destroy(reference);disc_tube2_destroy(s);
        contexts++;frames+=8192+2048;
    }
    printf("Tube2 upstream oracle: contexts=%u stereo_frames=%u max_double_error=%.17g; reset/in-place/mono/validation passed\n",contexts,frames,maximum_error);
    return 0;
}
