#ifndef DISC_FFHAAS_PARAMS_H
#define DISC_FFHAAS_PARAMS_H
#include <math.h>
#define DISC_FFHAAS_TYPE 14u
#define DISC_FFHAAS_MASK 16384u
/* All 13 upstream options fit 12 slots: middle_source0..3 and its inversion
 * flag share one enum0..7. No audio parameter is omitted or silently changed. */
static const float disc_ffhaas_default[12]={1,1,1,2,2.05f,-1,1,0,2.12f,1,1,1};
static const float disc_ffhaas_range[12][2]={{.015625f,64},{.015625f,64},{.015625f,64},{0,7},{0,40},{-1,1},{.015625f,64},{0,1},{0,40},{-1,1},{.015625f,64},{0,1}};
static inline int disc_ffhaas_params_valid(const float *p){
 for(unsigned i=0;i<12;i++)if(!isfinite(p[i])||p[i]<disc_ffhaas_range[i][0]||p[i]>disc_ffhaas_range[i][1])return 0;
 return p[3]==(float)(unsigned)p[3]&&p[7]==(float)(unsigned)p[7]&&p[11]==(float)(unsigned)p[11];
}
#endif
