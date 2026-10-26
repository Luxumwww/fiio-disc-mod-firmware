/* Bounded native differential validation. No audio devices or PCM recordings. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include "../settings-debug/cache_sync.h"
#include "dsp_api.h"
static const DiscDspApi *load(const char *path,void **lib){
 *lib=dlopen(path,RTLD_NOW);if(!*lib||!module_cache_sync(path))return 0;
 const DiscDspApi *(*get)(void)=dlsym(*lib,"disc_dsp_get_api");return get?get():0;
}
static int check(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;void *la,*lb;
 const DiscDspApi *a=load(getenv("DSP_BASELINE"),&la),*b=load(getenv("DSP_CANDIDATE"),&lb);if(!a||!b||(a->abi!=2&&a->abi!=3)||(b->abi!=2&&b->abi!=3))return 2;
 const unsigned rates[]={44100,48000,96000,176400,192000},chunks[]={1,777,1920,3840,7680,16384,32791},masks[]={7,2,6,0,7};
 unsigned rng=1234567;int *x=malloc(32791*8),*y=malloc(32791*8);if(!x||!y)return 3;
 for(unsigned r=0;r<5;r++){
  void *da=a->create(rates[r],7,getenv("DSP_BENCH_ROOT")),*db=b->create(rates[r],7,getenv("DSP_BENCH_ROOT"));if(!da||!db)return 4;
  unsigned aa,ab,al,bl,ac,bc;a->stats(da,&aa,&al,&ac);b->stats(db,&ab,&bl,&bc);if(aa!=7||ab!=7||al!=bl)return 5;
  unsigned peak=0,ci=0;
  for(unsigned phase=0;phase<5;phase++){
   a->set_mask(da,masks[phase]);b->set_mask(db,masks[phase]);
   for(unsigned pos=0;pos<rates[r]/2;){
    unsigned n=chunks[(ci++)%7];if(n>rates[r]/2-pos)n=rates[r]/2-pos;
    for(unsigned j=0;j<2*n;j++){rng=rng*1664525+1013904223;x[j]=(int)(rng>>8)-8388608;}
    memcpy(y,x,n*8);if(!a->process(da,x,n)||!b->process(db,y,n))return 6;
    for(unsigned j=0;j<2*n;j++){long long diff=(long long)x[j]-y[j];unsigned error=diff<0?-diff:diff;if(error>peak)peak=error;}
    pos+=n;
   }
  }
  /* Re-enabled effects must not replay stale audio or partial tail sums. */
  a->set_mask(da,0);b->set_mask(db,0);
  for(unsigned pos=0;pos<rates[r]/10;){unsigned n=1920;if(n>rates[r]/10-pos)n=rates[r]/10-pos;memset(x,0,n*8);memset(y,0,n*8);a->process(da,x,n);b->process(db,y,n);pos+=n;}
  a->set_mask(da,7);b->set_mask(db,7);
  for(unsigned pos=0;pos<rates[r]/2;){unsigned n=7680;if(n>rates[r]/2-pos)n=rates[r]/2-pos;memset(x,0,n*8);memset(y,0,n*8);a->process(da,x,n);b->process(db,y,n);for(unsigned j=0;j<2*n;j++)if(x[j]||y[j])return 7;pos+=n;}
  a->stats(da,&aa,&al,&ac);b->stats(db,&ab,&bl,&bc);
  printf("rate=%u max_s32_error=%u latency_frames=%u clipped=%u/%u reactivation_silence=PASS\n",rates[r],peak,bl,ac,bc);fflush(stdout);
  a->destroy(da);b->destroy(db);if(peak>512||ac!=bc)return 8;
 }
 void *da=a->create(48000,0,getenv("DSP_BENCH_ROOT")),*db=b->create(48000,0,getenv("DSP_BENCH_ROOT"));if(!da||!db)return 9;
 for(unsigned j=0;j<256;j++)x[j]=y[j]=(j&1)?INT32_MIN+j:INT32_MAX-j;
 int reference[256];memcpy(reference,x,sizeof(reference));a->process(da,x,128);b->process(db,y,128);if(memcmp(x,reference,sizeof(reference))||memcmp(y,reference,sizeof(reference)))return 10;
 a->destroy(da);b->destroy(db);free(x);free(y);dlclose(la);dlclose(lb);puts("PASS: five rates, independent stereo, mixed callback lengths, switch fades, OFF bit-exact");return 0;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(check,argc,argv,init,fini,rtld_fini,stack):90;
}
