#ifndef DISC_LHDCV5_H
#define DISC_LHDCV5_H
#include <stddef.h>
#include <stdint.h>
typedef struct {
 uint64_t frames,concealed_frames,clipped_frames,clipped_samples;
 uint32_t max_peak_q16,reserved;
} disc_lhdcv5_quality_stats;
__attribute__((visibility("default"))) void disc_lhdcv5_get_quality(disc_lhdcv5_quality_stats *out);
#define DISC_LHDCV5_API __attribute__((visibility("default")))
typedef struct disc_lhdcv5 disc_lhdcv5;
typedef struct { uint32_t rate, bits, frame_ms; } disc_lhdcv5_config;
typedef struct {
 uint64_t packets, frames, pcm_frames, malformed, decode_errors, gaps, duplicates;
 uint32_t pending_bytes, rate, source_bits, avg_decode_us, max_decode_us, over_budget;
} disc_lhdcv5_stats;
/* Callback receives interleaved S32_LE, full-scale aligned, exactly two channels.
 * Must consume or copy all frames, applying backpressure rather than dropping.
 * Returning nonzero aborts packet processing; caller must stop the transport. */
typedef int (*disc_lhdcv5_pcm)(void *,const int32_t *,size_t);
DISC_LHDCV5_API int disc_lhdcv5_parse_config(const uint8_t *,size_t,disc_lhdcv5_config *);
/* The upstream core has shared scratch. One decoder stream is enforced globally.
 * Transport owns the handle and serializes open/process/flush/close. */
DISC_LHDCV5_API disc_lhdcv5 *disc_lhdcv5_open(const uint8_t *,size_t);
DISC_LHDCV5_API void disc_lhdcv5_close(disc_lhdcv5 *);
DISC_LHDCV5_API void disc_lhdcv5_flush(disc_lhdcv5 *);
DISC_LHDCV5_API int disc_lhdcv5_packet(disc_lhdcv5 *,const uint8_t *,size_t,disc_lhdcv5_pcm,void *);
DISC_LHDCV5_API void disc_lhdcv5_get_stats(disc_lhdcv5 *,disc_lhdcv5_stats *);
#endif
