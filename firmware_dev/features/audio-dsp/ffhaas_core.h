#ifndef DISC_FFHAAS_CORE_H
#define DISC_FFHAAS_CORE_H
typedef struct DiscFFHaas DiscFFHaas;
DiscFFHaas *disc_ffhaas_create(unsigned,const float *);
void disc_ffhaas_destroy(DiscFFHaas *);
void disc_ffhaas_reset(DiscFFHaas *);
unsigned disc_ffhaas_memory(const DiscFFHaas *);
void disc_ffhaas_process_double(DiscFFHaas *,const double *,double *,unsigned);
void disc_ffhaas_sample(DiscFFHaas *,float *);
#endif
