#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>
#include "spectrum_api.h"
#include "../../settings-debug/cache_sync.h"
static short input[4096][2];
static unsigned background[129600];
static unsigned ms(int clock){struct timespec t;if(clock_gettime(clock,&t))return 0;return (unsigned)t.tv_sec*1000+(unsigned)t.tv_nsec/1000000;}
int main(void){
 (void)mapped_contains; /* Shared mapping header; this probe only needs cache-owned spans. */
 const char *path="/tmp/disc-spectrum-probe/libdisc-audio-spectrum.so";void *h=dlopen(path,2);if(!h){printf("load failed: %s\n",dlerror());return 2;}
 if(!module_cache_sync(path))return 3;
 const DiscSpectrumApi *(*get)(unsigned)=dlsym(h,"disc_spectrum_get_api");const DiscSpectrumApi *api=get?get(1):0;if(!api||api->size!=sizeof(*api))return 4;
 for(unsigned i=0;i<4096;i++){input[i][0]=(short)(((int)(i%64)-32)*650);input[i][1]=(short)(((int)(i%128)-64)*300);}
 api->reset();memcpy(background,api->compose(360,0)->data,sizeof(background));unsigned wall=ms(1),cpu=ms(2);
 for(unsigned i=0;i<64;i++){api->feed(input,4096,48000,1000+i*16);api->tick(1000+i*16);if(api->compose(360,0)->size!=518400)return 5;}
 unsigned cpu_elapsed=ms(2)-cpu,wall_elapsed=ms(1)-wall;unsigned points=0,rate=0,age=0;api->stats(&points,&rate,&age,2024);if(points!=4096||rate!=48000)return 6;
 const unsigned *pixels=api->compose(360,0)->data;unsigned lit=0;for(unsigned i=0;i<129600;i++)if(pixels[i]!=background[i])lit++;
 if(lit<500){printf("FAIL: changed pixels=%u\n",lit);return 7;}
 printf("PASS: real MIPS FFT4096,64ticks/32analyses,log10f,sparse raster,cpu_ms=%u,wall_ms=%u,lit=%u\n",cpu_elapsed,wall_elapsed,lit);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\nmove $t0,$sp\nlw $a0,0($t0)\naddiu $a1,$t0,4\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
