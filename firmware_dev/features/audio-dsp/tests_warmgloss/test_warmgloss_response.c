/* Test-only measurements of the project wrapper; no player or PCM access. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../warmgloss.h"
#define PI 3.14159265358979323846264338327950288
typedef struct {double gain_db,thd_pct,peak,dc;} Result;
static Result measure(unsigned rate,const float *p,double hz,double amplitude) {
    DiscWarmGloss *s=disc_warmgloss_create(rate,p);assert(s);
    double phase_s=0,phase_c=1,step=2*PI*hz/rate,step_s=sin(step),step_c=cos(step);
    double real[10]={0},imag[10]={0},peak=0,sum=0;
    unsigned warm=rate/2,count=rate;
    for(unsigned i=0;i<warm+count;i++) {
        float io[2]={(float)(amplitude*phase_s),(float)(amplitude*phase_s)};
        disc_warmgloss_sample(s,io);
        assert(isfinite(io[0])&&io[0]==io[1]);
        if(i>=warm) {
            double hs=phase_s,hc=phase_c,y=io[0];
            for(unsigned h=0;h<10;h++) {
                if((h+1)*hz<rate*.49) {real[h]+=y*hc;imag[h]+=y*hs;}
                double ns=hs*phase_c+hc*phase_s;
                hc=hc*phase_c-hs*phase_s;hs=ns;
            }
            sum+=y;if(fabs(y)>peak)peak=fabs(y);
        }
        double next_s=phase_s*step_c+phase_c*step_s;
        phase_c=phase_c*step_c-phase_s*step_s;phase_s=next_s;
        if(!(i%4096u)){double norm=1/sqrt(phase_s*phase_s+phase_c*phase_c);phase_s*=norm;phase_c*=norm;}
    }
    double fundamental=2*hypot(real[0],imag[0])/count,harmonics=0;
    for(unsigned h=1;h<10;h++)harmonics+=real[h]*real[h]+imag[h]*imag[h];
    Result result={20*log10(fundamental/amplitude),100*(2*sqrt(harmonics)/count)/fundamental,peak,sum/count};
    disc_warmgloss_destroy(s);return result;
}
static uint32_t state=0x12345678u;
static float random_sample(void) {state^=state<<13;state^=state>>17;state^=state<<5;return (float)(((double)state/4294967295.0)*2-1);}
int main(void) {
    const unsigned rates[]={8000,16000,44100,48000,88200,96000,176400,192000,384000};
    const unsigned frequencies[]={20,50,100,300,1000,3000,6000,10000};
    puts("kind,rate,frequency_hz,amplitude,amount,tube,gain_db,thd_pct,peak,dc");
    for(unsigned ri=0;ri<sizeof(rates)/sizeof(rates[0]);ri++)
    for(unsigned fi=0;fi<sizeof(frequencies)/sizeof(frequencies[0]);fi++) {
        unsigned rate=rates[ri],hz=frequencies[fi];if(hz>=rate*.45)continue;
        Result r=measure(rate,disc_warmgloss_default,hz,.0001);
        printf("small_signal,%u,%u,.0001,.45,.55,%.9g,%.9g,%.9g,%.9g\n",rate,hz,r.gain_db,r.thd_pct,r.peak,r.dc);
    }
    const unsigned musical_rates[]={44100,48000,96000,192000};
    const float amounts[]={0,.45f,1};
    const double amplitudes[]={.1,.5,.9};
    const unsigned mid_frequencies[]={300,1000};
    for(unsigned ri=0;ri<4;ri++)for(unsigned ai=0;ai<3;ai++)
    for(unsigned mi=0;mi<3;mi++)for(unsigned fi=0;fi<2;fi++) {
        float p[12];memcpy(p,disc_warmgloss_default,sizeof(p));p[0]=amounts[ai];
        Result r=measure(musical_rates[ri],p,mid_frequencies[fi],amplitudes[mi]);
        printf("mid_tone,%u,%u,%.3g,%.3g,.55,%.9g,%.9g,%.9g,%.9g\n",musical_rates[ri],mid_frequencies[fi],amplitudes[mi],(double)p[0],r.gain_db,r.thd_pct,r.peak,r.dc);
    }
    double max_zero=0,max_stress=0;
    unsigned stress_contexts=0,symmetry_frames=0;
    /* All 64 control endpoint combinations, all supported boundary rates. */
    for(unsigned ri=0;ri<sizeof(rates)/sizeof(rates[0]);ri++)for(unsigned corner=0;corner<64;corner++) {
        float p[12]={0};for(unsigned j=0;j<6;j++)p[j]=disc_warmgloss_range[j][(corner>>j)&1u];
        DiscWarmGloss *a=disc_warmgloss_create(rates[ri],p),*b=disc_warmgloss_create(rates[ri],p);assert(a&&b);
        assert(disc_warmgloss_memory(a)<2048);
        for(unsigned frame=0;frame<8192;frame++) {
            float l=frame<256?0:random_sample()*2,r=frame<256?0:random_sample()*2;
            if(frame==300)l=NAN;if(frame==301)r=INFINITY;
            float x[2]={l,r},y[2]={r,l};disc_warmgloss_sample(a,x);disc_warmgloss_sample(b,y);
            assert(isfinite(x[0])&&isfinite(x[1]));
            assert(x[0]==y[1]&&x[1]==y[0]);
            for(unsigned ch=0;ch<2;ch++)if(fabs(x[ch])>max_stress)max_stress=fabs(x[ch]);
            symmetry_frames++;
        }
        disc_warmgloss_reset(a);disc_warmgloss_reset(b);
        for(unsigned frame=0;frame<4096;frame++) {
            float x[2]={0,0},y[2]={0,0};disc_warmgloss_sample(a,x);disc_warmgloss_sample(b,y);
            assert(!memcmp(x,y,sizeof(x))&&x[0]==x[1]);
            if(fabs(x[0])>max_zero)max_zero=fabs(x[0]);
        }
        disc_warmgloss_destroy(a);disc_warmgloss_destroy(b);stress_contexts++;
    }
    fprintf(stderr,"WarmGloss endpoints: contexts=%u, swapped_stereo_frames=%u, zero_peak=%.17g (%.6f dBFS), stress_peak=%.17g; all finite; exact stereo swap/reset/mono passed\n",stress_contexts,symmetry_frames,max_zero,20*log10(max_zero),max_stress);
    return 0;
}
