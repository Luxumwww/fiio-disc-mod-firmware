#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
static unsigned context[8],queries;static int enabled=1,query_result;
static const char *selected;
static int original(void *,const char *);
static int pc_enabled(void){return enabled;}
static int cacheflush(void *p,int n,int which){(void)p;(void)n;(void)which;return 0;}
#define PC_WORD(a) (*(unsigned *)((char *)context+(uint32_t)((uint32_t)(a)-(uint32_t)(uintptr_t)context)))
#define PC_FN(a,t) ((t)original)
#include "../bluetooth-meter/lhdcv5_player.h"
static int original(void *ctx,const char *path){
 (void)path;unsigned *c=ctx;queries++;
 if(query_result)return query_result;
 c[4]=!strcmp(selected,"AAC")?1:!strcmp(selected,"SBC")?2:!strcmp(selected,"aptX")?3:!strcmp(selected,"aptX-HD")?5:!lhdcv5_codec_compare(selected,"LDAC")?9:0;
 return 0;
}
static int check(const char *name,unsigned rate,unsigned bits,unsigned ch,int on,int result,unsigned expected){
 memset(context,0,sizeof(context));context[1]=rate;context[2]=bits;context[3]=ch;
 selected=name;enabled=on;query_result=result;unsigned before=queries;
 int r=lhdcv5_sink_param(context,"/org/bluealsa/hci0/dev_00_11_22_33_44_55/a2dpsnk/source");
 return r==result&&queries==before+1&&context[4]==expected;
}
int main(void){
 unsigned rates[]={48000,96000,192000};
 for(unsigned i=0;i<3;i++)if(!check("LHDC-V5",rates[i],32,2,1,0,8))return 1;
 const char *names[]={"AAC","SBC","aptX","aptX-HD","LDAC","future-codec"};unsigned ids[]={1,2,3,5,9,0};
 for(unsigned i=0;i<6;i++)if(!check(names[i],48000,32,2,1,0,ids[i]))return 2;
 if(!check("LHDC-V5",48000,32,2,0,0,0)||!check("LHDC-V5",44100,32,2,1,0,0)||!check("LHDC-V5",48000,16,2,1,0,0)||!check("LHDC-V5",48000,32,1,1,0,0)||!check("LHDC-V5",48000,32,2,1,-1,0))return 3;
 puts("PASS single native query: V5 48/96/192k, original AAC/SBC/aptX/HD/LDAC IDs, unknown/disabled/error/invalid-format rejection");return 0;
}
