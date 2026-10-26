#ifndef DISC_STEREOFX_PARAMS_H
#define DISC_STEREOFX_PARAMS_H
#include <math.h>
#define DISC_STEREOFX_TYPE 15u
#define DISC_STEREOFX_MASK 32768u
static const float disc_stereofx_default[12]={.5f,0,0,0,0,0,0,0,0,0,0,0};
static const float disc_stereofx_range[3][2]={{0,1},{0,.7f},{0,1}};
static inline int disc_stereofx_params_valid(const float *p){if(!p)return 0;for(unsigned i=0;i<12;i++)if(!isfinite(p[i])||(i<3?(p[i]<disc_stereofx_range[i][0]||p[i]>disc_stereofx_range[i][1]):p[i]!=0))return 0;return 1;}
#endif
