#include <stdio.h>
#include <string.h>
extern int close(int);
#include "../bluetooth-meter/realtime.h"
int main(int argc,char **argv){
 if(argc!=2)return 2;FILE *f=fopen(argv[1],"rb");if(!f)return 3;
 unsigned char p[8192],h[8];unsigned total=0,count=0;
 if(fread(h,1,8,f)!=8||memcmp(h,"DLHDC001",8))return 4;
 while(fread(h,1,4,f)==4){
  unsigned n=h[0]|h[1]<<8|h[2]<<16|h[3]<<24;
  if(n>sizeof(p)||fread(p,1,n,f)!=n||p[0]!=0x80||p[1]!=0x60)return 5;
  unsigned b=media_bytes(p,n,"LHDC V5");if(b!=n-14)return 6;
  total+=b;count++;
 }
 fclose(f);if(count!=512||!total)return 7;
 unsigned char ext[28]={0x90,0x60};ext[15]=1;
 if(media_bytes(ext,28,"LHDC V5")!=6)return 8;
 ext[0]=0xb0;ext[27]=2;if(media_bytes(ext,28,"LHDC V5")!=4)return 9;
 ext[27]=255;if(media_bytes(ext,28,"LHDC V5"))return 10;
 if(media_bytes(ext,12,"LHDC V5")||realtime_supported("LHDC")||!realtime_supported("AAC")||!realtime_supported("LDAC")||!realtime_supported("SBC"))return 11;
 printf("PASS V5 bitrate: %u encoded bytes, %u real RTP packets; two-byte media header, extension, padding, truncation and existing codec support\n",total,count);
 return 0;
}
