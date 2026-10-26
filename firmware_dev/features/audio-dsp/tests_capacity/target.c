#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include "../dsp_api.h"
#include "../chain_format.h"
#include "../../settings-debug/cache_sync.h"
void *stereofx_reference_create(unsigned,const float*);
void stereofx_reference_process(void*,const double*,double*,unsigned);
void stereofx_reference_destroy(void*);
#define CHECK(x) do{if(!(x)){printf("FAIL line%d %s\n",__LINE__,#x);return 2;}}while(0)
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
static int capacity_checks(const DiscDspApi *api,void *(*create)(unsigned,const DiscChain*)){
 unsigned rates[]={8000,44100,48000,96000,192000,384000};int x[1024],y[1024];
 for(unsigned r=0;r<6;r++)for(unsigned n=9;n<=16;n+=7){DiscChain c={0};c.count=n;
  for(unsigned i=0;i<n;i++){c.nodes[i].id=i+1;c.nodes[i].type=i+1==n?7:5;c.nodes[i].enabled=i+1==n;memcpy(c.nodes[i].params,disc_param_default[c.nodes[i].type],sizeof(c.nodes[i].params));}
  void *d=create(rates[r],&c);CHECK(d);for(unsigned k=0;k<40;k++){for(unsigned i=0;i<1024;i++)x[i]=(int)(sin((i+k*1024)*.07)*214748364.);api->process(d,x,512);}unsigned limited,invalid,av,lat,clip;float peak,gain;api->diagnostics(d,&limited,&invalid,&peak,&gain);api->stats(d,&av,&lat,&clip);CHECK(!invalid&&!clip&&isfinite(gain));
  api->set_mask(d,0);for(unsigned k=0;k<80;k++){memset(x,0,sizeof(x));api->process(d,x,512);}for(unsigned i=0;i<1024;i++)x[i]=y[i]=i&1?INT32_MIN+(int)i:INT32_MAX-(int)i;api->process(d,x,512);CHECK(!memcmp(x,y,sizeof(x)));api->destroy(d);
  for(unsigned i=0;i<n;i++)c.nodes[i].enabled=i<9;CHECK(!create(rates[r],&c));printf("PASS Galactic single effect at slot%u/rate%u:9/16 saved accepted, finite output/OFF exact;9 enabled rejected\n",n,rates[r]);fflush(stdout);
 }return 0;
}
static int check(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;setpriority(PRIO_PROCESS,0,10);const char *path=getenv("DSP_CANDIDATE"),*old=getenv("DSP_BASELINE");void *lib=dlopen(path,2),*base=dlopen(old,2);CHECK(lib&&base&&module_cache_sync(path)&&module_cache_sync(old));
 const DiscDspApi *(*get)(void)=dlsym(lib,"disc_dsp_get_api"),*(*getold)(void)=dlsym(base,"disc_dsp_get_api");void *(*create)(unsigned,const DiscChain*)=dlsym(lib,"disc_dsp_create_chain");CHECK(get&&getold&&create);const DiscDspApi *api=get(),*before=getold();CHECK(api->abi==3);CHECK(!capacity_checks(api,create));
 unsigned rates[]={8000,44100,48000,96000,192000,384000},rng=1100;int x[1024],y[1024];DiscChain c={0};c.count=1;c.nodes[0].id=1;c.nodes[0].type=15;c.nodes[0].enabled=1;
 for(unsigned r=0;r<6;r++){
  for(unsigned v=0;v<7;v++){memcpy(c.nodes[0].params,disc_stereofx_default,sizeof(c.nodes[0].params));if(v){unsigned j=(v-1)/2;c.nodes[0].params[j]=disc_stereofx_range[j][(v-1)&1];}void *d=create(rates[r],&c);CHECK(d);
   for(unsigned k=0;k<48;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=(int)rng;}api->process(d,x,512);}unsigned lim,bad,av,lat,clip;float peak,gain;api->diagnostics(d,&lim,&bad,&peak,&gain);api->stats(d,&av,&lat,&clip);CHECK(!bad&&!clip&&isfinite(peak)&&isfinite(gain));
   api->set_mask(d,0);memset(x,0,sizeof(x));for(unsigned k=0;k<80;k++)api->process(d,x,512);for(unsigned i=0;i<1024;i++)x[i]=y[i]=(i&1)?INT32_MIN+(int)i:INT32_MAX-(int)i;api->process(d,x,512);CHECK(!memcmp(x,y,sizeof(x)));api->set_mask(d,4096);memset(x,0,sizeof(x));for(unsigned k=0;k<48;k++){api->process(d,x,512);for(unsigned i=0;i<1024;i++)CHECK(x[i]>=-2048&&x[i]<=2048);}api->destroy(d);
  }
  memcpy(c.nodes[0].params,disc_stereofx_default,sizeof(c.nodes[0].params));void *d=create(rates[r],&c),*ref=stereofx_reference_create(rates[r],disc_stereofx_default);CHECK(d&&ref);double in[1024],wet[1024];float ring[1536][2]={{0}};unsigned delay=2*((rates[r]+999)/1000-1),pos=0,frames=0;unsigned maxdelta=0;
  unsigned blocks=(rates[r]/4+511)/512;for(unsigned k=0;k<blocks;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=(int)rng/32;in[i]=(float)(x[i]*(1.f/2147483648.f));}stereofx_reference_process(ref,in,wet,512);
   for(unsigned i=0;i<512;i++){for(unsigned ch=0;ch<2;ch++){float dry=(float)in[2*i+ch],w=(float)wet[2*i+ch],mixed=dry+(w-dry);y[2*i+ch]=(int)(ring[pos][ch]*2147483648.f);ring[pos][ch]=mixed;}if(++pos==delay)pos=0;}
   api->process(d,x,512);if(frames>rates[r]/10)for(unsigned i=0;i<1024;i++){long long diff=(long long)x[i]-y[i];unsigned a=(unsigned)(diff<0?-diff:diff);if(a>maxdelta)maxdelta=a;}frames+=512;
  }CHECK(maxdelta==0);api->destroy(d);stereofx_reference_destroy(ref);printf("PASS rate=%u: 7 endpoint contexts/OFF/reset; steady S32 output equals unmodified Airwindows StereoFX, max_delta=%u\n",rates[r],maxdelta);fflush(stdout);
 }
 for(unsigned r=1;r<6;r++)for(unsigned fx=0;fx<12;fx++){if(fx==1)continue;void *a=before->create(rates[r],1u<<fx,"."),*b=api->create(rates[r],1u<<fx,".");CHECK(a&&b);for(unsigned k=0;k<48;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=y[i]=(int)rng/8;}before->process(a,x,512);api->process(b,y,512);CHECK(!memcmp(x,y,sizeof(x)));}before->destroy(a);api->destroy(b);}
 puts("PASS 55 legacy single-effect contexts byte-identical to1.11.0");
 void *(*oldcreate)(unsigned,const DiscChain*)=dlsym(base,"disc_dsp_create_chain");CHECK(oldcreate);
 for(unsigned r=0;r<6;r++)for(unsigned v=0;v<25;v++){
  DiscChain keep={0};keep.count=1;keep.nodes[0].id=1;keep.nodes[0].type=14;keep.nodes[0].enabled=1;memcpy(keep.nodes[0].params,disc_ffhaas_default,sizeof(keep.nodes[0].params));if(v)keep.nodes[0].params[(v-1)/2]=disc_ffhaas_range[(v-1)/2][(v-1)&1];void *a=oldcreate(rates[r],&keep),*b=create(rates[r],&keep);CHECK(a&&b);
  for(unsigned k=0;k<20;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=y[i]=(int)rng/8;}before->process(a,x,512);api->process(b,y,512);CHECK(!memcmp(x,y,sizeof(x)));}before->destroy(a);api->destroy(b);
 }puts("PASS150 retained FFmpeg Haas contexts byte-identical to1.11.0");

 if(getenv("DISC_BENCH"))for(unsigned r=2;r<6;r++){
  memcpy(c.nodes[0].params,disc_stereofx_default,sizeof(c.nodes[0].params));void *d=create(rates[r],&c);CHECK(d);memset(x,0,sizeof(x));for(unsigned k=0;k<50;k++)api->process(d,x,512);
  unsigned blocks=(rates[r]/2+511)/512;double start=now();for(unsigned k=0;k<blocks;k++){for(unsigned i=0;i<1024;i++){rng=rng*1664525u+1013904223u;x[i]=(int)rng/32;}api->process(d,x,512);}double ms=(now()-start)*1000.,audio=(double)blocks*512000./rates[r];printf("BENCH single_StereoFX rate=%u audio_ms=%.2f elapsed_ms=%.2f realtime_ratio=%.4f\n",rates[r],audio,ms,ms/audio);fflush(stdout);CHECK(ms<audio*.85);api->destroy(d);
 }
 puts("PASS isolated only: no PCM opened, no playback/volume/config operations");return 0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void(*init)(void),void(*fini)(void),void(*rtld)(void),void *stack){(void)fn;int(*next)(int(*)(int,char**,char**),int,char**,void(*)(void),void(*)(void),void(*)(void),void*)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(check,argc,argv,init,fini,rtld,stack):90;}
