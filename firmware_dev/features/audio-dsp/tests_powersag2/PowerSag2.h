#ifndef DISC_POWERSAG2_REFERENCE_H
#define DISC_POWERSAG2_REFERENCE_H
#include <stdint.h>
#include <math.h>
typedef int VstInt32;
class PowerSag2 {
public:
    double dL[16386]={0},dR[16386]={0},controlL=0,controlR=0;
    int gcount=0;
    uint32_t fpdL=16386u,fpdR=16386u;
    float A=.3f,B=.5f;
    void processReplacing(float **,float **,VstInt32);
    void processDoubleReplacing(double **,double **,VstInt32);
};
#endif
