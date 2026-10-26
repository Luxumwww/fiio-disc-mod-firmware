/* Core-only forced include. Disable test-environment switches and synchronous
 * diagnostics in the production audio path; keep the upstream math unchanged. */
#include <stdlib.h>
#include <stdio.h>
#include "disc_lhdcv5.h"
extern disc_lhdcv5_quality_stats disc_core_quality;
static inline char *disc_codec_no_env(const char *key){(void)key;return NULL;}
static inline int disc_codec_no_log(const char *fmt,...){(void)fmt;return 0;}
#define getenv disc_codec_no_env
#define printf disc_codec_no_log
