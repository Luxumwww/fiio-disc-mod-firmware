#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#define RAM "/tmp/"
static int thermal_protocol=2;
static unsigned owner=1,token=1;
static unsigned char camera[512],before[512];
static unsigned n=28,group,writes,fail_readback;
static unsigned u16(const unsigned char*p){return p[0]|(unsigned)p[1]<<8;}
static unsigned u32(const unsigned char*p){return u16(p)|u16(p+2)<<16;}
static void put32(unsigned char*p,unsigned v){for(int i=0;i<4;i++)p[i]=v>>(8*i);}
static int atomic_text(const char*p,const char*d){(void)p;(void)d;return 1;}
static int text(const char*p,char*b,unsigned c){(void)p;(void)b;(void)c;return 0;}
static int control(unsigned t,unsigned r,unsigned v,unsigned index,void *data,unsigned len){
 assert(index==10<<8);
 if(t==0x21&&r==1&&v==5<<8){assert(len==2);group=((unsigned char*)data)[0];return 2;}
 assert(group==3&&v==3<<8);
 if(t==0xa1&&r==0x85){put32(data,n);return 4;}
 if(t==0xa1&&r==0x81){memcpy(data,camera,n);if(fail_readback&&writes==1)((unsigned char*)data)[0]=0;return n;}
 if(t==0x21&&r==1){assert(len==n);for(unsigned i=0;i<n;i++)if(i<16||i>=24)assert(((unsigned char*)data)[i]==before[i]);memcpy(camera,data,n);writes++;return n;}
 assert(0);return 0;
}
#include "thermal_center.h"
static void reset(void){for(unsigned i=0;i<28;i++)camera[i]=i*7;camera[0]=camera[1]=camera[2]=1;camera[3]=0;put32(camera+16,555);put32(camera+20,555);memcpy(before,camera,512);n=28;writes=fail_readback=0;}
int main(void){
 reset();assert(center_apply());assert(writes==1&&u32(camera+16)==500&&u32(camera+20)==500);assert(center_apply()&&writes==1);
 reset();camera[3]=1;assert(!center_apply()&&!writes);
 reset();n=24;assert(!center_apply()&&!writes);
 reset();put32(camera+16,1001);assert(!center_apply()&&!writes);
 reset();fail_readback=1;assert(!center_apply()&&writes==2&&!memcmp(camera,before,28));
 puts("center position: preservation, idempotence, validation and rollback PASS");
}
