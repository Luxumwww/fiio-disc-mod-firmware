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
static void *native_function(unsigned);
#define PC_FN(a,t) ((t)native_function(a))
#define PLAYER_DBUS_POP_GOT 0x8f0000
static unsigned tick=1000,query_delay,discovery_delay;static int enabled=1,mode=2,original,remote=0,send_errors,auto_ack=1,plays,pauses,created,freed;
static char mac[20]="EC:21:50:2A:DA:A4";
static int test_lock(void *p){(void)p;return 0;}
static int test_clock(int c,void *p){(void)c;((long *)p)[0]=tick/1000;((long *)p)[1]=(tick%1000)*1000000;return 0;}
#define pthread_mutex_lock test_lock
#define pthread_mutex_trylock test_lock
#define pthread_mutex_unlock test_lock
#define clock_gettime test_clock
static int pc_enabled(void){return enabled;}
static int pc_workmode(void){return mode;}
static void (*pc_transport_tick)(void);
static int dlna_track_click(void *request){(void)request;return 0;}
#include "bt_playback.h"
typedef struct {int reply;const char *member;char status[20];int signal,connected;const char *path,*iface,*property;} Message;
static void err_init(void *e){memset(e,0,sizeof(BtDbusStorage));}
static void err_free(void *e){(void)e;}
static int err_set(const void *e){return *(const int *)e;}
static void *msg_new(const char *dest,const char *path,const char *iface,const char *member){assert(!strcmp(dest,"org.bluez"));assert(strstr(path,"dev_EC_21_50_2A_DA_A4"));assert(!strcmp(iface,"org.bluez.MediaControl1")||!strcmp(iface,"org.freedesktop.DBus.Properties"));Message *m=calloc(1,sizeof(*m));m->member=member;created++;return m;}
static void msg_unref(void *p){Message *m=p;memset(m->status,'X',sizeof(m->status));freed++;free(m);}
static void *msg_reply(void *conn,void *p,int timeout,void *error){(void)conn;assert(timeout==200);Message *m=p;
 if(!strcmp(m->member,"Get"))tick+=query_delay;
 if(strcmp(m->member,"Get")){if(!strcmp(m->member,"Play"))plays++;else {assert(!strcmp(m->member,"Pause"));pauses++;}if(send_errors){send_errors--;*(int *)error=1;return 0;}if(auto_ack)remote=!strcmp(m->member,"Play");}
 Message *r=calloc(1,sizeof(*r));r->reply=1;r->member=m->member;snprintf(r->status,sizeof(r->status),"%s",remote==1?"playing":remote==0?"paused":"unknown");created++;return r;
}
static void append_init(void *m,void *i){*(void **)i=m;}
static int append(void *i,int type,const void *value){(void)i;assert(type=='s'&&*(const char *const *)value);return 1;}
static int iter_init(void *m,void *i){*(void **)i=m;if(((Message *)m)->signal)((BtDbusStorage *)i)->words[21]=1;return 1;}
static int iter_type(void *i){BtDbusStorage *s=i;Message *m=*(void **)i;if(m->signal){switch(s->words[21]){case 1:case 4:return 's';case 2:return 'a';case 3:return 'e';case 5:return 'v';case 6:return !strcmp(m->property,"Connected")?'b':'s';default:return 0;}}return s->words[20]?'s':'v';}
static void recurse(void *i,void *v){*(void **)v=*(void **)i;Message *m=*(void **)i;if(m->signal)((BtDbusStorage *)v)->words[21]=((BtDbusStorage *)i)->words[21]+1;else ((BtDbusStorage *)v)->words[20]=1;}
static void basic(void *i,void *out){Message *m=*(void **)i;unsigned node=((BtDbusStorage *)i)->words[21];if(!m->signal||node==6){if(m->signal&&!strcmp(m->property,"Connected"))*(int *)out=m->connected;else *(char **)out=m->status;}else *(const char **)out=node==1?m->iface:m->property;}
static int iter_next(void *i){BtDbusStorage *s=i;if(s->words[21]==1||s->words[21]==4){s->words[21]++;return 1;}s->words[21]=0;return 0;}
static int is_signal(void *p,const char *iface,const char *member){return ((Message *)p)->signal&&!strcmp(iface,"org.freedesktop.DBus.Properties")&&!strcmp(member,"PropertiesChanged");}
static const char *msg_path(void *p){return ((Message *)p)->path;}
static void observe(const char *path,const char *iface,const char *property,const char *value,int connected){Message m={0};m.signal=1;m.path=path;m.iface=iface;m.property=property;m.connected=connected;snprintf(m.status,sizeof(m.status),"%s",value);bp_observe(&m);memset(m.status,'X',sizeof(m.status));}
static int scans;
static int native_device(char *p,unsigned n){scans++;tick+=discovery_delay;snprintf(p,n,"%s",mac);return mac[0]!=0;}
static int native_original(void *p){(void)p;original++;return 7;}
static int native_init(void){PC_WORD(0x83e170)=1;return 0;}
static void native_noop(void){}
static void native_unsigned(unsigned x){(void)x;}
static void *native_function(unsigned a){switch(a){case 0x40fb30:return native_device;case 0x4ed84c:return native_original;case 0x40b330:return native_init;case 0x4e8b94:return native_unsigned;case 0x4e8888:case 0x4eb284:return native_noop;default:abort();}}
static void reset(void){memset(memory,0,sizeof(memory));PC_WORD(0x83e170)=1;bp_job=(DiscBtPlay){0};bp_device[0]=bp_target[0]=bp_signal_mac[0]=0;bp_state=-1;bp_probe_attempts=0;bp_next_probe=bp_next_device=bp_signal_serial=bp_signal_seen=bp_events=0;bp_signal_state=bp_signal_connected=-1;query_delay=discovery_delay=0;scans=0;remote=0;auto_ack=1;send_errors=plays=pauses=0;tick+=3000;enabled=1;mode=2;snprintf(mac,sizeof(mac),"EC:21:50:2A:DA:A4");}
int main(void){
 bp_error_init=err_init;bp_error_free=err_free;bp_error_set=err_set;bp_unref=msg_unref;bp_message=msg_new;bp_reply=msg_reply;bp_iter_append_init=append_init;bp_append=append;bp_iter_init=iter_init;bp_iter_type=iter_type;bp_recurse=recurse;bp_basic=basic;
 bp_next=iter_next;bp_is_signal=is_signal;bp_message_path=msg_path;
 unsigned request[5]={0};reset();assert(bp_get_state(mac)==0);remote=1;assert(bp_get_state(mac)==1);remote=-1;assert(bp_get_state(mac)==-1);assert(created==freed);
 reset();assert(bp_click(request)==0);bp_poll();assert(plays==1&&pauses==0);tick+=450;bp_poll();assert(!bp_job.pending&&bp_job.result==1&&plays==1);
 assert(bp_click(request)==0);bp_poll();assert(pauses==1);tick+=450;bp_poll();assert(!bp_job.pending&&pauses==1&&remote==0);
 reset();auto_ack=0;send_errors=1;bp_click(request);bp_poll();assert(plays==1&&pauses==0);tick+=450;bp_poll();tick+=450;bp_poll();tick+=450;bp_poll();assert(plays==3&&pauses==0&&!bp_job.pending);
 reset();auto_ack=0;bp_click(request);bp_poll();remote=1;tick+=450;bp_poll();assert(plays==1&&!bp_job.pending);
 reset();bp_click(request);tick+=1800;bp_poll();assert(plays==0&&!bp_job.pending);
 reset();auto_ack=0;bp_click(request);bp_poll();observe("/org/bluez/hci0/dev_EC_21_50_2A_DA_A4","org.bluez.Device1","Connected","",0);mac[0]=0;tick+=450;bp_poll();assert(plays==1&&!bp_job.pending&&bp_job.result==-4);
 reset();auto_ack=0;bp_click(request);bp_poll();mode=1;tick+=450;bp_poll();assert(!bp_job.pending&&plays==1);assert(bp_click(request)==7);
 reset();enabled=0;assert(bp_click(request)==7);enabled=1;request[4]=1;assert(bp_click(request)==7);request[4]=0;
 reset();bp_click(request);bp_poll();assert(plays==1);bp_click(request);bp_poll();assert(pauses==1);tick+=450;bp_poll();assert(!bp_job.pending&&remote==0);
 const char *path="/org/bluez/hci0/dev_EC_21_50_2A_DA_A4/player0";
 reset();bp_poll();unsigned queries=bp_queries;int oldscans=scans;
 for(int i=0;i<50;i++){tick+=100;bp_poll();}assert(bp_queries==queries&&scans==oldscans);
 observe(path,"org.bluez.MediaPlayer1","Status","playing",-1);tick+=100;bp_poll();assert(bp_state==1&&bp_events==1&&bp_queries==queries);
 observe(path,"org.bluez.MediaPlayer1","Status","paused",-1);tick+=100;bp_poll();assert(bp_state==0&&bp_events==2);
 observe(path,"org.bluez.MediaPlayer1","Position","playing",-1);tick+=100;bp_poll();assert(bp_state==0&&bp_events==2);
 observe(path,"org.bluez.Device1","Status","playing",-1);tick+=100;bp_poll();assert(bp_state==0);
 observe("/org/bluez/hci0/dev_EC_21_50_2A_DA_A5/player0","org.bluez.MediaPlayer1","Status","playing",-1);tick+=100;bp_poll();assert(bp_state==0);
 reset();auto_ack=0;bp_click(request);bp_poll();queries=bp_queries;observe(path,"org.bluez.MediaPlayer1","Status","playing",-1);tick+=100;bp_poll();assert(!bp_job.pending&&bp_job.result==1&&plays==1&&bp_queries==queries);
 reset();bp_poll();observe("/org/bluez/hci0/dev_EC_21_50_2A_DA_A4","org.bluez.Device1","Connected","",0);observe(path,"org.bluez.MediaPlayer1","Status","paused",-1);mac[0]=0;tick+=100;bp_poll();assert(!bp_device[0]&&bp_state==-1);
 reset();queries=bp_queries;observe("/org/bluez/hci0/dev_EC_21_50_2A_DA_A4","org.bluez.Device1","Connected","",1);observe(path,"org.bluez.MediaPlayer1","Status","playing",-1);bp_poll();assert(bp_device[0]&&bp_state==1&&bp_queries==queries);
 reset();discovery_delay=1900;bp_click(request);bp_poll();assert(!bp_job.pending&&bp_job.result==-2&&plays==0&&pauses==0);
 reset();query_delay=1900;bp_click(request);bp_poll();assert(!bp_job.pending&&bp_job.result==-2&&plays==0&&pauses==0);
 assert(created==freed);puts("PASS: production query/signal state read before release, explicit retries, ACK/disconnect/mode cancellation, native fallback, idle no queries; burst Status retains Connected/disconnect and expired slow discovery/query cannot send; all messages released");
 return 0;
}
