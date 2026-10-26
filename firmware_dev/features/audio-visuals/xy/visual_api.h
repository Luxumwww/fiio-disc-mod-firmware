#ifndef DISC_XY_VISUAL_API_H
#define DISC_XY_VISUAL_API_H
#define DISC_XY_ABI 1u
#define DISC_XY_FROM_RIGHT 65536u
/* Mode-independent visualization. PCM from any reviewed source, same-time
 * L/R pairs normalized to signed16; mono duplicates L into R. UI thread only. */
typedef struct {unsigned header,size;const void *data;} DiscVisualImage;
typedef struct {unsigned abi,size;
 void (*reset)(void);
 void (*feed)(const short (*stereo)[2],unsigned count,unsigned sampling,unsigned timestamp);
 void (*tick)(unsigned timestamp);
 const DiscVisualImage *(*compose)(unsigned progress,const unsigned *main_physical_bgra);
 unsigned (*revision)(void);
 void (*stats)(unsigned *samples,unsigned *rate,unsigned *age,unsigned timestamp);
} DiscXyApi;
#endif
