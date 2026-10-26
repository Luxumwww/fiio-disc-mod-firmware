#include <stdio.h>
#include <string.h>
#include <assert.h>
#define RAM "/unused/"
#define DISC_THERMAL_MEASURE_TEST 1
static int thermal_protocol=2,thermal_measurement,selected,writes,reject;
static unsigned basic_length=64,mode_length=8;
static unsigned char basic_device[512],mode_device[512];
static unsigned u16(const unsigned char *p){return p[0]|(unsigned)p[1]<<8;}
static unsigned u32(const unsigned char *p){return u16(p)|(u16(p+2)<<16);}
static int atomic_text(const char *p,const char *s){(void)p;(void)s;return 1;}
static int control(unsigned type,unsigned request,unsigned value,unsigned index,void *data,unsigned n){
 assert(index==10u<<8);unsigned char *p=data;unsigned sel=value>>8;
 if(sel==5){assert(type==0x21&&n==2&&p[0]==3&&(p[1]==1||p[1]==2));selected=p[1];return 2;}
 assert(sel==3&&selected);unsigned length=selected==1?basic_length:mode_length;unsigned char *source=selected==1?basic_device:mode_device;
 if(type==0xa1&&request==0x85){assert(n==4);memset(p,0,n);p[0]=length&255;p[1]=length>>8;return 2;}
 assert(n==length);if(type==0xa1){memcpy(p,source,n);return (int)n;}
 writes++;if(reject==selected){reject=0;return -1;}memcpy(source,p,n);return (int)n;
}
#include "thermal_measurement.h"
static void initialize(void){for(unsigned i=0;i<512;i++){basic_device[i]=(unsigned char)i;mode_device[i]=(unsigned char)(255-i);}basic_device[0]=mode_device[0]=1;basic_device[1]=1;basic_device[2]=basic_device[3]=basic_device[44]=0;basic_device[4]=1;basic_device[30]=2;basic_device[31]=1;mode_device[1]=2;mode_device[2]=1;}
int main(void){
 unsigned char original[512],original_mode[512];initialize();memcpy(original,basic_device,512);memcpy(original_mode,mode_device,512);
 assert(measurement_apply()&&thermal_measurement==1&&writes==2);
 for(unsigned i=0;i<512;i++){int targeted=i==1||i==2||i==3||i==4||i==30||i==31||i==44;if(!targeted)assert(basic_device[i]==original[i]);if(i!=1&&i!=2)assert(mode_device[i]==original_mode[i]);}
 int before=writes;assert(measurement_apply()&&writes==before);
 initialize();basic_length=32;assert(measurement_apply()&&mode_device[1]==1&&mode_device[2]==0&&basic_device[2]==1&&basic_device[3]==1&&basic_device[4]==0);for(unsigned i=32;i<512;i++)assert(basic_device[i]==original[i]);basic_length=64;
 initialize();reject=1;assert(!measurement_apply()&&thermal_measurement==-1);assert(!memcmp(original,basic_device,512)&&!memcmp(original_mode,mode_device,512));
 initialize();basic_length=8;before=writes;assert(!measurement_apply()&&writes==before);basic_length=64;
 basic_device[44]=9;assert(!measurement_apply()&&writes==before);
 puts("PASS center/max/min tracked mode, all calibration/unknown bytes preserved, idempotent writes, partial-write rollback and short/unknown payload rejection");
}
