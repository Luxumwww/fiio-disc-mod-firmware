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
static int owned=1,card=1,enabled=1,fail_switch,original_calls,soft_calls,hardware_calls,soft_volume,echo_volume,power_ups,unmuted,switch_calls;
static float digital_db=99;static unsigned seconds=10;
static int test_access(const char *p,int m){(void)m;if(strstr(p,"/disabled")||strstr(p,"SAFE_MODE"))return -1;if(strstr(p,"/enabled"))return enabled?0:-1;if(strstr(p,"bt-usb.request"))return owned?0:-1;if(strstr(p,"card1"))return card?0:-1;return -1;}
static int test_lock(void *p){(void)p;return 0;}
static int test_clock(int mode,void *p){(void)mode;((long *)p)[0]=seconds;((long *)p)[1]=0;return 0;}
#define access test_access
#define pthread_mutex_lock test_lock
#define pthread_mutex_unlock test_lock
#define clock_gettime test_clock
#include "player_controls.h"
#include "display_transform.h"
static int original(void *p){(void)p;original_calls++;return 7;}
static void dsd(unsigned v){PC_WORD(0x810098)=v;}
static void mute(unsigned v){unmuted=!v;}
static void power(void){power_ups++;}
static void configure(unsigned r,unsigned b,unsigned d){assert(r==96000&&b==32&&!d);}
static void echo(unsigned id,unsigned v){assert(id==0x3f);echo_volume=v;}
static void gain(float v){digital_db=v;}
static void volume(unsigned v){soft_volume=v;soft_calls++;}
static void restore_volume(void){}
static int db(void *e,long v,int dir){(void)e;(void)dir;assert(v==0);hardware_calls++;return 0;}
static int native_switch(unsigned v){switch_calls++;if(fail_switch)return -9;unsigned old=PC_WORD(0x820058);if(old==v)return 0;PC_WORD(0x820000+old*0x54+0x68)=0;PC_WORD(0x820000+old*0x54+0x74)=0;PC_WORD(0x820058)=v;PC_WORD(0x820000+v*0x54+0x68)=1;PC_WORD(0x820000+v*0x54+0x74)=99;return 0;}
static void *test_function(unsigned a){switch(a){case 0x4f1294:return original;case 0x4536d0:return dsd;case 0x455c58:return native_switch;case 0x4e8b94:return mute;case 0x44dc80:return gain;case 0x4ea5d4:return volume;case 0x4e8888:return power;case 0x4eae58:return restore_volume;case 0x4e84f0:return configure;case 0x43db60:return echo;default:fprintf(stderr,"unexpected native call %x\n",a);abort();}}
int main(void){
 PC_WORD(0x83dfc0)=0x830000;PC_WORD(0x830000)=2;PC_WORD(0x83dfc4)=0x810000;PC_WORD(0x83dfe4)=0x820000;PC_WORD(0x898d70)=0x840000;PC_WORD(0x840018)=PC_WORD(0x84001c)=1;PC_WORD(0x820058)=6;PC_WORD(0x810098)=2;PC_WORD(0x810048)=1;PC_WORD(0x81001c)=96000;PC_WORD(0x810024)=32;PC_BYTE(0x83a759)=6;PC_BYTE(0x83a75a)=6;PC_BYTE(0x83a7a0)=31;PC_BYTE(0x83a789)=50;PC_WORD(0x820000+6*0x54+0x68)=1;PC_WORD(0x820000+6*0x54+0x74)=99;
 unsigned req[5]={0,0,0,0,3};card=0;assert(pc_output(req)==0&&pc_route()==6&&!pc_volume_active&&original_calls==0);
 card=1;PC_WORD(0x810048)=4;PC_WORD(0x820058)=3;assert(pc_output(req)==0&&pc_pcm_open(3)&&pc_volume_active&&pc_internal_volume==31&&echo_volume==60);assert(PC_WORD(0x840018)&&PC_WORD(0x84001c)&&power_ups==0&&!unmuted);
 pc_mixer_db=db;PC_WORD(0x85fde0)=1;PC_WORD(0x85fddc)=2;
 for(unsigned v=0;v<=120;v+=60){PC_BYTE(0x83a7a0)=v;pc_volume_tick();assert(soft_volume==(int)((v*100+60)/120));int n=soft_calls;pc_volume_tick();assert(soft_calls==n&&hardware_calls==1&&pc_internal_volume==31);}
 unsigned old=PC_WORD(0x810098);req[4]=6;fail_switch=1;assert(pc_output(req)==-9&&PC_WORD(0x810098)==old&&pc_volume_active&&pc_route()==3);fail_switch=0;
 assert(pc_output(req)==0&&pc_route()==6&&!pc_volume_active&&PC_BYTE(0x83a7a0)==31&&pc_usb_volume==120&&digital_db==0&&unmuted&&power_ups==0);
 PC_BYTE(0x83a7a0)=42;req[4]=3;assert(pc_output(req)==0&&pc_internal_volume==42&&PC_BYTE(0x83a7a0)==120);PC_BYTE(0x83a7a0)=24;pc_volume_tick();assert(soft_volume==20);req[4]=6;assert(pc_output(req)==0&&PC_BYTE(0x83a7a0)==42&&pc_usb_volume==24);
 for(int i=0;i<12;i++){req[4]=3;assert(pc_output(req)==0);req[4]=6;assert(pc_output(req)==0);}assert(power_ups==0&&PC_BYTE(0x83a7a0)==42&&pc_usb_volume==24);
 owned=0;assert(pc_output(req)==7&&original_calls==1);owned=1;PC_WORD(0x830000)=1;assert(pc_output(req)==7&&original_calls==2);
 const unsigned corner[]={0,129240,129599,359};for(unsigned r=0;r<4;r++){unsigned char seen[129600]={0};unsigned physical[129600];for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){unsigned i=disc_physical_index(x,y,r);assert(i<129600&&!seen[i]);seen[i]=1;physical[i]=y*360+x;}for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++)assert(physical[disc_physical_index(x,y,r)]==y*360+x);assert(disc_physical_index(0,0,r)==corner[r]);assert(((0x410c|(r<<4))&~0x30u)==0x410c);}
 puts("PASS: native hotplug route repaired before output compare; profiles isolated/restored across12 toggles; PCM retained/failed switch; unchanged software volume emits no repeat writes; one stable USB hardware setting; no repeated DAC power_up; V2.57 four-direction buffer/snapshot mapping");
}
