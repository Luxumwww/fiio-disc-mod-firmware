/* Offline, one effect only: bass sag response, old preset compatibility and bounds. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../warmgloss.h"
#define PI 3.14159265358979323846264338327950288
void *old_warmgloss_create(unsigned,const float *);
void old_warmgloss_sample(void *,float *);
void old_warmgloss_reset(void *);
void old_warmgloss_destroy(void *);
static uint32_t random_state=123456789u;
static float random_sample(void){random_state^=random_state<<13;random_state^=random_state>>17;random_state^=random_state<<5;return (float)(((double)random_state/4294967295.0)*2-1);}
static void burst(unsigned rate,double amplitude){
    float p[12],no_sag[12];memcpy(p,disc_warmgloss_default,sizeof(p));memcpy(no_sag,p,sizeof(p));no_sag[6]=0;
    DiscWarmGloss *a=disc_warmgloss_create(rate,p),*b=disc_warmgloss_create(rate,no_sag);assert(a&&b);
    double initial_energy=0,initial_base=0,late_energy=0,late_base=0;
    double gain_initial=1,gain_late=1,gain_at_end=1,release=-1,release_50=1,release_100=1;
    unsigned warm=rate/5,end=warm+rate/5,total=end+rate/5;
    for(unsigned i=0;i<total;i++){
        double t=(double)(i-warm)/rate;
        float x=(i>=warm&&i<end)?(float)(amplitude*sin(2*PI*80*t)):0;
        float left[2]={x,x},right[2]={x,x};disc_warmgloss_sample(a,left);disc_warmgloss_sample(b,right);
        assert(isfinite(left[0])&&left[0]==left[1]);
        if(i>=warm&&i<warm+rate/40){initial_energy+=left[0]*left[0];initial_base+=right[0]*right[0];}
        if(i>=end-rate/20&&i<end){late_energy+=left[0]*left[0];late_base+=right[0]*right[0];}
        if(i==warm+rate/80)gain_initial=a->sag_gain;
        if(i==end-rate/40)gain_late=a->sag_gain;
        if(i==end)gain_at_end=a->sag_gain;
        if(i>=end&&release<0&&a->sag_gain>=.99)release=(double)(i-end)*1000/rate;
        if(i==end+rate/20)release_50=a->sag_gain;
        if(i==end+rate/10)release_100=a->sag_gain;
    }
    double start_db=10*log10(initial_energy/initial_base),late_db=10*log10(late_energy/late_base);
    printf("burst,%u,80,%.3g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g\n",rate,amplitude,start_db,late_db,gain_initial,gain_late,gain_at_end,release,release_50,release_100);
    if(amplitude==.1){assert(gain_late>.99);assert(fabs(late_db)<.01);}
    if(amplitude==.8){assert(gain_late<.8);assert(late_db<-.5&&late_db>-9);assert(release>20&&release<150);}
    disc_warmgloss_destroy(a);disc_warmgloss_destroy(b);
}
static void tone(unsigned rate,double hz,double amplitude){
    DiscWarmGloss *s=disc_warmgloss_create(rate,disc_warmgloss_default);assert(s);
    double real[10]={0},imag[10]={0},peak=0;
    unsigned warm=rate/2,count=rate;
    for(unsigned i=0;i<warm+count;i++){
        double phase=2*PI*hz*i/rate,sn=sin(phase),cs=cos(phase),hs=sn,hc=cs;
        float io[2]={(float)(amplitude*sn),(float)(amplitude*sn)};disc_warmgloss_sample(s,io);assert(isfinite(io[0]));
        if(i>=warm){
            for(unsigned h=0;h<10;h++){
                if((h+1)*hz<rate*.49){real[h]+=io[0]*hc;imag[h]+=io[0]*hs;}
                double next=hs*cs+hc*sn;hc=hc*cs-hs*sn;hs=next;
            }
            if(fabs(io[0])>peak)peak=fabs(io[0]);
        }
    }
    double fundamental=2*hypot(real[0],imag[0])/count,harmonics=0;
    for(unsigned h=1;h<10;h++)harmonics+=real[h]*real[h]+imag[h]*imag[h];
    printf("tone,%u,%.9g,%.3g,%.9g,%.9g,%.9g,%.9g\n",rate,hz,amplitude,20*log10(fundamental/amplitude),100*2*sqrt(harmonics)/count/fundamental,peak,s->sag_gain);
    disc_warmgloss_destroy(s);
}
int main(void){
    const unsigned rates[]={8000,44100,48000,96000,192000,384000};
    puts("kind,rate,frequency_hz,amplitude,start_or_gain_db,late_or_thd,initial_or_peak,late_or_sag_gain,end_gain,release99_ms,gain_after50ms,gain_after100ms");
    for(unsigned ri=0;ri<6;ri++){burst(rates[ri],.1);burst(rates[ri],.8);}
    for(unsigned ri=1;ri<6;ri++){tone(rates[ri],300,.5);tone(rates[ri],10000,.0001);}
    unsigned old_contexts=0,stress_contexts=0,swapped_frames=0;
    double zero_peak=0,stress_peak=0;
    const float legacy[][12]={{.45,.55,1.8,-1.5,160,-1},{0,0,0,0,60,0},{1,1,4,-6,250,0}};
    for(unsigned ri=0;ri<6;ri++)for(unsigned pi=0;pi<3;pi++){
        DiscWarmGloss *current=disc_warmgloss_create(rates[ri],legacy[pi]);void *old=old_warmgloss_create(rates[ri],legacy[pi]);assert(current&&old);
        for(unsigned frame=0;frame<8192;frame++){
            float l=frame<128?0:random_sample(),r=frame<128?0:random_sample(),x[2]={l,r},y[2]={l,r};
            disc_warmgloss_sample(current,x);old_warmgloss_sample(old,y);assert(!memcmp(x,y,sizeof(x)));
        }
        disc_warmgloss_reset(current);old_warmgloss_reset(old);
        for(unsigned frame=0;frame<256;frame++){float x[2]={0,0},y[2]={0,0};disc_warmgloss_sample(current,x);old_warmgloss_sample(old,y);assert(!memcmp(x,y,sizeof(x)));}
        disc_warmgloss_destroy(current);old_warmgloss_destroy(old);old_contexts++;
    }
    for(unsigned ri=0;ri<6;ri++){
        DiscWarmGloss *mono=disc_warmgloss_create(rates[ri],disc_warmgloss_default),*anti=disc_warmgloss_create(rates[ri],disc_warmgloss_default);assert(mono&&anti);
        for(unsigned i=0;i<rates[ri]/3;i++){
            float x=(float)(.8*sin(2*PI*80*i/rates[ri])),m[2]={x,x},a[2]={x,-x};
            disc_warmgloss_sample(mono,m);disc_warmgloss_sample(anti,a);
            assert(mono->sag_gain==anti->sag_gain&&mono->sag_target==anti->sag_target);
        }
        assert(mono->sag_gain<.8);disc_warmgloss_destroy(mono);disc_warmgloss_destroy(anti);
    }
    for(unsigned ri=0;ri<6;ri++)for(unsigned corner=0;corner<256;corner++){
        float p[12]={0};for(unsigned j=0;j<8;j++)p[j]=disc_warmgloss_range[j][(corner>>j)&1u];
        DiscWarmGloss *a=disc_warmgloss_create(rates[ri],p),*b=disc_warmgloss_create(rates[ri],p);assert(a&&b&&disc_warmgloss_memory(a)<280000);
        for(unsigned i=0;i<8192;i++){
            float l=i<128?0:random_sample()*2,r=i<128?0:random_sample()*2;
            if(i==300)l=NAN;if(i==301)r=INFINITY;
            float x[2]={l,r},y[2]={r,l};disc_warmgloss_sample(a,x);disc_warmgloss_sample(b,y);
            assert(isfinite(x[0])&&isfinite(x[1])&&x[0]==y[1]&&x[1]==y[0]);
            assert(a->sag_gain>=0&&a->sag_gain<=1&&a->sag_target>=0&&a->sag_target<=1);
            for(unsigned ch=0;ch<2;ch++)if(fabs(x[ch])>stress_peak)stress_peak=fabs(x[ch]);swapped_frames++;
        }
        disc_warmgloss_reset(a);disc_warmgloss_reset(b);
        for(unsigned i=0;i<256;i++){float x[2]={0,0},y[2]={0,0};disc_warmgloss_sample(a,x);disc_warmgloss_sample(b,y);assert(!memcmp(x,y,sizeof(x))&&x[0]==x[1]);if(fabs(x[0])>zero_peak)zero_peak=fabs(x[0]);}
        disc_warmgloss_destroy(a);disc_warmgloss_destroy(b);stress_contexts++;
    }
    fprintf(stderr,"Sag wrapper: old_sag0_bitwise_contexts=%u stress_contexts=%u swapped_stereo_frames=%u zero_peak=%.17g hot_input_stress_peak=%.17g; all finite; mono/antiphase same detector; reset preserved\n",old_contexts,stress_contexts,swapped_frames,zero_peak,stress_peak);
    return 0;
}
