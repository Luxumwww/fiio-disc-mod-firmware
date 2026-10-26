#ifndef DISC_TUBE2_CORE_H
#define DISC_TUBE2_CORE_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct DiscTube2 DiscTube2;
DiscTube2 *disc_tube2_create(unsigned rate, double input_pad, double tube);
void disc_tube2_destroy(DiscTube2 *);
void disc_tube2_reset(DiscTube2 *);
unsigned disc_tube2_memory(const DiscTube2 *);
void disc_tube2_process_double(DiscTube2 *, const double *interleaved_input,
                               double *interleaved_output, int frames);
void disc_tube2_sample_double(DiscTube2 *, double io[2]);
#ifdef __cplusplus
}
#endif
#endif
