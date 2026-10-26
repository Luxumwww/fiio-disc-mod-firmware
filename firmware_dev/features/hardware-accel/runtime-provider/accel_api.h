#ifndef DISC_ACCEL_API_H
#define DISC_ACCEL_API_H
#define DISC_ACCEL_ABI 1u
#define DISC_ACCEL_CAP_DPU 1u
/* FPU/MSA and algorithm-specific DSP providers use separate future APIs.
 * DPU currently grants one exclusive overlay/controller lease. */
typedef void (*DiscAccelPaint)(unsigned *physical_bgra,unsigned frame,void *context);
typedef struct {unsigned enabled,active,failed,frames,copy_bytes,fullcopies,lease;char owner[64];} DiscAccelStats;
typedef struct {
 unsigned abi,size,capabilities,width,height,stride,rotation;
 unsigned (*claim)(const char *,DiscAccelPaint,void *);
 int (*valid)(unsigned);
 void (*release)(unsigned);
 void (*begin)(unsigned);
 void (*present)(unsigned);
 void (*set_enabled)(int);
 void (*stats)(unsigned,int,DiscAccelStats *);
 void (*pump)(void);
} DiscAccelApi;
/* Calls and paint callbacks are confined to the original UI thread/mutex.
 * A paint callback must remain mapped until lease release. Physical BGRA
 * surface:360x360, stride1440, rotate logical coordinates180deg. */
#endif
