#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../audio-visuals/visual_normalize.h"
#include "../audio-visuals/visual_reconstruct.h"
static int pcm[15360*2];static short out[15360][2],reference[15360][2];
int main(void){
 const unsigned rates[]={44100,48000};const unsigned frequencies[]={20,1000,10000,18000,20000};
 for(unsigned r=0;r<2;r++)for(unsigned f=0;f<5;f++){
  unsigned rate=rates[r],samples=rate/50;double w=2*3.141592653589793*frequencies[f]/rate;
  for(unsigned i=0;i<samples+64;i++){pcm[i*2]=(int)(sin(w*i)*16000*65536.);pcm[i*2+1]=(int)(cos(w*i)*16000*65536.);}
  DiscVisualGain gain={0};unsigned count=disc_visual_scope_convert(&gain,(const unsigned char *)pcm,32,2,samples,rate,1,0,out);assert(count==samples*4);
  double peak_error=0;for(unsigned i=0;i<count;i++)for(unsigned c=0;c<2;c++){double phase=w*(32.+i*.25),expect=(c?cos(phase):sin(phase))*16000,error=fabs(out[i][c]-expect);if(error>peak_error)peak_error=error;}
  assert(peak_error<40);printf("PASS %uHz tone%u peak_error=%.4f /16000\n",rate,frequencies[f],peak_error);
 }
 for(unsigned i=0;i<15360*2;i++)pcm[i]=(int)((i*2654435761u)>>2);
 const unsigned high[]={88200,96000,176400,192000,352800,384000};
 for(unsigned i=0;i<6;i++){DiscVisualGain a={0},b={0};unsigned samples=disc_scope_samples(high[i]);unsigned n=disc_visual_scope_convert(&a,(const unsigned char *)pcm,32,2,samples,high[i],12,0,out);disc_visual_convert(&b,(const unsigned char *)pcm,32,2,samples,disc_scope_step(high[i]),12,0,reference);assert(n==samples&&!memcmp(out,reference,n*4));}
 puts("PASS original88.2..384k scope conversion byte-for-byte unchanged; real low-rate context and shared L/R reconstruction");fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
