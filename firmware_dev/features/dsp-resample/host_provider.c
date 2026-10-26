#define DISC_DSP_UNITY
#define DISC_HALL_SCALAR
#include "../audio-dsp/dsp.c"
/* The binding harness exercises one warmgloss instance, never Dragonfly.
 * Actual MIPS provider regressions separately use the complete C++ backend. */
void *disc_dragonfly_create(unsigned r){(void)r;return 0;}
void *disc_dragonfly_create_configured(unsigned r,const float *p){(void)r;(void)p;return 0;}
void disc_dragonfly_destroy(void *p){(void)p;}
void disc_dragonfly_reset(void *p){(void)p;}
unsigned disc_dragonfly_memory(void *p){(void)p;return 0;}
void disc_dragonfly_sample(void *p,float *x){(void)p;(void)x;}
