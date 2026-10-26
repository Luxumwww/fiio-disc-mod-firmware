#ifndef DISC_SPECTRUM_CORE_H
#define DISC_SPECTRUM_CORE_H
#define DISC_FFT_SIZE 1024
#define DISC_BANDS 72
typedef struct {
  int re[DISC_FFT_SIZE], im[DISC_FFT_SIZE];
  unsigned magnitude[DISC_FFT_SIZE/2];
  unsigned char levels[DISC_BANDS];
} DiscSpectrumCore;
/* Signed Q15 mono input. No heap, floating-point, libc or audio-device access. */
void disc_spectrum_analyze(DiscSpectrumCore *, const short *, unsigned sample_rate);
int disc_pcm_mono(const unsigned char *, unsigned channels, unsigned format, short *);
#endif
