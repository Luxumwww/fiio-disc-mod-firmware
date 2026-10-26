#ifndef DISC_HAAS_PARAMS_H
#define DISC_HAAS_PARAMS_H
#include <math.h>
/* Chain-only ID13 preserves the legacy 12x12 ABI and PEQ ID12. */
#define DISC_HAAS_TYPE 13u
#define DISC_HAAS_MASK 8192u
static const float disc_haas_default[12]={3.f,.35f,2.f,150.f,0.f,0,0,0,0,0,0,0};
static const float disc_haas_range[12][2]={{0,20},{0,1},{0,2},{0,500},{-12,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};
static inline int disc_haas_params_valid(const float *p){
 for(unsigned i=0;i<12;i++)if(!isfinite(p[i])||p[i]<disc_haas_range[i][0]||p[i]>disc_haas_range[i][1])return 0;
 return p[2]==0||p[2]==1||p[2]==2;
}
#endif
