#ifndef DISC_TUBE2_REFERENCE_H
#define DISC_TUBE2_REFERENCE_H
#include <stdint.h>
#include <math.h>
typedef int VstInt32;
class Tube2 {
public:
    float A=0.5f,B=0.5f;
    double previousSampleA=0,previousSampleB=0,previousSampleC=0;
    double previousSampleD=0,previousSampleE=0,previousSampleF=0;
    uint32_t fpdL=16386u,fpdR=16386u;
    float rate=48000;
    float getSampleRate(){return rate;}
    void processReplacing(float **,float **,VstInt32);
    void processDoubleReplacing(double **,double **,VstInt32);
};
#endif
