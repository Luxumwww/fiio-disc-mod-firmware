/* Execute the real output wrapper with separate DSP and original-EQ oracles. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define DISC_DOP_TEST 1
#define DISC_VISUAL_TAP_TEST 1
static int dop_stock_profile,classified,stage,taps,return_code;
static unsigned char native_byte;
#define PC_BYTE(a) native_byte
#define PC_FN(a,t) ((t)native_dispatch(a))
static int dop_source(void){return classified;}
static void dsp_process_buffer(void *data,int bytes,int rate){assert(stage==0&&rate==384000);stage=1;int *p=data;for(int i=0;i<bytes/4;i++)p[i]+=3;}
static int native_eq(void *data,int bytes,int rate){assert(stage==1&&rate==384000);stage=2;int *p=data;for(int i=0;i<bytes/4;i++)p[i]*=2;return return_code;}
static void vs_post_eq(const void *data,int bytes,int rate){assert(stage==2&&rate==384000);stage=3;const int *p=data;for(int i=0;i<bytes/4;i++)assert(p[i]==(i+3)*2);taps++;}
static int native_sleep(unsigned us){(void)us;return 0;}
static void *native_dispatch(unsigned a){return a==0x826440?(void *)native_sleep:(void *)native_eq;}
#include "../usbdac-dop/dop_eq.h"
int main(void){int pcm[64];for(int trial=0;trial<3;trial++){for(int i=0;i<64;i++)pcm[i]=i;stage=0;return_code=trial==0?0:trial==1?-1:7;assert(dop_eq_filter(pcm,sizeof(pcm),384000)==return_code);assert(stage==3&&taps==trial+1);for(int i=0;i<64;i++)assert(pcm[i]==(i+3)*2);}
 classified=1;stage=0;for(int i=0;i<64;i++)pcm[i]=i;assert(!dop_eq_filter(pcm,sizeof(pcm),384000));assert(stage==0&&taps==3);for(int i=0;i<64;i++)assert(pcm[i]==i);
 puts("PASS: production DSP -> native EQ -> immutable visual tap, original return retained, DoP bypasses all PCM processing");fflush(stdout);return 0;}

#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
