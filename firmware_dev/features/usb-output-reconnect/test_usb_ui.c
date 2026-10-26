/* Executes the complete production UI state machine with a fake kernel,
 * runtime handoff, native worker, and player readback. Never touches devices. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct {int unused;} LvObj;
static int pure_editing,active=1,playerpid=5678,usb_icon_1,usb_icon_0;
static unsigned fake_now=10000,config_output=6,player_route=6,player_state=1,pcm_ready,pcm_handle;
static unsigned native_due,native_pending,native_source_calls;
static int native_auto=1,request_exists,ep0_exists,android_exists,udc_exists,card_present,typec_state=2;
static char runtime_state[1024],player_state_text[1024],role_text[24]="device",udc_text[128],debug_request[96];
static int ui_source_calls,ui_sink_calls,role_host_writes,role_device_writes,command_calls,ack_writes;
static int role_write_fail,source_fail,sink_fail,copy_fail;
static unsigned last_command;
static char events[4096];
static void event(const char *s){strncat(events,s,sizeof(events)-strlen(events)-1);}
static int control_clicked(void *e){(void)e;return 1;}
static unsigned now(void){return fake_now;}
static void discover(unsigned t){(void)t;}
static void lv_text(LvObj *o,const char *s){(void)o;(void)s;}
static void src(LvObj *o,const void *s){(void)o;(void)s;}
static int textread(const char *path,char *b,unsigned cap){
 const char *s=0;char temp[96];
 if(!strcmp(path,"/tmp/disc-runtime/state"))s=runtime_state;
 else if(!strcmp(path,"/tmp/disc-runtime/player-usb.state"))s=player_state_text;
 else if(!strcmp(path,"/proc/asound/cards"))s=card_present?" 0 internal\n 1 USB-Audio - DAC\n":" 0 internal\n";
 else if(strstr(path,"role-switch/role"))s=role_text;
 else if(strstr(path,"typec_state")){snprintf(temp,sizeof(temp),"%d state\n",typec_state);s=temp;}
 else if(!strcmp(path,"/tmp/disc-runtime/debug-usb-priority")&&debug_request[0])s=debug_request;
 if(!s||!s[0]){if(cap)b[0]=0;return 0;}unsigned n=(unsigned)strlen(s);if(n>=cap)n=cap-1;memcpy(b,s,n);b[n]=0;return (int)n;
}
static int copy(unsigned address,void *data,unsigned size){
 if(copy_fail||size!=4)return 0;unsigned value;
 if(address==0x83a758)value=config_output<<16;
 else if(address==0x83dfe4)value=0x10000000;
 else if(address==0x10000058)value=player_route;
 else if(address==0x10000164)value=pcm_ready;
 else if(address==0x10000170)value=pcm_handle;
 else if(address==0x83dfc4)value=0x10010000;
 else if(address==0x10010048)value=player_state;
 else return 0;memcpy(data,&value,4);return 1;
}
static void native_command(const char *key,unsigned output,const char *extra){
 (void)key;(void)extra;command_calls++;last_command=output;event(output==3?"command3;":"commandInternal;");
 if(native_auto){native_pending=output;native_due=fake_now+330;}
}
static uintptr_t test_fn(unsigned a){return a==0x48b28c?(uintptr_t)native_command:0;}
static int test_open(const char *path,int mode,...){
 (void)mode;if(strstr(path,"role-switch/role"))return 11;if(!strcmp(path,"/dev/aw35615"))return 12;
 if(strstr(path,"disc_adb/UDC"))return udc_exists?13:-1;
 if(strstr(path,"debug-usb-priority.ack.new"))return 15;
 if(strstr(path,"bt-usb.request.new"))return 14;return -1;
}
static int test_close(int fd){(void)fd;return 0;}
static int test_read(int fd,void *b,unsigned cap){if(fd!=13)return -1;unsigned n=(unsigned)strlen(udc_text);if(n>cap)n=cap;memcpy(b,udc_text,n);return (int)n;}
static int test_write(int fd,const void *b,unsigned n){
 if(fd==11){if(role_write_fail)return -1;unsigned k=n<sizeof(role_text)-1?n:sizeof(role_text)-1;memcpy(role_text,b,k);role_text[k]=0;if(!strcmp(role_text,"host")){role_host_writes++;event("roleHost;");}else {role_device_writes++;event("roleDevice;");}return (int)n;}
 if(fd==14)return (int)n;if(fd==15){ack_writes++;event("ack;");return (int)n;}return -1;
}
static int test_access(const char *path,int mode){(void)mode;if(strstr(path,"/usb-ffs/adb/ep0"))return ep0_exists?0:-1;if(strstr(path,"/dev/android_adb"))return android_exists?0:-1;if(strstr(path,"bt-usb.request"))return request_exists?0:-1;return -1;}
static int test_unlink(const char *path){if(strstr(path,"debug-usb-priority")&&!strstr(path,".ack"))debug_request[0]=0;if(strstr(path,"bt-usb.request")){request_exists=0;event("releaseRequest;");}return 0;}
static int test_rename(const char *from,const char *to){(void)from;if(strstr(to,"bt-usb.request"))request_exists=1;return 0;}
static int test_getpid(void){return 1234;}
static int test_ioctl(int fd,unsigned command,...){
 if(fd!=12)return -1;if(command==0x20004e27u){ui_source_calls++;event("uiSource;");return source_fail?-1:0;}
 if(command==0x20004e26u){ui_sink_calls++;event("uiSink;");return sink_fail?-1:0;}return -1;
}
#define FN(a,t) ((t)test_fn(a))
#define open test_open
#define close test_close
#define read test_read
#define write test_write
#define access test_access
#define unlink test_unlink
#define rename test_rename
#define getpid test_getpid
#define ioctl test_ioctl
#include "../bluetooth-meter/usb_output.h"
#undef open
#undef close
#undef read
#undef write
#undef access
#undef unlink
#undef rename
#undef getpid
#undef ioctl

#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s (phase=%d text=%s events=%s)\n",__LINE__,#x,usb_phase,usb_text,events);return 1;}}while(0)
static void runtime(unsigned ui_mode,unsigned updated,const char *transition){
 snprintf(runtime_state,sizeof(runtime_state),"mode=off\nusb_active=0\nwireless_active=0\nrequested_usb=1\nrequested_wireless=0\npause_reason=%s\nrescue=0\nwork_mode=%u\nupdated=%u\ntransition=%s\nusb_priority=\nusb_error=\n",ui_mode==3?"bt_usb_audio":"pure_usb_audio",ui_mode,updated,transition);
}
static void player(unsigned updated,int ready,int present,int recovering,int error,unsigned pid){
 snprintf(player_state_text,sizeof(player_state_text),"pid=%u\nupdated=%u\nusb_ready=%d\nusb_present=%d\nusb_recovering=%d\nusb_error=%d\nplayer_state=%u\n",pid,updated,ready,present,recovering,error,player_state);
}
static void step(unsigned t){
 fake_now=t;if(native_due&&t>=native_due){config_output=native_pending;if(native_pending==3){native_source_calls++;event("nativeSource;");}else player_route=native_pending;native_due=0;}
 usb_poll(t);
}
static void fresh_step(unsigned t){runtime(*(unsigned *)0x8e16d4,t/1000,"");step(t);}
static void reset(unsigned ui_mode){
 fake_now=10000;*(unsigned *)0x8e16d4=ui_mode;*(unsigned char *)0x8e1728=6;
 config_output=player_route=6;player_state=1;pcm_ready=pcm_handle=0;native_due=native_pending=native_source_calls=0;native_auto=1;
 request_exists=ep0_exists=android_exists=udc_exists=card_present=0;typec_state=2;
 runtime_state[0]=player_state_text[0]=udc_text[0]=debug_request[0]=events[0]=0;strcpy(role_text,"device");
 ui_source_calls=ui_sink_calls=role_host_writes=role_device_writes=command_calls=ack_writes=role_write_fail=source_fail=sink_fail=copy_fail=0;
 usb_phase=0;usb_previous=6;usb_actual=usb_route=-1;usb_at=usb_checked=usb_retry_at=usb_retries=usb_requested_at=usb_host_at=usb_host_checked=usb_source_retries=0;
 usb_owner_mode=99;usb_was_present=usb_host_ready=usb_attach_seen=usb_sink_done=0;usb_text[0]=0;usb_debug_checked=usb_debug_attempts=0;usb_debug_done[0]=usb_debug_pending[0]=0;active=1;pure_editing=0;
}
static int enter_waiting(unsigned mode){
 reset(mode);usb_click((void *)1);if(usb_phase!=1||!request_exists)return 0;
 fresh_step(11000);if(usb_phase!=2||command_calls!=1)return 0;
 fresh_step(11500);if(usb_phase!=2||native_source_calls!=1||ui_source_calls)return 0;
 fresh_step(12000);return usb_phase==3&&role_host_writes==1&&ui_source_calls==0;
}
int main(void){
 if(mmap((void *)0x8e0000,0x20000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)!=(void *)0x8e0000)return 2;
 /* Integration side effects must remain zero for every incomplete handoff. */
 reset(0);usb_click((void *)1);runtime(0,11,"debug");step(11000);CHECK(usb_phase==1&&!command_calls&&!role_host_writes&&!ui_source_calls);
 runtime(0,10,"");step(11500);CHECK(usb_phase==1&&!command_calls);
 runtime(3,12,"");step(12000);CHECK(usb_phase==1&&!command_calls);
 runtime(0,12,"");ep0_exists=1;step(12500);CHECK(usb_phase==1&&!command_calls);ep0_exists=0;android_exists=1;step(13000);CHECK(usb_phase==1&&!command_calls);
 android_exists=0;udc_exists=1;strcpy(udc_text,"13500000.otg_new\n");runtime(0,13,"");step(13500);CHECK(usb_phase==1&&!command_calls);udc_text[0]=0;step(14000);CHECK(usb_phase==2&&command_calls==1);
 /* Both modes use the native first source request; no tail never causes
  * repeated AW ioctls, and AttachWaitSource is never reset. */
 for(unsigned m=0;m<2;m++){
  CHECK(enter_waiting(m?3:0));for(unsigned t=13000;t<=24000;t+=1000)fresh_step(t);CHECK(ui_source_calls==0&&native_source_calls==1&&role_host_writes==1);
  typec_state=5;for(unsigned t=25000;t<=34000;t+=1000)fresh_step(t);CHECK(ui_source_calls==0&&usb_attach_seen);
  typec_state=2;fresh_step(35000);CHECK(ui_source_calls==1);
  typec_state=5;fresh_step(38000);typec_state=2;fresh_step(41000);CHECK(ui_source_calls==2);
  typec_state=5;fresh_step(44000);typec_state=2;fresh_step(47000);CHECK(ui_source_calls==2);
 }
 /* Fresh same-process player health, rather than stale nonzero PCM memory,
  * decides success. Paused/stopped connected hardware needs no open PCM. */
 CHECK(enter_waiting(0));card_present=1;player_route=3;pcm_ready=1;pcm_handle=0xdead;
 player(12,0,1,1,0,5678);fresh_step(13000);CHECK(strcmp(usb_text,"USB 输出 开")&&strstr(usb_text,"连接"));
 player(13,1,1,0,0,9999);fresh_step(14000);CHECK(strcmp(usb_text,"USB 输出 开"));
 player(9,1,1,0,0,5678);fresh_step(15000);CHECK(strcmp(usb_text,"USB 输出 开"));
 player(16,1,1,0,0,5678);fresh_step(16000);CHECK(!strcmp(usb_text,"USB 输出 开"));
 player_state=3;pcm_ready=pcm_handle=0;player(17,0,1,0,0,5678);fresh_step(17000);CHECK(!strcmp(usb_text,"USB 已连接"));
 /* A real disappearance starts a new bounded source/reopen budget. */
 usb_source_retries=2;usb_retries=3;card_present=0;fresh_step(18000);CHECK(usb_source_retries==0&&usb_retries==0&&!usb_was_present);
 card_present=1;player(19,0,1,1,0,5678);fresh_step(19000);CHECK(usb_retries==0&&usb_was_present);
 /* Explicit debug ON must not sink/ack until native PCM route has returned. */
 CHECK(enter_waiting(3));card_present=1;player_route=3;config_output=3;native_auto=0;fake_now=13000;strcpy(debug_request,"1234 3 13\n");usb_debug_poll(fake_now);CHECK(usb_phase==4&&last_command==6&&!ui_sink_calls&&!ack_writes&&request_exists);
 fake_now=13500;usb_debug_poll(fake_now);CHECK(!ui_sink_calls&&!ack_writes&&request_exists);
 config_output=player_route=6;fake_now=14000;usb_debug_poll(fake_now);CHECK(usb_phase==0&&ui_sink_calls==1&&!request_exists);CHECK(strstr(events,"commandInternal;")<strstr(events,"uiSink;"));
 fake_now=14500;usb_debug_poll(fake_now);CHECK(ack_writes==1&&ui_source_calls==0&&ui_sink_calls==1);
 puts("PASS: complete production USB UI phases, causal handoff/endpoint+UDC gates, native-first power, idle/AttachWaitSource no-reset, bounded source fallback, plug generation budgets, stale PCM/readback rejection, paused connection, debug close ordering");return 0;
}
