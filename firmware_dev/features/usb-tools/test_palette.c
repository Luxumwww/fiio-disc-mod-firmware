#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define RAM "/unused/"
static unsigned owner=1,token=1;
static int thermal_palette=-1,mode=2,selected,writes,reject;
static unsigned char device[512];
static unsigned wirelen=512,replylen=4;
static unsigned u16(const unsigned char *p){return p[0]|(unsigned)p[1]<<8;}
static unsigned u32(const unsigned char *p){return p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;}
static int control(unsigned type,unsigned request,unsigned value,unsigned index,void *data,unsigned len){
 assert(index==(10u<<8));unsigned selector=value>>8;
 if(selector==4&&type==0xa1){if(request==0x85){memset(data,0,len);return len;}memcpy(data,mode==2?"2.0":mode==1?"1.0":"9.0",4);return 4;}
 if(mode==1&&selector==2){unsigned char *p=data;if(type==0x21&&len==3){assert(p[0]==3&&p[1]==0&&p[2]==5);selected=1;return 3;}assert(selected&&len==27);if(type==0xa1){p[0]=3;p[1]=0;p[2]=5;memcpy(p+3,device,24);return 27;}assert(p[0]==3&&p[1]==0x88&&p[2]==5);writes++;memcpy(device,p+3,24);return 27;}
 if(selector==5){assert(type==0x21&&request==1&&len==2);unsigned char *p=data;assert(p[0]==2&&p[1]==5);selected=1;return 2;}
 assert(selector==2&&selected);
 if(type==0xa1&&request==0x85){unsigned char *p=data;p[0]=wirelen&255;p[1]=wirelen>>8;p[2]=p[3]=0;return replylen;}
 if(type==0xa1){assert(request==0x81&&len==wirelen);memcpy(data,device,len);return len;}
 assert(type==0x21&&request==1&&len==wirelen);writes++;if(reject){reject=0;return -1;}memcpy(device,data,len);return len;
}
static int text(const char *a,char *b,unsigned n){(void)a;(void)b;(void)n;return 0;}
static int atomic_text(const char *a,const char *b){(void)a;(void)b;return 1;}
static int unlink(const char *a){(void)a;return 0;}
static void state(const char *a,const char *b){(void)a;(void)b;}
#include "thermal_palette.h"
int main(void){
 for(unsigned i=0;i<512;i++)device[i]=(unsigned char)i;device[0]=1;device[5]=10;unsigned char before[512];memcpy(before,device,512);
 assert(palette_set(11));assert(thermal_palette==11&&writes==1);for(unsigned i=0;i<512;i++)assert(device[i]==(i==5?11:before[i]));
 assert(!palette_set(99));assert(writes==1);
 reject=1;assert(!palette_set(1));assert(device[5]==11&&thermal_palette==11);
 thermal_protocol=0;mode=1;assert(palette_set(2));assert(device[5]==2&&thermal_palette==2);for(unsigned i=0;i<512;i++)assert(device[i]==(i==5?2:before[i]));
 thermal_protocol=0;mode=9;int w=writes;assert(!palette_set(2));assert(writes==w);
 thermal_protocol=0;mode=2;wirelen=8;replylen=2;assert(palette_set(10));assert(thermal_palette==10);for(unsigned i=0;i<512;i++)assert(device[i]==before[i]);
 puts("PASS palette wire selector/offset, other parameters preserved, verified readback, rollback, unknown protocol rejected");return 0;
}
