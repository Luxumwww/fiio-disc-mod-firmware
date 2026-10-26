#include "../audio-visuals/xy/visual_api.h"
#include "../audio-visuals/polar/polar_api.h"
#include "../audio-visuals/spectrum/spectrum_api.h"
#include "../audio-visuals/mandala/mandala_api.h"
#include "../audio-visuals/page_navigation.h"
static const DiscXyApi *xy_api;
static const DiscPolarApi *polar_api;
static const DiscStereoApi *stereo_api;
static const DiscSpectrumApi *spectrum_api;
static const DiscMandalaApi *mandala_api;
/* One visible page and one DPU lease, independent of renderer selection. */
static const DiscXyApi *page_api;
static unsigned page_kind;
static unsigned xy_progress,xy_frame_progress;
static const DiscVisualImage *xy_frame;
static unsigned xy_drawn_generation[3],xy_drawn_progress[3];
static int dpu_content[3];
static void xy_feed_pcm(const unsigned char *,unsigned,unsigned,unsigned,unsigned,unsigned);
