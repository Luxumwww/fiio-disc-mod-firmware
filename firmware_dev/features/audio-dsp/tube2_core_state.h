#ifndef DISC_TUBE2_CORE_STATE_H
#define DISC_TUBE2_CORE_STATE_H
#include <stdint.h>
#include "tube2_core.h"
struct DiscTube2 {
    double rate, overallscale, asymPad, gainscaling, outputscaling;
    double previousSampleA,previousSampleB,previousSampleC,previousSampleD;
    double previousSampleE,previousSampleF;
    float A,B;
    int powerfactor;
    uint32_t fpdL,fpdR;
};
#endif
