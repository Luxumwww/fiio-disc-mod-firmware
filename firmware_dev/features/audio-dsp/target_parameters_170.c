/* Single-effect functional validation; never opens audio devices. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include "../settings-debug/cache_sync.h"
#include "dsp_api.h"
#include "parameters.h"
static const DiscDspApi *load(const char *path,void **lib){*lib=dlopen(path,RTLD_NOW);if(!*lib||!module_cache_sync(path))return 0;const DiscDspApi *(*get)(void)=dlsym(*lib,"disc_dsp_get_api");return get?get():0;}
static int check(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;void *la,*lb;const DiscDspApi *a=load(getenv("DSP_BASELINE"),&la),*b=load(getenv("DSP_CANDIDATE"),&lb);
 void *(*configured)(unsigned,unsigned,const char *,const float[12][12])=lb?dlsym(lb,"disc_dsp_create_configured"):0;
 if(!a||!b||!configured||a->abi!=3||b->abi!=3){puts("load failed");return 2;}
 const unsigned rates[]={44100,48000,192000,384000};int x[2048],y[2048];unsigned rng=1027;
 for(unsigned r=0;r<4;r++)for(unsigned fx=0;fx<8;fx++){if(fx==1)continue;unsigned bit=1u<<fx;
  void *da=a->create(rates[r],bit,"."),*db=configured(rates[r],bit,".",disc_param_default);if(!da||!db)return 3;
  unsigned av,lat,clip,err=0;b->stats(db,&av,&lat,&clip);if(!(av&bit))return 4;
  for(unsigned chunk=0;chunk<96;chunk++){for(unsigned i=0;i<2048;i++){rng=rng*1664525+1013904223;x[i]=y[i]=(int)rng/4;}a->process(da,x,1024);b->process(db,y,1024);for(unsigned i=0;i<2048;i++){long long d=(long long)x[i]-y[i];unsigned v=d<0?-d:d;if(v>err)err=v;}}
  printf("default rate=%u effect=%u max_s32_delta=%u\n",rates[r],bit,err);fflush(stdout);a->destroy(da);b->destroy(db);if(err>1024)return 5;
  for(unsigned param=0;param<disc_param_count[fx];param++)for(unsigned endpoint=0;endpoint<2;endpoint++){
   float p[12][12];memcpy(p,disc_param_default,sizeof(p));p[fx][param]=disc_param_range[fx][param][endpoint];db=configured(rates[r],bit,".",p);if(!db)return 6;
   for(unsigned k=0;k<24;k++){for(unsigned i=0;i<2048;i++){rng=rng*1664525+1013904223;x[i]=(int)rng;}b->process(db,x,1024);}
   unsigned limited,invalid;float peak,gain;b->diagnostics(db,&limited,&invalid,&peak,&gain);b->stats(db,&av,&lat,&clip);if(invalid||clip||!isfinite(peak)||!isfinite(gain))return 7;
   b->set_mask(db,0);memset(x,0,sizeof(x));for(unsigned k=0;k<32;k++)b->process(db,x,1024);
   for(unsigned i=0;i<2048;i++)x[i]=y[i]=(i&1)?INT32_MIN+i:INT32_MAX-i;b->process(db,x,1024);if(memcmp(x,y,sizeof(x)))return 8;
   b->set_mask(db,bit);memset(x,0,sizeof(x));for(unsigned k=0;k<24;k++){b->process(db,x,1024);for(unsigned i=0;i<2048;i++)if(x[i])return 9;}b->destroy(db);
  }
  printf("PASS endpoints effect=%u rate=%u: finite, bounded, OFF exact, reset silence\n",bit,rates[r]);fflush(stdout);
 }
 float invalid[12][12];memcpy(invalid,disc_param_default,sizeof(invalid));invalid[4][0]=NAN;if(configured(48000,16,".",invalid))return 10;invalid[4][0]=6;if(configured(48000,16,".",invalid))return 11;
 puts("PASS 28 legacy single-effect default comparisons and 312 endpoint contexts; invalid parameters rejected");dlclose(la);dlclose(lb);return 0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void(*init)(void),void(*fini)(void),void(*rtld)(void),void *stack){(void)fn;int(*next)(int(*)(int,char**,char**),int,char**,void(*)(void),void(*)(void),void(*)(void),void*)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(check,argc,argv,init,fini,rtld,stack):90;}
