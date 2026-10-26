/* Airwindows Tube2 original arithmetic/order, exposed inline to its voicing wrapper. */
#ifndef DISC_TUBE2_CORE_INLINE_H
#define DISC_TUBE2_CORE_INLINE_H
#include <stdint.h>
#include <math.h>
#include "tube2_core.h"
#include "tube2_core_state.h"

static __attribute__((always_inline)) inline void disc_tube2_sample_double_inline(DiscTube2 *s,double io[2]){
#ifdef __clang__
#pragma clang fp contract(off)
#endif
    if(!s||!io)return;
    double overallscale=s->overallscale;
    double inputPad=s->A;
    int powerfactor=s->powerfactor;
    double asymPad=s->asymPad;
    double gainscaling=s->gainscaling;
    double outputscaling=s->outputscaling;
		double inputSampleL = io[0];
		double inputSampleR = io[1];
		if (fabs(inputSampleL)<1.18e-23) inputSampleL = s->fpdL * 1.18e-17;
		if (fabs(inputSampleR)<1.18e-23) inputSampleR = s->fpdR * 1.18e-17;
		
		if (inputPad < 1.0) {
			inputSampleL *= inputPad;
			inputSampleR *= inputPad;
		}
		
		if (overallscale > 1.9) {
			double stored = inputSampleL;
			inputSampleL += s->previousSampleA; s->previousSampleA = stored; inputSampleL *= 0.5;
			stored = inputSampleR;
			inputSampleR += s->previousSampleB; s->previousSampleB = stored; inputSampleR *= 0.5;
		} //for high sample rates on this plugin we are going to do a simple average		
		
		if (inputSampleL > 1.0) inputSampleL = 1.0;
		if (inputSampleL < -1.0) inputSampleL = -1.0;
		if (inputSampleR > 1.0) inputSampleR = 1.0;
		if (inputSampleR < -1.0) inputSampleR = -1.0;
		
		//flatten bottom, point top of sine waveshaper L
		inputSampleL /= asymPad;
		double sharpen = -inputSampleL;
		if (sharpen > 0.0) sharpen = 1.0+sqrt(sharpen);
		else sharpen = 1.0-sqrt(-sharpen);
		inputSampleL -= inputSampleL*fabs(inputSampleL)*sharpen*0.25;
		//this will take input from exactly -1.0 to 1.0 max
		inputSampleL *= asymPad;
		//flatten bottom, point top of sine waveshaper R
		inputSampleR /= asymPad;
		sharpen = -inputSampleR;
		if (sharpen > 0.0) sharpen = 1.0+sqrt(sharpen);
		else sharpen = 1.0-sqrt(-sharpen);
		inputSampleR -= inputSampleR*fabs(inputSampleR)*sharpen*0.25;
		//this will take input from exactly -1.0 to 1.0 max
		inputSampleR *= asymPad;
		//end first asym section: later boosting can mitigate the extreme
		//softclipping of one side of the wave
		//and we are asym clipping more when Tube is cranked, to compensate
		
		//original Tube algorithm: powerfactor widens the more linear region of the wave
		double factor = inputSampleL; //Left channel
		for (int x = 0; x < powerfactor; x++) factor *= inputSampleL;
		if ((powerfactor % 2 == 1) && (inputSampleL != 0.0)) factor = (factor/inputSampleL)*fabs(inputSampleL);
		factor *= gainscaling;
		inputSampleL -= factor;
		inputSampleL *= outputscaling;
		factor = inputSampleR; //Right channel
		for (int x = 0; x < powerfactor; x++) factor *= inputSampleR;
		if ((powerfactor % 2 == 1) && (inputSampleR != 0.0)) factor = (factor/inputSampleR)*fabs(inputSampleR);
		factor *= gainscaling;
		inputSampleR -= factor;
		inputSampleR *= outputscaling;
		
		if (overallscale > 1.9) {
			double stored = inputSampleL;
			inputSampleL += s->previousSampleC; s->previousSampleC = stored; inputSampleL *= 0.5;
			stored = inputSampleR;
			inputSampleR += s->previousSampleD; s->previousSampleD = stored; inputSampleR *= 0.5;
		} //for high sample rates on this plugin we are going to do a simple average
		//end original Tube. Now we have a boosted fat sound peaking at 0dB exactly
		
		//hysteresis and spiky fuzz L
		double slew = s->previousSampleE - inputSampleL;
		if (overallscale > 1.9) {
			double stored = inputSampleL;
			inputSampleL += s->previousSampleE; s->previousSampleE = stored; inputSampleL *= 0.5;
		} else s->previousSampleE = inputSampleL; //for this, need s->previousSampleC always
		if (slew > 0.0) slew = 1.0+(sqrt(slew)*0.5);
		else slew = 1.0-(sqrt(-slew)*0.5);
		inputSampleL -= inputSampleL*fabs(inputSampleL)*slew*gainscaling;
		//reusing gainscaling that's part of another algorithm
		if (inputSampleL > 0.52) inputSampleL = 0.52;
		if (inputSampleL < -0.52) inputSampleL = -0.52;
		inputSampleL *= 1.923076923076923;
		//hysteresis and spiky fuzz R
		slew = s->previousSampleF - inputSampleR;
		if (overallscale > 1.9) {
			double stored = inputSampleR;
			inputSampleR += s->previousSampleF; s->previousSampleF = stored; inputSampleR *= 0.5;
		} else s->previousSampleF = inputSampleR; //for this, need s->previousSampleC always
		if (slew > 0.0) slew = 1.0+(sqrt(slew)*0.5);
		else slew = 1.0-(sqrt(-slew)*0.5);
		inputSampleR -= inputSampleR*fabs(inputSampleR)*slew*gainscaling;
		//reusing gainscaling that's part of another algorithm
		if (inputSampleR > 0.52) inputSampleR = 0.52;
		if (inputSampleR < -0.52) inputSampleR = -0.52;
		inputSampleR *= 1.923076923076923;
		//end hysteresis and spiky fuzz section
		
		//begin 64 bit stereo floating point dither
		//int expon; frexp((double)inputSampleL, &expon);
		s->fpdL ^= s->fpdL << 13; s->fpdL ^= s->fpdL >> 17; s->fpdL ^= s->fpdL << 5;
		//inputSampleL += ((double(s->fpdL)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
		//frexp((double)inputSampleR, &expon);
		s->fpdR ^= s->fpdR << 13; s->fpdR ^= s->fpdR >> 17; s->fpdR ^= s->fpdR << 5;
		//inputSampleR += ((double(s->fpdR)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
		//end 64 bit stereo floating point dither
		
		io[0] = inputSampleL;
		io[1] = inputSampleR;

}
#endif
