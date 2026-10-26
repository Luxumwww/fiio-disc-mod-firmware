#ifndef DISC_FFMPEG_TEST_STUB_H
#define DISC_FFMPEG_TEST_STUB_H
/* Test-only frame/memory plumbing. af_haas.c itself is compiled unmodified. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef offsetof
#define offsetof(t,m) __builtin_offsetof(t,m)
#endif
typedef struct {int unused;} AVClass;
typedef struct {const char *name,*help;int offset,type;union{double dbl;int64_t i64;} default_val;double min,max;int flags;const char *unit;} AVOption;
enum{AV_OPT_TYPE_DOUBLE,AV_OPT_TYPE_INT,AV_OPT_TYPE_BOOL,AV_OPT_TYPE_CONST};
#define AV_OPT_FLAG_AUDIO_PARAM 1
#define AV_OPT_FLAG_FILTERING_PARAM 2
#define AVFILTER_DEFINE_CLASS(n) static const AVClass n##_class={0}
#define AVERROR(n) (-(n))
#define NULL_IF_CONFIG_SMALL(s) (s)
#define av_cold
enum AVSampleFormat {AV_SAMPLE_FMT_NONE=-1,AV_SAMPLE_FMT_DBL=0};
typedef struct{int nb_channels;} AVChannelLayout;
#define AV_CHANNEL_LAYOUT_STEREO {.nb_channels=2}
typedef struct{int unused;} AVFilterFormatsConfig;
typedef struct AVFilterContext AVFilterContext;
typedef struct {AVFilterContext *dst;int sample_rate;} AVFilterLink;
struct AVFilterContext{void *priv;AVFilterLink *outputs[1];};
typedef struct{uint8_t *data[8];int nb_samples;} AVFrame;
typedef struct{const char *name;int type;int(*filter_frame)(AVFilterLink*,AVFrame*);int(*config_props)(AVFilterLink*);} AVFilterPad;
enum{AVMEDIA_TYPE_AUDIO};
typedef struct{struct{const char *name,*description;const AVClass *priv_class;}p;size_t priv_size;void(*uninit)(AVFilterContext*);const AVFilterPad *inputs;const void *outputs;int(*query_formats)(const AVFilterContext*,AVFilterFormatsConfig**,AVFilterFormatsConfig**);} FFFilter;
#define FILTER_INPUTS(x) .inputs=(x)
#define FILTER_OUTPUTS(x) .outputs=&(x)
#define FILTER_QUERY_FUNC2(x) .query_formats=(x)
static const int ff_audio_default_filterpad=0;
static inline void av_freep(void *p){void **q=p;free(*q);*q=0;}
static inline void *av_calloc(size_t n,size_t s){return calloc(n,s);}
static inline int ff_set_common_formats_from_list2(const AVFilterContext *c,AVFilterFormatsConfig **i,AVFilterFormatsConfig **o,const enum AVSampleFormat *f){(void)c;(void)i;(void)o;(void)f;return 0;}
static inline int ff_set_common_channel_layouts_from_list2(const AVFilterContext *c,AVFilterFormatsConfig **i,AVFilterFormatsConfig **o,const AVChannelLayout *l){(void)c;(void)i;(void)o;(void)l;return 0;}
static inline int av_frame_is_writable(AVFrame *f){(void)f;return 1;}
static inline AVFrame *ff_get_audio_buffer(AVFilterLink *l,int n){(void)l;AVFrame *f=calloc(1,sizeof(*f));if(f){f->nb_samples=n;f->data[0]=calloc((size_t)n*2,sizeof(double));}return f;}
static inline void av_frame_copy_props(AVFrame *a,AVFrame *b){(void)a;(void)b;}
static inline void av_frame_free(AVFrame **f){if(*f){free((*f)->data[0]);free(*f);*f=0;}}
static inline int ff_filter_frame(AVFilterLink *l,AVFrame *f){(void)l;(void)f;return 0;}
#endif
