#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "resample_api.h"
extern const DiscResampleApi *disc_resample_get_api(void);
static unsigned in_audio,allocations,random_state=127;
void *__real_malloc(size_t);void *__real_calloc(size_t,size_t);void *__real_realloc(void *,size_t);
void *__wrap_malloc(size_t n){if(in_audio)allocations++;return __real_malloc(n);}
void *__wrap_calloc(size_t a,size_t b){if(in_audio)allocations++;return __real_calloc(a,b);}
void *__wrap_realloc(void *p,size_t n){if(in_audio)allocations++;return __real_realloc(p,n);}
static int identity(void *p,float *x,unsigned n){(void)p;(void)x;(void)n;return 1;}
int main(void){
 const DiscResampleApi *api=disc_resample_get_api();int *pcm=calloc(32768,8);assert(pcm);unsigned contexts=0;
 for(unsigned r=0;r<6;r++)for(unsigned ratio=2;ratio<=8;ratio*=2){
  unsigned rate=(unsigned[]){88200,96000,176400,192000,352800,384000}[r],target=rate/ratio;if(target<44100)continue;
  void *s=api->create(rate,target);assert(s);
  for(unsigned k=0;k<400;k++){
   random_state=random_state*1664525u+1013904223u;unsigned n=k<8?(unsigned[]){1,1023,1024,1025,30720,8191,2,32768}[k]:1+(random_state%32768);
   memset(pcm,0,n*8);in_audio=1;int ok=api->process(s,pcm,n,identity,0);in_audio=0;
   if(!ok||allocations){fprintf(stderr,"FAIL rate=%u target=%u block=%u n=%u ok=%d allocations=%u\n",rate,target,k,n,ok,allocations);return 2;}
  }
  unsigned latency,errors,underflows,invalid;api->stats(s,&latency,&errors,&underflows,&invalid);assert(!errors&&!underflows&&!invalid);api->destroy(s);contexts++;
 }
 free(pcm);printf("PASS allocator instrumentation: %u mappings,400 random/extreme chunks each;0 audio allocations,errors,underflows\n",contexts);return 0;
}
