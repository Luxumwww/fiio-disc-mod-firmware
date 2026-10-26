#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
static unsigned char memory[0x100000];
#define PC_WORD(a) (*(unsigned *)(memory+(unsigned)(a)-0x800000))
#define PC_BYTE(a) (memory[(unsigned)(a)-0x800000])
static void *test_function(unsigned);
#define PC_FN(a,t) ((t)test_function(a))
static int owned=1,card=1,enabled=1,fail_switch,original_calls,mixer_calls,mixer_fail,soft_volume;
static long mixer_value;static float digital_db=99;
static int test_access(const char *path,int mode){(void)mode;if(strstr(path,"/disabled")||strstr(path,"SAFE_MODE"))return -1;if(strstr(path,"/enabled"))return enabled?0:-1;if(strstr(path,"bt-usb.request"))return owned?0:-1;if(strstr(path,"card1"))return card?0:-1;return -1;}
static int test_lock(void *p){(void)p;return 0;}
static int test_clock(int mode,void *p){(void)mode;((long *)p)[0]=10;((long *)p)[1]=0;return 0;}
#define access test_access
#define pthread_mutex_lock test_lock
#define pthread_mutex_unlock test_lock
#define clock_gettime test_clock
#include "player_controls.h"
#include "display_transform.h"
static int native_original(void *p){(void)p;original_calls++;return 7;}
static void native_dsd(unsigned v){PC_WORD(0x810098)=v;}
static int native_switch(unsigned v){if(fail_switch)return -9;PC_WORD(0x820058)=v;return 0;}
static void native_void(void){}
static void native_unsigned(unsigned p){(void)p;}
static void native_float(float v){digital_db=v;}
static void native_volume(unsigned v){soft_volume=v;}
static int native_mixer(void *e,long v){(void)e;mixer_calls++;mixer_value=v;return mixer_fail?-1:0;}
static void *test_function(unsigned a){switch(a){case 0x4f1294:return native_original;case 0x4536d0:return native_dsd;case 0x455c58:return native_switch;case 0x4e8b94:return native_unsigned;case 0x44dc80:return native_float;case 0x4ea5d4:return native_volume;case 0x4e83b8:case 0x4e8888:case 0x4eae58:return native_void;default:fprintf(stderr,"unexpected native call %x\n",a);abort();}}
int main(void){
 PC_WORD(0x83dfc0)=0x830000;PC_WORD(0x830000)=2;PC_WORD(0x83dfc4)=0x810000;PC_WORD(0x83dfe4)=0x820000;PC_WORD(0x898d70)=0x840000;PC_WORD(0x840018)=PC_WORD(0x84001c)=1;PC_WORD(0x820058)=6;PC_WORD(0x810098)=2;PC_BYTE(0x83a759)=6;PC_BYTE(0x83a75a)=6;
 unsigned request[5]={0,0,0,0,3};card=0;assert(pc_output(request)==0&&PC_WORD(0x820058)==6&&PC_BYTE(0x83a75a)==3&&original_calls==0);
 card=1;assert(pc_output(request)==0&&PC_WORD(0x820058)==3&&original_calls==0);assert(PC_WORD(0x840018)==1&&PC_WORD(0x84001c)==1);
 pc_mixer_set=native_mixer;PC_WORD(0x85fde0)=1;PC_WORD(0x85fddc)=2;PC_WORD(0x85fdd8)=0;PC_WORD(0x839e70)=255;
 for(unsigned v=0;v<=120;v+=60){PC_BYTE(0x83a7a0)=v;pc_volume_tick();assert(mixer_value==(255*v/120)&&digital_db==0&&pc_last_volume==(int)v);}
 PC_WORD(0x85fdd8)=(unsigned)-64;PC_WORD(0x839e70)=0;PC_BYTE(0x83a7a0)=60;pc_last_volume=-1;pc_volume_tick();assert(mixer_value==-32);
 mixer_fail=1;PC_BYTE(0x83a7a0)=60;pc_last_volume=-1;pc_volume_tick();assert(soft_volume==50&&pc_last_volume==-1);
 PC_WORD(0x85fde0)=0;PC_BYTE(0x83a7a0)=120;pc_volume_tick();assert(soft_volume==100);
 unsigned old=PC_WORD(0x810098);fail_switch=1;request[4]=6;assert(pc_output(request)==-9&&PC_WORD(0x810098)==old&&PC_WORD(0x820058)==3);fail_switch=0;
 assert(pc_output(request)==0&&PC_WORD(0x820058)==6&&PC_BYTE(0x83a75a)==6&&original_calls==0);
 owned=0;assert(pc_output(request)==7&&original_calls==1);owned=1;PC_WORD(0x830000)=1;assert(pc_output(request)==7&&original_calls==2);
 const unsigned corner[]={0,359,129599,129240};for(unsigned r=0;r<4;r++){unsigned char seen[129600]={0};for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){unsigned p=disc_physical_index(x,y,r);assert(p<129600&&!seen[p]);seen[p]=1;}assert(disc_physical_index(0,0,r)==corner[r]);}
 puts("PASS: actual player callback retains BT; absent DAC, switch failure, disabled ownership/native fallback, mixer range/failure/software fallback; all four full-buffer rotations bijective");
}
