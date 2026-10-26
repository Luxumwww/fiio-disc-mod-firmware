#ifndef DISC_DRAGONFLY_ADAPTER_H
#define DISC_DRAGONFLY_ADAPTER_H
#ifdef __cplusplus
extern "C" {
#endif
void *disc_dragonfly_create(unsigned rate);
void *disc_dragonfly_create_configured(unsigned rate,const float *);
void disc_dragonfly_destroy(void *);
void disc_dragonfly_reset(void *);
unsigned disc_dragonfly_memory(void *);
void disc_dragonfly_sample(void *,float *);
#ifdef __cplusplus
}
#endif
#endif
