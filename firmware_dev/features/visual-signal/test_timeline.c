#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ring.h"
static DiscVisualRing ring;
static int input[3840*2],output[4096*2];
int main(void){
 ring.magic=DISC_VS_MAGIC;ring.abi=1;ring.owner=42;ring.bytes=sizeof(ring);ring.request=3;ring.ui_mode=4;
 for(unsigned rate_index=0;rate_index<3;rate_index++){
  unsigned rate=rate_index==0?44100:rate_index==1?192000:384000,cursor=0,previous=0,feeds=0,skipped=0;DiscVisualClock clock={0};
  memset(&ring.pcm,0,sizeof(ring.pcm));ring.request+=2;ring.valid=0;ring.request_seen=0;
  for(unsigned t=0;t<4000;t+=16){
   ring.lease=t;
   while(cursor<3840||cursor<(unsigned long long)t*rate/1000){for(unsigned i=0;i<3840;i++){input[i*2]=(int)(cursor+i);input[i*2+1]=-(int)(cursor+i);}disc_vs_write(&ring,input,3840,rate,4,6,t);cursor+=3840;}
   DiscVisualWindow window;unsigned end;
   assert(disc_vs_meta(&ring,rate,4,42,t,&window));
   if(!disc_vs_clock(&clock,&window,946,t,&end)){skipped++;continue;}
   assert(disc_vs_read_window(&ring,output,946,rate,4,42,t,end,0,&window));
   for(unsigned i=0;i<946;i++){assert(output[i*2]==(int)(end-946+i));assert(output[i*2+1]==-(int)(end-946+i));}
   if(feeds){unsigned increment=end-previous;assert(increment==rate*16/1000||increment==rate*16/1000+1);}
   previous=end;feeds++;
  }
  assert(feeds>230&&skipped<12);printf("PASS chronological %u Hz: %u distinct UI windows, %u warmup waits\n",rate,feeds,skipped);
 }
 fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
