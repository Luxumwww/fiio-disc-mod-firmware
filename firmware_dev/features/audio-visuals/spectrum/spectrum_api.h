#ifndef DISC_SPECTRUM_API_H
#define DISC_SPECTRUM_API_H
#include "../xy/visual_api.h"
typedef DiscXyApi DiscSpectrumApi;
#define DISC_SPECTRUM_ABI 1u
#define DISC_SPECTRUM_SAMPLES 4096u
#define DISC_SPECTRUM_FROM_LEFT 262144u
/* Source-neutral signed16, same-time stereo pairs; exactly4096 per feed.
 * Angle: log frequency20Hz..20kHz clockwise. Radius: digital power/dBFS. */
#endif
