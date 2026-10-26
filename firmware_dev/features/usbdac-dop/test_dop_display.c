#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
typedef void LvObj;
static unsigned char *memory;
#define WORD(a) (*(unsigned *)(memory+(a)-0x800000))
static LvObj *labels[2]={(void *)1,(void *)2};
#define LV_GLOBAL(a) (labels[(a)==0x8de210?0:1])
static unsigned player=0x810000,rate=352800,state=1,mode=4,dop=2,is_dsd=1,bits=32,channels=2,tag=0,ui_mode=1,disabled;
static unsigned h[40],texts,restores,last_restore,ring_counter;
static char names[2][32];
static int copy(unsigned a,void *b,unsigned z){memset(b,0,z);if(a==0x83dfc4)memcpy(b,&player,4);else if(a==player){h[3]=rate;h[4]=channels;h[5]=bits;h[6]=tag;h[11]=dop;h[22]=is_dsd;memcpy(b,h,z);}else if(a==player+0x130){unsigned ring[4]={0,0,ring_counter,0};memcpy(b,ring,z);}else if(a==0x839a74)memcpy(b,&state,4);else if(a==0x83dfc0){unsigned mp=0x830000;memcpy(b,&mp,4);}else if(a==0x830000)memcpy(b,&mode,4);else return 0;return 1;}
static int lv_valid(const LvObj *o){return o!=0;}
static void lv_text(LvObj *o,const char *s){assert(strlen(s)<32);strcpy(names[(uintptr_t)o-1],s);texts++;}
static void native_restore(unsigned v){last_restore=v;restores++;}
#define FN(a,t) ((t)native_restore)
static void discover(unsigned t){(void)t;}
static int access(const char *p,int m){(void)m;return strstr(p,"disabled")&&disabled?0:-1;}
static int textread(const char *p,char *b,unsigned z){(void)p;assert(z>16);strcpy(b,"usbdac-dop-abi3\n");return 16;}
static void *font22=(void *)22;
static unsigned layout_calls;
static void ensurefonts(void){}
static void smallfont(LvObj *o,void *f){assert(o==labels[1]&&f==font22);}
static void statictext(LvObj *o){assert(o==labels[1]);}
static void textalign(LvObj *o,unsigned a,unsigned v){assert(o==labels[1]&&a==2&&!v);}
static void lv_size(LvObj *o,int w,int h){assert(o==labels[1]&&w==240&&h==30);}
static void lv_align(LvObj *o,int a,int x,int y){assert(o==labels[1]&&a==2&&!x&&y==106);layout_calls++;}
#define DISC_DOP_DISPLAY_TEXT(o) names[(uintptr_t)(o)-1]
#include "dop_display.h"
int main(void){
 memory=mmap((void *)0x800000,0x100000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0);assert(memory==(void *)0x800000);
 WORD(0x8e16d4)=ui_mode;dop_display_poll(500);assert(!strcmp(names[0],"DSD128")&&!strcmp(names[1],"DoP / 5.64 MHz")&&layout_calls==1&&texts==2);dop_display_poll(1000);assert(texts==2);
 dop=1;is_dsd=0;dop_display_poll(1500);assert(restores==1&&last_restore==352&&!dop_display_owned);dop_display_poll(2000);assert(texts==2); /* Same-rate PCM must not show DSD. */
 dop=2;is_dsd=1;rate=176400;dop_display_poll(2500);assert(!strcmp(names[0],"DSD64")&&!strcmp(names[1],"DoP / 2.82 MHz"));disabled=1;dop_display_poll(3000);assert(restores==2&&last_restore==176);disabled=0;
 rate=352800;dop_display_poll(3500);state=0;dop_display_poll(4000);assert(restores==3&&last_restore==0);state=1;
 tag=1;dop_display_poll(4500);assert(!dop_display_owned);tag=0;dop_display_poll(5000);WORD(0x8e16d4)=3;dop_display_poll(5500);assert(!dop_display_owned&&!dop_display_protocol);
 WORD(0x8e16d4)=1;rate=192000;dop=0;is_dsd=0;strcpy(names[0],"未连接");unsigned previous=restores;
 dop_display_poll(6000);assert(restores==previous); /* A stale configured rate alone cannot reconnect. */
 ring_counter+=4096;dop_display_poll(6500);assert(restores==previous+1&&last_restore==192&&uac_display_repairs==1);
 dop_display_poll(8000);assert(restores==previous+1); /* A stopped ring cannot repair later text. */
 state=0;ring_counter+=4096;dop_display_poll(8500);assert(restores==previous+1); /* Disconnected rejects old ring progress. */
 puts("PASS: real classified DoP label only; DSD128/64 mapping; same-rate PCM never mislabeled; no repeated text updates; disabled/idle restored by stock handler; native DSD untouched; page lifecycle reset");
}
