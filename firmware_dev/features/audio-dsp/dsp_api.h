#ifndef DISC_DSP_API_H
#define DISC_DSP_API_H
enum { DISC_DSP_HALL=1, DISC_DSP_DX8=2, DISC_DSP_V3=DISC_DSP_DX8, DISC_DSP_COMPRESSOR=4, DISC_DSP_TUBE=8, DISC_DSP_V8=16, DISC_DSP_WARM=32, DISC_DSP_DRAGONFLY=64, DISC_DSP_GALACTIC=128, DISC_DSP_STEREO=256, DISC_DSP_DELAY=512, DISC_DSP_DEESSER=1024, DISC_DSP_DYNAMIC_EQ=2048 };
typedef struct {
 unsigned abi;
 void *(*create)(unsigned,unsigned,const char *);
 void (*destroy)(void *);
 void (*set_mask)(void *,unsigned);
 int (*process)(void *,int *,unsigned);
 void (*stats)(void *,unsigned *,unsigned *,unsigned *);
 void (*health)(void *,unsigned *,unsigned *);
 /* ABI3: sample-peak protection activity, invalid values, peak and gain. */
 void (*diagnostics)(void *,unsigned *,unsigned *,float *,float *);
} DiscDspApi;
#endif
