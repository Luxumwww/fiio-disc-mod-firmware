#ifndef DISC_POLAR_API_H
#define DISC_POLAR_API_H
#include "../xy/visual_api.h"
/* Same source-neutral PCM/image contract as XY, a separately owned renderer. */
typedef DiscXyApi DiscPolarApi;
typedef DiscXyApi DiscStereoApi;
#define DISC_POLAR_ABI 1u
#define DISC_STEREO_ABI 1u
#define DISC_POLAR_FROM_BOTTOM 131072u
#endif
