#ifndef DISC_STEREOFX_CORE_H
#define DISC_STEREOFX_CORE_H
typedef struct DiscStereoFx DiscStereoFx;
DiscStereoFx *disc_stereofx_create(unsigned,const float *);
void disc_stereofx_destroy(DiscStereoFx *);
void disc_stereofx_reset(DiscStereoFx *);
unsigned disc_stereofx_memory(const DiscStereoFx *);
void disc_stereofx_process_double(DiscStereoFx *,const double *,double *,int);
void disc_stereofx_sample(DiscStereoFx *,float *);
#endif
