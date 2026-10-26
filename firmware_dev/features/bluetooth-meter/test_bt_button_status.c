#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
typedef struct {unsigned char bytes[128];} LvObj;
static LvObj image,button;static unsigned tick=1000,status=1,mode=3;static int active=1,playerpid=42,remote,connected=1,reportpid=42,invalid;
static unsigned binds,native_calls;static uintptr_t spec[12],events[15];
static LvObj *global(unsigned a){return a==0x8de3c8?&image:a==0x8de3d4?&button:0;}
#define LV_GLOBAL global
#define BT_BUTTON_WORD(a) (*((a)==0x8da030?&status:&mode))
static unsigned now(void){return tick;}
static int lv_valid(LvObj *o){return !invalid&&o;}
static unsigned lv_event_code(void *e){return *(unsigned *)e;}
static void src(LvObj *o,const void *path){*(const char **)(o->bytes+0x2c)=path;*(unsigned short *)(o->bytes+0x4e)=1;binds++;}
static void native_event(void *e){native_calls++;if(lv_event_code(e)==8)src(&image,status==0?"D:/usr/project/config/img/common/play.png":"D:/usr/project/config/img/common/pause.png");}
#define BT_BUTTON_NATIVE_EVENT native_event
static int textread(const char *p,char *b,unsigned z){(void)p;return snprintf(b,z,"version=1.3.32\npid=%d\nconnected=%d\nremote_state=%d\npending=0\nintent=0\nattempts=0\nresult=0\nrequests=1\nqueries=2\nevents=3\n",reportpid,connected,remote);}
#include "bt_button_status.h"
static const char *current(void){return *(const char **)(image.bytes+0x2c);}
int main(void){
 *(uintptr_t **)(button.bytes+8)=spec;spec[8]=5u<<16;spec[3]=(uintptr_t)events;
 events[0]=events[3]=events[6]=0x477b00;events[2]=1;events[5]=2;events[8]=8;events[9]=1234;events[11]=8;events[12]=0x477b00;events[14]=33;
 events[1]=17;bt_button_sync();assert(events[0]==(uintptr_t)bt_button_event&&events[3]==events[0]&&events[6]==events[0]&&events[9]==1234&&events[12]==0x477b00&&events[1]==17);
 /* Stock pause.png contains TRIANGLE; stock play.png contains TWO BARS. */
 assert(strstr(current(),"/pause.png")&&status==1);unsigned n=binds;
 for(int i=0;i<100;i++){tick+=200;bt_button_sync();}assert(binds==n);
 unsigned release=8;bt_button_event(&release);assert(native_calls==1&&strstr(current(),"/pause.png"));
 remote=1;tick+=200;bt_button_sync();assert(status==0&&strstr(current(),"/play.png"));
 bt_button_event(&release);assert(strstr(current(),"/play.png"));remote=0;tick+=200;bt_button_sync();assert(status==1&&strstr(current(),"/pause.png"));
 /* Correct a late native flip despite identical query/event counters. */
 src(&image,"D:/usr/project/config/img/common/play.png");tick+=200;bt_button_sync();assert(strstr(current(),"/pause.png"));
 reportpid=99;remote=1;tick+=200;bt_button_sync();assert(status==1);reportpid=42;
 connected=0;tick+=200;bt_button_sync();assert(bt_button_state==-1);connected=1;
 remote=-1;tick+=200;bt_button_sync();assert(bt_button_state==-1);
 active=0;status=1;bt_button_event(&release);assert(strstr(current(),"/pause.png"));
 puts("PASS: production UI uses verified stock TRIANGLE when paused / TWO BARS when playing; parsing, no prediction, unchanged image no rebind, wrong image corrected, lifecycle/unknown guards and original/XY callbacks preserved");return 0;
}
