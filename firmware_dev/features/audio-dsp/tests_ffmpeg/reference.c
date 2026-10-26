#include <assert.h>
#include "../third_party/ffmpeg-haas/n8.0/af_haas.c"
static void set(HaasContext *s,const char *name,double value){
 for(const AVOption *o=haas_options;o->name;o++)if(!strcmp(o->name,name)){char *p=(char*)s+o->offset;if(o->type==AV_OPT_TYPE_DOUBLE)*(double*)p=value;else *(int*)p=(int)value;return;}assert(0);
}
void *disc_haas_reference_create(unsigned rate,const float *p){
 HaasContext *s=calloc(1,sizeof(*s));if(!s)return 0;
 for(const AVOption *o=haas_options;o->name;o++){if(o->type==AV_OPT_TYPE_CONST)continue;set(s,o->name,o->type==AV_OPT_TYPE_DOUBLE?o->default_val.dbl:(double)o->default_val.i64);}
 if(p){const char *keys[]={"level_in","level_out","side_gain","middle_source","left_delay","left_balance","left_gain","left_phase","right_delay","right_balance","right_gain","right_phase"};for(unsigned i=0;i<12;i++)set(s,keys[i],i==3?(unsigned)p[i]%4:p[i]);set(s,"middle_phase",(unsigned)p[3]/4);}
 AVFilterContext c={.priv=s};AVFilterLink l={.dst=&c,.sample_rate=(int)rate};if(config_input(&l)){free(s);return 0;}return s;
}
void disc_haas_reference_process(void *context,const double *src,double *dst,unsigned n){
 HaasContext *s=context;if(src!=dst)memcpy(dst,src,n*2*sizeof(double));AVFilterContext c={.priv=s};AVFilterLink out={0},in={.dst=&c};c.outputs[0]=&out;AVFrame f={.nb_samples=(int)n};f.data[0]=(uint8_t*)dst;assert(!filter_frame(&in,&f));
}
void disc_haas_reference_destroy(void *p){AVFilterContext c={.priv=p};uninit(&c);free(p);}
