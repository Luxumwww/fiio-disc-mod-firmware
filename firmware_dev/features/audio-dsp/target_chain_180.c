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
#include "chain_format.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);return 2;}}while(0)
static int check(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;const char *path=getenv("DSP_CANDIDATE");void *lib=dlopen(path,2);CHECK(lib&&module_cache_sync(path));
 const DiscDspApi *(*get)(void)=dlsym(lib,"disc_dsp_get_api");void *(*create)(unsigned,const DiscChain*)=dlsym(lib,"disc_dsp_create_chain");unsigned(*error)(void)=dlsym(lib,"disc_dsp_chain_error");CHECK(get&&create&&error);const DiscDspApi *api=get();
 DiscChain c={0},invalid;CHECK(disc_chain_parse("1 0\n",&c));CHECK(!disc_chain_parse("1 9",&invalid));CHECK(!disc_chain_parse("1 0 extra",&invalid));
 const unsigned rates[]={44100,48000,192000,384000};int x[2048],y[2048],z[2048];unsigned rng=179;
 for(unsigned r=0;r<4;r++){
  c.count=0;void *d=create(rates[r],&c);CHECK(d);for(unsigned i=0;i<2048;i++)x[i]=y[i]=(i&1)?INT32_MIN+i:INT32_MAX-i;api->process(d,x,1024);CHECK(!memcmp(x,y,sizeof(x)));api->destroy(d);
  c.count=2;for(unsigned i=0;i<2;i++){c.nodes[i].id=i+1;c.nodes[i].type=12;c.nodes[i].enabled=1;memcpy(c.nodes[i].params,disc_chain_eq_default,sizeof(c.nodes[i].params));}c.nodes[0].params[0]=200;c.nodes[0].params[1]=-6;c.nodes[1].params[0]=6000;c.nodes[1].params[1]=3;
  d=create(rates[r],&c);CHECK(d);DiscChain one=c;one.count=1;void *a=create(rates[r],&one);CHECK(a);int different=0;
  for(unsigned k=0;k<32;k++){for(unsigned i=0;i<2048;i++){rng=rng*1664525+1013904223;x[i]=y[i]=(int)rng/8;}api->process(d,x,1024);api->process(a,y,1024);if(k>20&&memcmp(x,y,sizeof(x)))different=1;}CHECK(different);
  unsigned lim,bad;float peak,gain;api->diagnostics(d,&lim,&bad,&peak,&gain);CHECK(!bad&&isfinite(peak)&&isfinite(gain));api->destroy(a);
  api->set_mask(d,0);memset(x,0,sizeof(x));for(unsigned k=0;k<32;k++)api->process(d,x,1024);for(unsigned i=0;i<2048;i++)x[i]=y[i]=(int)(i*381191);api->process(d,x,1024);CHECK(!memcmp(x,y,sizeof(x)));api->set_mask(d,4096);memset(x,0,sizeof(x));for(unsigned k=0;k<32;k++){api->process(d,x,1024);for(unsigned i=0;i<2048;i++)CHECK(x[i]==0);}api->destroy(d);
  c.nodes[0].type=3;memcpy(c.nodes[0].params,disc_param_default[3],sizeof(c.nodes[0].params));c.nodes[1].type=12;c.nodes[1].params[0]=250;c.nodes[1].params[1]=9;
  DiscChain reverse=c;reverse.nodes[0]=c.nodes[1];reverse.nodes[1]=c.nodes[0];d=create(rates[r],&c);a=create(rates[r],&reverse);CHECK(d&&a);different=0;
  for(unsigned k=0;k<40;k++){for(unsigned i=0;i<2048;i++){rng=rng*1664525+1013904223;x[i]=y[i]=(int)rng/4;}api->process(d,x,1024);api->process(a,y,1024);if(k>20&&memcmp(x,y,sizeof(x)))different=1;}CHECK(different);api->destroy(d);api->destroy(a);
  c.nodes[0].enabled=c.nodes[1].enabled=0;d=create(rates[r],&c);CHECK(d);for(unsigned i=0;i<2048;i++)x[i]=z[i]=(int)(i*571119);api->process(d,x,1024);CHECK(!memcmp(x,z,sizeof(x)));api->destroy(d);
  printf("PASS rate=%u empty/disabled exact; independent duplicate EQ; real serial order; bypass/reset finite\n",rates[r]);fflush(stdout);
 }
 invalid=c;invalid.nodes[1].id=invalid.nodes[0].id;CHECK(!create(48000,&invalid)&&error()==4);invalid=c;invalid.nodes[0].params[0]=NAN;CHECK(!create(48000,&invalid)&&error()==4);CHECK(!create(768000,&c)&&error()==1);
 puts("PASS chain validation and failure codes; no realtime or combined-performance benchmark");return 0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void(*init)(void),void(*fini)(void),void(*rtld)(void),void *stack){(void)fn;int(*next)(int(*)(int,char**,char**),int,char**,void(*)(void),void(*)(void),void(*)(void),void*)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(check,argc,argv,init,fini,rtld,stack):90;}
