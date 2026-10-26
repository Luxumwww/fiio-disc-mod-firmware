/* Offline target validation only: no PCM devices, DAC, or clock registers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include "../settings-debug/cache_sync.h"
#include "dsp_api.h"
static int check(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;
 const char *path=getenv("DSP_BENCH_LIB"),*root=getenv("DSP_BENCH_ROOT");
 if(!path||!root)return 2;void *lib=dlopen(path,RTLD_NOW);if(!lib||!module_cache_sync(path))return 3;
 const DiscDspApi *(*get)(void)=dlsym(lib,"disc_dsp_get_api");if(!get)return 4;const DiscDspApi *api=get();if(api->abi!=2)return 5;
 void *d=api->create(48000,0,root);if(!d)return 6;int orig[256],off[256];for(unsigned j=0;j<256;j++)off[j]=orig[j]=(j&1)?INT32_MIN+(int)j:INT32_MAX-(int)j;
 api->process(d,off,128);if(memcmp(off,orig,sizeof(off)))return 7;api->destroy(d);
 const unsigned rates[]={44100,48000,96000,192000,384000,768000};
 for(unsigned i=0;i<6;i++){
  unsigned rate=rates[i],frames=rate*2,available,latency,clipped;d=api->create(rate,2,root);if(!d)return 8;api->stats(d,&available,&latency,&clipped);if(!(available&2))return 9;
  int *pcm=calloc(frames*2,sizeof(int));if(!pcm)return 10;
  /* Finish the activation fade on silence before placing the impulses. */
  api->process(d,pcm,rate/10);pcm[0]=1073741824;pcm[275]=536870912;
  for(unsigned pos=0;pos<frames;){unsigned n=1+(pos*17+83)%777;if(n>frames-pos)n=frames-pos;api->process(d,pcm+pos*2,n);pos+=n;}
  char file[256];snprintf(file,sizeof(file),"/tmp/sdcard/disc-dsp-target-impulse-%u.i32",rate);FILE *f=fopen(file,"wb");if(!f||fwrite(pcm,8,frames,f)!=frames)return 11;fclose(f);
  api->set_mask(d,0);memset(pcm,0,frames*8);api->process(d,pcm,rate/10);api->set_mask(d,2);api->process(d,pcm,frames);
  for(unsigned j=0;j<frames*2;j++)if(pcm[j])return 12;
  free(pcm);api->destroy(d);printf("PASS target rate=%u latency_frames=%u; reactivation silence\n",rate,latency);fflush(stdout);
 }
 dlclose(lib);puts("PASS target off bit-exact and six-rate stereo convolution");return 0;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(check,argc,argv,init,fini,rtld_fini,stack):90;
}
