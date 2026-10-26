#ifndef DISC_POWERSAG2_CORE_H
#define DISC_POWERSAG2_CORE_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct DiscPowerSag2 DiscPowerSag2;
DiscPowerSag2 *disc_powersag2_create(double range,double invwet);
void disc_powersag2_destroy(DiscPowerSag2 *);
void disc_powersag2_reset(DiscPowerSag2 *);
unsigned disc_powersag2_memory(const DiscPowerSag2 *);
void disc_powersag2_process_double(DiscPowerSag2 *,const double *interleaved_input,
                                  double *interleaved_output,int frames);
void disc_powersag2_sample_double(DiscPowerSag2 *,double io[2]);
/* Project linked detector adapter. Use an exclusive context, invwet=1 and a
 * positive finite detector (the project wrapper floors it to 1e-12). Advances
 * original L history only and returns its full-wet clamp before output mixing.
 * Do not interleave this and stereo processing on the same context. */
double disc_powersag2_linked_gain(DiscPowerSag2 *,double detector);
#ifdef __cplusplus
}
#endif
#endif
