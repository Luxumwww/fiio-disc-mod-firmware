#ifndef DISC_NATIVE_EQ_API_H
#define DISC_NATIVE_EQ_API_H
/* Optional pure computation API. The host retains native allocation, gain,
 * coefficients, state and all settings. Counts are stereo frames. */
typedef void (*DiscNativeEqKernel)(int *,unsigned,double *,const unsigned *,const float [10][5],double [10][5],double [10][5]);
void disc_dsp_native_eq(int *,unsigned,double *,const unsigned *,const float [10][5],double [10][5],double [10][5]);
#endif
