#ifndef DISC_MANDALA_API_H
#define DISC_MANDALA_API_H
#include "../xy/visual_api.h"
#define DISC_MANDALA_ABI 1u
typedef DiscXyApi DiscMandalaApi;
const DiscMandalaApi *disc_mandala_get_api(unsigned abi);
#endif
