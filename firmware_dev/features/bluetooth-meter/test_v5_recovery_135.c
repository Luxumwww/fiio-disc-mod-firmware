#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef PARSER_HEADER
#define PARSER_HEADER "lhdcv5_bitrate.h"
#endif
#include PARSER_HEADER
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void frame(unsigned char *p,unsigned kbps){unsigned bytes=2*(kbps*5/16),h=bytes/2|1024;p[0]=h;p[1]=h>>8;memset(p+2,0x55,bytes);}
static void packet(unsigned seq,unsigned ssrc,unsigned char *rtp){memset(rtp,0,12);rtp[0]=0x80;rtp[1]=0x60;rtp[2]=seq>>8;rtp[3]=seq;rtp[8]=ssrc>>24;rtp[9]=ssrc>>16;rtp[10]=ssrc>>8;rtp[11]=ssrc;}
int main(int argc,char **argv){
 unsigned char data[4096],rtp[12];unsigned size=626;
 frame(data,1000);frame(data+size,1000);frame(data+2*size,1000);
 v5_bitrate_reset();packet(30000,255,rtp);v5_bitrate_packet(rtp,data,2*size,100);CHECK(v5_bitrate_current(100)==1000);
 /* Same SSRC, restarted sequence keeps arriving. It must recover without a
  * song change, while genuine duplicate/reordered packets stay ignored. */
 packet(29999,255,rtp);v5_bitrate_packet(rtp,data,size,110);CHECK(v5_last_frame==100);
 for(unsigned i=0;i<330;i++){unsigned t=120+i*5;packet(100+i,255,rtp);v5_bitrate_packet(rtp,data,size,t);}
 CHECK(v5_bitrate_current(1765)==1000);
 /* Missing packet followed by a continuation fragment and complete frames. */
 v5_bitrate_reset();packet(10,1,rtp);v5_bitrate_packet(rtp,data,102,100);
 unsigned char tail[4096];memset(tail,0x55,524);memcpy(tail+524,data,2*size);
 packet(12,1,rtp);v5_bitrate_packet(rtp,tail,524+2*size,110);CHECK(v5_bitrate_current(110)==1000);
 /* Garbage prefix, unknown flags and unsupported budgets must not label. */
 v5_bitrate_reset();memset(tail,0x55,37);memcpy(tail+37,data,2*size);
 packet(1,2,rtp);v5_bitrate_packet(rtp,tail,37+2*size,100);CHECK(v5_bitrate_current(100)==1000);
 v5_bitrate_reset();memcpy(tail,data,2*size);tail[1]|=0x80;tail[size+1]|=0x80;
 packet(1,2,rtp);v5_bitrate_packet(rtp,tail,2*size,100);CHECK(!v5_bitrate_current(100));
 v5_bitrate_reset();packet(65535,3,rtp);v5_bitrate_packet(rtp,data,2*size,0xffffff00u);packet(0,3,rtp);v5_bitrate_packet(rtp,data,size,0x20);CHECK(v5_bitrate_current(0x20)==1000);
 packet(1,4,rtp);v5_bitrate_packet(rtp,data,size,0x30);CHECK(!v5_bitrate_current(0x30));packet(2,4,rtp);v5_bitrate_packet(rtp,data,size,0x40);CHECK(v5_bitrate_current(0x40)==1000);CHECK(!v5_bitrate_current(0x700));
 if(argc!=2)return 2;FILE *f=fopen(argv[1],"rb");CHECK(f);v5_bitrate_reset();packet(30000,255,rtp);v5_bitrate_packet(rtp,data,2*size,100);
 unsigned h[2],packets=0,last=0;
 while(fread(h,1,8,f)==8){CHECK(h[1]<=4096);CHECK(fread(rtp,1,12,f)==12);CHECK(fread(data,1,h[1],f)==h[1]);v5_bitrate_packet(rtp,data,h[1],h[0]+100);last=h[0]+100;packets++;}
 fclose(f);CHECK(packets>1000);CHECK(v5_bitrate_current(last)==1000);
 printf("PASS restart/continuous reverse sequence, reorder, mid-frame gap, byte resync, unsupported flags, wrap, SSRC, stale; actual fault %u packets recover=1000\n",packets);
 return 0;
}
