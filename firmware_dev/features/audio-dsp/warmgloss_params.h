#ifndef DISC_WARMGLOSS_PARAMS_H
#define DISC_WARMGLOSS_PARAMS_H
#include <math.h>
#define DISC_WARMGLOSS_TYPE 16u
#define DISC_WARMGLOSS_MASK 65536u
static const float disc_warmgloss_default[12]={.75f,.8f,3.f,-.75f,160.f,-1.f,.55f,.5f,0,0,0,0};
static const float disc_warmgloss_range[8][2]={{0,1},{0,1},{0,4},{-6,0},{60,250},{-12,0},{0,1},{0,1}};
static inline int disc_warmgloss_params_valid(const float *p){
 if(!p)return 0;
 for(unsigned i=0;i<12;i++)if(!isfinite(p[i])||(i<8?(p[i]<disc_warmgloss_range[i][0]||p[i]>disc_warmgloss_range[i][1]):p[i]!=0))return 0;
 return 1;
}
#endif
