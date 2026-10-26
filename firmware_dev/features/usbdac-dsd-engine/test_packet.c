#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "packet.h"
static unsigned char source[0x12c000],out[0x12c000];
int main(void){
 for(unsigned i=0;i<sizeof source;i+=8){source[i+1]=(i>>4)|1;source[i+2]=(i>>7)|3;source[i+5]=(i>>6)|7;source[i+6]=(i>>8)|15;source[i+3]=source[i+7]=(i&8)?250:5;}
 assert(dsde_packet_valid(source,sizeof source));DiscDsdPacket state;dsde_packet_reset(&state);
 for(unsigned offset=0;offset<sizeof source;){unsigned n=sizeof(source)-offset;if(n>65528)n=65528;dsde_packet_copy(&state,source+offset,n,out+offset,0);offset+=n;}
 assert(!memcmp(source,out,sizeof source));
 dsde_packet_copy(&state,0,8,out,1);assert(out[1]==0x69&&out[2]==0x69&&out[3]==5&&out[7]==5);
 dsde_packet_copy(&state,source,16,out,0);assert(out[3]==250&&out[7]==250&&out[11]==5&&out[15]==5);
 assert(out[1]==source[1]&&out[2]==source[2]&&out[5]==source[5]&&out[6]==source[6]);
 unsigned char snapshot[sizeof state];memcpy(snapshot,&state,sizeof state);source[15]=5;
 assert(!dsde_packet_valid(source,sizeof source)&&!memcmp(snapshot,&state,sizeof state));
 assert(!dsde_packet_valid(source,0)&&!dsde_packet_valid(source,7));source[15]=250;source[0]=1;assert(!dsde_packet_valid(source,sizeof source));
 puts("PASS: entire1.2MB retained, arbitrary odd-frame chunks, stereo payload/order unchanged, shared output markers span silence/song seam, malformed padding/markers rejected before state mutation");
}
