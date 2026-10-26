#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bluetooth-meter/lhdcv5_bitrate.h"
static void frame(unsigned char *p,unsigned kbps){unsigned bytes=2*(kbps*5/16),h=bytes/2|1024;p[0]=h;p[1]=h>>8;memset(p+2,0x55,bytes);}
static void packet(unsigned seq,unsigned char *rtp){memset(rtp,0,12);rtp[0]=0x80;rtp[1]=0x60;rtp[2]=seq>>8;rtp[3]=seq;rtp[11]=1;}
int main(int argc,char **argv){
 unsigned char data[3000],rtp[12];unsigned modes[]={256,320,400,500,600,700,800,900,1000};
 for(unsigned i=0;i<sizeof(modes)/sizeof(modes[0]);i++){
  unsigned mode=modes[i],n=2+2*(mode*5/16);v5_bitrate_reset();frame(data,mode);frame(data+n,mode);
  packet(65535,rtp);v5_bitrate_packet(rtp,data,1,100);
  packet(0,rtp);v5_bitrate_packet(rtp,data+1,n*2-1,110);
  if(v5_bitrate_current(110)!=mode||v5_bitrate_current(1700))return 10;
  packet(0,rtp);v5_bitrate_packet(rtp,data,n*2,120);if(v5_last_frame!=110)return 11;
  packet(2,rtp);v5_bitrate_packet(rtp,data,1,130);if(v5_bitrate_current(130))return 12;
 }
 v5_bitrate_reset();memset(data,0,2);packet(1,rtp);v5_bitrate_packet(rtp,data,2,100);if(v5_bitrate_current(100))return 13;
 if(argc!=2)return 2;FILE *f=fopen(argv[1],"rb");if(!f)return 3;unsigned char p[8192],h[8];unsigned packets=0,matches=0;
 if(fread(h,1,8,f)!=8||memcmp(h,"DLHDC001",8))return 4;
 v5_bitrate_reset();
 while(fread(h,1,4,f)==4){
  unsigned n=(unsigned)h[0]|(unsigned)h[1]<<8|(unsigned)h[2]<<16|(unsigned)h[3]<<24;
  if(n>sizeof(p)||fread(p,1,n,f)!=n||p[0]!=0x80||p[1]!=0x60||n<14)return 5;
  v5_bitrate_packet(p,p+14,n-14,100+packets*10);packets++;
  if(v5_bitrate_current(100+(packets-1)*10))matches++;
 }
 fclose(f);printf("observed matches=%u packets=%u nominal=%u invalid=%u\n",matches,packets,v5_nominal_kbps,v5_invalid_headers);if(packets!=512||matches<500)return 6;
 printf("PASS nominal V5: known budgets, split header/frame, wrap, duplicate, gap, stale/invalid; %u/%u real packets recognized, last=%u kbps\n",matches,packets,v5_nominal_kbps);return 0;
}
