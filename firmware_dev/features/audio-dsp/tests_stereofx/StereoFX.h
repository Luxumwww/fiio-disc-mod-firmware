#ifndef DISC_STEREOFX_REFERENCE_H
#define DISC_STEREOFX_REFERENCE_H
#include <stdint.h>
#include <math.h>
typedef int VstInt32;
class StereoFX {public:float A=0,B=0,C=0;double iirSampleA=0,iirSampleB=0;uint32_t fpdL=16386,fpdR=16386;bool flip=false;float rate=48000;float getSampleRate(){return rate;}void processReplacing(float**,float**,VstInt32);void processDoubleReplacing(double**,double**,VstInt32);};
#endif
