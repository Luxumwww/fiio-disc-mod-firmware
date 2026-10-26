/* StereoFX by Chris Johnson / Airwindows, Copyright (c) 2016, MIT.
 * Pinned source in third_party/airwindows-stereofx. VST wrapper replaced with
 * create/reset/interleaved callbacks; original Airwindows processing retained.
 * Bounded musl/Sun double trig kernels; inactive controls skip zero work.
 * Coefficients cached, deterministic equal RNG seeds, no IR or BASS dependency.
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "stereofx_core.h"
#include "stereofx_params.h"
#include "stereofx_bounded_trig.h"
struct DiscStereoFx {double rate,A,B,C,iirSampleA,iirSampleB,overallscale,stereowide,centersquish,density,sustain,iirAmount;uint32_t fpdL,fpdR;int flip;};
static void configure(DiscStereoFx *s,unsigned rate,const float *p){
 s->rate=rate;s->A=p[0];s->B=p[1];s->C=p[2];s->fpdL=s->fpdR=16386u;
 s->overallscale=1.0;s->overallscale/=44100.0;s->overallscale*=s->rate;
 s->stereowide=s->A;s->centersquish=s->C;s->density=s->stereowide*2.4;
 s->sustain=1.0-(1.0/(1.0+(s->density/7.0)));s->iirAmount=pow(s->B,3)/s->overallscale;
}
DiscStereoFx *disc_stereofx_create(unsigned rate,const float *p){if(rate<8000||rate>384000||!disc_stereofx_params_valid(p))return 0;DiscStereoFx *s=calloc(1,sizeof(*s));if(s)configure(s,rate,p);return s;}
void disc_stereofx_destroy(DiscStereoFx *s){free(s);}
void disc_stereofx_reset(DiscStereoFx *s){unsigned r=(unsigned)s->rate;float p[12]={(float)s->A,(float)s->B,(float)s->C};memset(s,0,sizeof(*s));configure(s,r,p);}
unsigned disc_stereofx_memory(const DiscStereoFx *s){(void)s;return sizeof(*s);}
static __attribute__((always_inline)) inline void disc_stereofx_frame(DiscStereoFx *s,double inputSampleL,double inputSampleR,double *left,double *right){
 double mid,side,bridgerectifier,count,offset;
 const double centersquish=s->centersquish,density=s->density,sustain=s->sustain,iirAmount=s->iirAmount,tight=-0.33333333333333;

		if (fabs(inputSampleL)<1.18e-23) inputSampleL = s->fpdL * 1.18e-17;
		if (fabs(inputSampleR)<1.18e-23) inputSampleR = s->fpdR * 1.18e-17;
		//assign working variables		
		mid = inputSampleL + inputSampleR;
		side = inputSampleL - inputSampleR;
		//assign mid and side. Now, High Impact code
		count = density;
		while (count > 1.0)
		{
			bridgerectifier = fabs(side)*1.57079633;
			if (bridgerectifier > 1.57079633) bridgerectifier = 1.57079633;
			//max value for sine function
			bridgerectifier = disc_sf_sin(bridgerectifier);
			if (side > 0.0) side = bridgerectifier;
			else side = -bridgerectifier;
			count = count - 1.0;
		}
		//we have now accounted for any really high density settings.
		bridgerectifier = fabs(side)*1.57079633;
		if (bridgerectifier > 1.57079633) bridgerectifier = 1.57079633;
		//max value for sine function
		bridgerectifier = disc_sf_sin(bridgerectifier);
		if (side > 0) side = (side*(1-count))+(bridgerectifier*count);
		else side = (side*(1-count))-(bridgerectifier*count);
		//blend according to density control
		//done first density. Next, sustain-reducer
		bridgerectifier = fabs(side)*1.57079633;
		if (bridgerectifier > 1.57079633) bridgerectifier = 1.57079633;
		bridgerectifier = (1-disc_sf_cos(bridgerectifier))*3.141592653589793;
		if (side > 0) side = (side*(1-sustain))+(bridgerectifier*sustain);
		else side = (side*(1-sustain))-(bridgerectifier*sustain);
		//done with High Impact code
		
		//now, Highpass code. Zero amount has zero state for this immutable context.
		if (iirAmount != 0.0) {
		offset = 0.666666666666666 + ((1-fabs(side))*tight);
		if (offset < 0) offset = 0;
		if (offset > 1) offset = 1;
		if (s->flip)
		{
			s->iirSampleA = (s->iirSampleA * (1 - (offset * iirAmount))) + (side * (offset * iirAmount));
			side = side - s->iirSampleA;
		}
		else
		{
			s->iirSampleB = (s->iirSampleB * (1 - (offset * iirAmount))) + (side * (offset * iirAmount));
			side = side - s->iirSampleB;
		}
		}
		//done with Highpass code
		
		if (centersquish != 0.0) {
		bridgerectifier = fabs(mid)/1.273239544735162;
		if (bridgerectifier > 1.57079633) bridgerectifier = 1.57079633;
		bridgerectifier = disc_sf_sin(bridgerectifier)*1.273239544735162;
		if (mid > 0) mid = (mid*(1-centersquish))+(bridgerectifier*centersquish);
		else mid = (mid*(1-centersquish))-(bridgerectifier*centersquish);
		}
		//done with the mid saturating section.
		
		inputSampleL = (mid+side)/2.0;
		inputSampleR = (mid-side)/2.0;
		
		//begin 64 bit stereo floating point dither
		//int expon; frexp((double)inputSampleL, &expon);
		s->fpdL ^= s->fpdL << 13; s->fpdL ^= s->fpdL >> 17; s->fpdL ^= s->fpdL << 5;
		//inputSampleL += ((double(s->fpdL)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
		//frexp((double)inputSampleR, &expon);
		s->fpdR ^= s->fpdR << 13; s->fpdR ^= s->fpdR >> 17; s->fpdR ^= s->fpdR << 5;
		//inputSampleR += ((double(s->fpdR)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
		//end 64 bit stereo floating point dither
		

 *left=inputSampleL;*right=inputSampleR;
}
void disc_stereofx_process_double(DiscStereoFx *s,const double *inputs,double *outputs,int frames){
 for(int i=0;i<frames;i++)disc_stereofx_frame(s,inputs[i*2],inputs[i*2+1],outputs+i*2,outputs+i*2+1);
}
void disc_stereofx_sample(DiscStereoFx *s,float *io){
 double left,right;disc_stereofx_frame(s,io[0],io[1],&left,&right);io[0]=(float)left;io[1]=(float)right;
}
