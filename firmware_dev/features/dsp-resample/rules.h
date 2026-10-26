#ifndef DISC_SRC_RULES_H
#define DISC_SRC_RULES_H
#define DISC_SRC_CONFIG "/usr/data/disc-runtime/module-data/disc-audio-dsp/resample.conf"
typedef struct {unsigned enabled,shift[6];} DiscSrcRules;
static const unsigned disc_src_rates[6]={88200,96000,176400,192000,352800,384000};
static inline DiscSrcRules disc_src_default(void){DiscSrcRules r={0,{1,1,1,1,1,1}};return r;}
static inline unsigned disc_src_max_shift(unsigned rate){unsigned n=0;while(rate/2>=44100&&!(rate&1)){rate/=2;n++;}return n;}
static inline int disc_src_rules_valid(const DiscSrcRules *r){if(r->enabled>1)return 0;for(unsigned i=0;i<6;i++)if(r->shift[i]>disc_src_max_shift(disc_src_rates[i]))return 0;return 1;}
static inline int disc_src_uint(const char **p,unsigned *out){while(**p==' '||**p=='\t'||**p=='\n'||**p=='\r')(*p)++;unsigned v=0,n=0;while(**p>='0'&&**p<='9'){if(++n>3)return 0;v=v*10+(unsigned)(*(*p)++-'0');}if(!n)return 0;*out=v;return 1;}
static inline int disc_src_parse(const char *text,DiscSrcRules *r){const char *p=text;unsigned version;if(!text||!disc_src_uint(&p,&version)||version!=1)return 0;DiscSrcRules value;if(!disc_src_uint(&p,&value.enabled))return 0;for(unsigned i=0;i<6;i++)if(!disc_src_uint(&p,&value.shift[i]))return 0;while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r')p++;if(*p||!disc_src_rules_valid(&value))return 0;*r=value;return 1;}
static inline unsigned disc_src_target(const DiscSrcRules *r,unsigned input){if(r->enabled&&disc_src_rules_valid(r))for(unsigned i=0;i<6;i++)if(input==disc_src_rates[i])return input>>r->shift[i];return input;}
#endif
