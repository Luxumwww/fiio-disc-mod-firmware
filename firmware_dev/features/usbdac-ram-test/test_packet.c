#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "packet.h"
int main(void){
 unsigned char clip[264],out[4096];
 for(unsigned i=0;i<33;i++)for(unsigned j=0;j<8;j++)clip[i*8+j]=(j==0||j==4)?0:(j==3||j==7)?((i&1)?250:5):(unsigned char)(i*7+j);
 assert(ram_packet_valid(clip,sizeof(clip)));
 /* Odd clip length, arbitrary wrap and partial-write advancement. */
 for(unsigned pos=0;pos<sizeof(clip);pos+=8)for(unsigned first=5;first<=250;first+=245){
  ram_packet_copy(out,clip,sizeof(clip),pos,512,first);
  assert(ram_packet_valid(out,sizeof(out)));
  for(unsigned i=0;i<512;i++)for(unsigned j=0;j<8;j++)if(j!=3&&j!=7)assert(out[i*8+j]==clip[(pos+i*8)%sizeof(clip)+j]);
  unsigned char next[256];ram_packet_copy(next,clip,sizeof(clip),(pos+13*8)%sizeof(clip),32,first^255);
  for(unsigned i=0;i<32;i++)for(unsigned j=0;j<8;j++)if(j!=3&&j!=7)assert(next[i*8+j]==out[(i+13)*8+j]);
  assert(next[3]==(out[12*8+3]^255));
 }
 unsigned h=ram_packet_hash(clip,sizeof(clip));clip[100]^=1;assert(ram_packet_hash(clip,sizeof(clip))!=h);
 clip[3]=0;assert(!ram_packet_valid(clip,sizeof(clip)));assert(!ram_packet_valid(out,7));
 puts("PASS: payload identity, odd wrap, marker continuity, partial write, corrupt packet/hash rejection");
}
