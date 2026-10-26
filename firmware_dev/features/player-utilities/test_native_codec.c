/* Isolated stock decoder + production gain core. Never opens an audio device. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
extern int open(const char *,int,...),close(int);
extern void *dlopen(const char *,int),*dlsym(void *,const char *);
extern char *dlerror(void);
#include "replaygain_core.h"
int test(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;
 setbuf(stdout,0);puts("isolated decoder starting");
 void *lib=dlopen("/usr/lib/libfiio_decoder.so",2);if(!lib){printf("dlopen: %s\n",dlerror());fflush(stdout);return 10;}
 int (*codec_open)(const char *,const char *,const void *,unsigned)=dlsym(lib,"AudioCodecOpen");int (*codec_read)(int,void *,unsigned)=dlsym(lib,"AudioCodecRead");int (*codec_close)(int)=dlsym(lib,"AudioCodecClose");if(!codec_open||!codec_read||!codec_close)return 11;
 void (*initialize)(void)=dlsym(lib,"wzInitAllFileHandle");if(!initialize)return 15;initialize();
 for(unsigned bits=16;bits<=32;bits+=8){
  char path[80];snprintf(path,sizeof path,"/tmp/disc-rg-native-%u.wav",bits);int h=codec_open(path,0,0,0);if(h<0){printf("open failure %d\n",h);return 12;}unsigned char buffer[65536];int n=codec_read(h,buffer,sizeof buffer);if(n<=0){printf("read failure %d\n",n);return 13;}
  printf("stock source=%u decoded_bytes=%d first16=",bits,n);for(unsigned j=0;j<16;j++)printf("%02x",buffer[j]);puts("");
  /* All tested source depths use the factory's signed 32-bit container. */
  unsigned decoded_bits=32;DiscRgRamp ramp={0};disc_rg_pcm_bytes(&ramp,buffer,(unsigned)n,192000,decoded_bits,2,.5f);
  int value=decoded_bits==16?(int)(short)((unsigned)buffer[0]|(unsigned)buffer[1]<<8):(int)((unsigned)buffer[0]|(unsigned)buffer[1]<<8|(unsigned)buffer[2]<<16|(unsigned)buffer[3]<<24);
  int expected=bits==16?327680000:bits==24?1280000:500000;printf("gained sample=%d expected=%d\n",value,expected);if(value!=expected)return 14;codec_close(h);
 }
 unsigned char pcm[32768];for(unsigned i=0;i<sizeof pcm;i++)pcm[i]=(unsigned char)i;
 DiscRgRamp ramp={0};struct timespec a,b;int timing=!clock_gettime(3,&a);
 for(unsigned i=0;i<100;i++)disc_rg_pcm_bytes(&ramp,pcm,sizeof pcm,192000,32,2,.999f);
 timing=timing&&!clock_gettime(3,&b);if(timing){double ms=(b.tv_sec-a.tv_sec)*1000.0+(b.tv_nsec-a.tv_nsec)*.000001;printf("RG_ONLY_192K_STEREO frames=409600 cpu_ms=%.3f per_block_ms=%.4f realtime_cpu_percent=%.3f\n",ms,ms/100,ms/2133.333333*100);}
 puts("PASS isolated stock decoder PCM container + production RG on actual MIPS; zero sound-device opens");fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal test\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
