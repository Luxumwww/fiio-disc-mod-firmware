/* PLAYER only. Existing 100ms worker owns all BlueZ requests; no UI blocking. */
#include "bt_play_policy.h"
typedef union {unsigned words[32];void *alignment;} BtDbusStorage;
static void (*bp_error_init)(void *),(*bp_error_free)(void *),(*bp_unref)(void *);
static int (*bp_error_set)(const void *),(*bp_iter_init)(void *,void *),(*bp_iter_type)(void *);
static void (*bp_iter_append_init)(void *,void *),(*bp_recurse)(void *,void *),(*bp_basic)(void *,void *);
static int (*bp_append)(void *,int,const void *);
static int (*bp_next)(void *),(*bp_is_signal)(void *,const char *,const char *);
static const char *(*bp_message_path)(void *);static void *(*bp_real_pop)(void *);
static void *(*bp_message)(const char *,const char *,const char *,const char *),*(*bp_reply)(void *,void *,int,void *);
static unsigned bp_mutex[10],bp_last_report,bp_requests,bp_queries,bp_next_probe;
static int bp_installed,bp_state=-1,bp_probe_attempts;
static char bp_device[20],bp_target[20];static DiscBtPlay bp_job;
static unsigned bp_signal_mutex[10],bp_signal_serial,bp_signal_seen,bp_events,bp_next_device;
static char bp_signal_mac[20];static int bp_signal_state=-1,bp_signal_connected=-1;
static unsigned bp_ms(void){long t[2];return clock_gettime(1,t)?0:(unsigned)t[0]*1000u+(unsigned)t[1]/1000000u;}
/* Inspect the already subscribed native monitor message before its unref.
 * Keep its ownership/dispatch unchanged; no extra bus subscription/thread. */
static void bp_observe(void *msg){
 if(!msg||!bp_is_signal(msg,"org.freedesktop.DBus.Properties","PropertiesChanged"))return;
 const char *path=bp_message_path(msg),*prefix="/org/bluez/hci0/dev_";unsigned i=0;
 if(!path)return;while(prefix[i]&&path[i]==prefix[i])i++;if(prefix[i])return;
 char mac[20];for(unsigned j=0;j<17;j++){char c=path[i+j];if(!c)return;if(j==2||j==5||j==8||j==11||j==14){if(c!='_')return;mac[j]=':';}else{if(!((c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f')))return;mac[j]=c;}}mac[17]=0;
 const char *suffix=path+i+17;
 BtDbusStorage root={0},array={0},entry={0},variant={0};const char *iface=0;
 if(!bp_iter_init(msg,&root)||bp_iter_type(&root)!='s')return;bp_basic(&root,&iface);if(!iface)return;
 int player=!strcmp(iface,"org.bluez.MediaPlayer1"),device=!strcmp(iface,"org.bluez.Device1");
 if((player&&strcmp(suffix,"/player0"))||(device&&suffix[0])||(!player&&!device))return;
 if(!bp_next(&root)||bp_iter_type(&root)!='a')return;bp_recurse(&root,&array);
 int state=-1,connected=-1;unsigned bound=0;
 while(bp_iter_type(&array)=='e'&&bound++<32){
  const char *key=0;bp_recurse(&array,&entry);if(bp_iter_type(&entry)=='s')bp_basic(&entry,&key);
  if(key&&bp_next(&entry)&&bp_iter_type(&entry)=='v'){bp_recurse(&entry,&variant);
   if(player&&!strcmp(key,"Status")&&bp_iter_type(&variant)=='s'){const char *s=0;bp_basic(&variant,&s);if(s)state=bt_play_state(s);}
   if(device&&!strcmp(key,"Connected")&&bp_iter_type(&variant)=='b')bp_basic(&variant,&connected);
  }
  if(!bp_next(&array))break;
 }
 if(state<0&&connected<0)return;
 pthread_mutex_lock(bp_signal_mutex);
 /* Different properties from one device may arrive in a single worker tick.
  * Coalesce pending fields so Status cannot erase its disconnect event. */
 if(strcmp(bp_signal_mac,mac)){bp_signal_state=-1;bp_signal_connected=-1;}
 snprintf(bp_signal_mac,sizeof(bp_signal_mac),"%s",mac);if(state>=0)bp_signal_state=state;if(connected>=0)bp_signal_connected=connected;bp_signal_serial++;pthread_mutex_unlock(bp_signal_mutex);
}
__attribute__((noinline,visibility("hidden"))) static void *bp_pop(void *conn){void *msg=bp_real_pop(conn);if(bp_installed&&conn==(void *)(unsigned long)PC_WORD(0x83e174))bp_observe(msg);return msg;}
static void bp_apply_signal(void){
 pthread_mutex_lock(bp_signal_mutex);
 if(bp_signal_seen!=bp_signal_serial){bp_signal_seen=bp_signal_serial;
  if(bp_signal_connected==1){snprintf(bp_device,sizeof(bp_device),"%s",bp_signal_mac);bp_state=-1;bp_probe_attempts=0;bp_next_probe=0;}
  if(bp_signal_connected==0&&!strcmp(bp_device,bp_signal_mac)){bp_device[0]=0;bp_state=-1;}
  if(bp_signal_state>=0&&!strcmp(bp_device,bp_signal_mac)){bp_state=bp_signal_state;bp_events++;}
  bp_signal_state=bp_signal_connected=-1;
 }
 pthread_mutex_unlock(bp_signal_mutex);
}
static void bp_path(char *out,unsigned n,const char *mac,int player){char part[20];unsigned i;for(i=0;i<17&&mac[i];i++)part[i]=mac[i]==':'?'_':mac[i];part[i]=0;snprintf(out,n,"/org/bluez/hci0/dev_%s%s",part,player?"/player0":"");}
static int bp_get_state(const char *mac){
 void *conn=(void *)(unsigned long)PC_WORD(0x83e170);if(!conn)return -1;
 char path[100];bp_path(path,sizeof(path),mac,1);void *msg=bp_message("org.bluez",path,"org.freedesktop.DBus.Properties","Get");if(!msg)return -1;
 BtDbusStorage error={0},append={0},iter={0},variant={0};const char *iface="org.bluez.MediaPlayer1",*property="Status";bp_error_init(&error);bp_iter_append_init(msg,&append);
 if(!bp_append(&append,'s',&iface)||!bp_append(&append,'s',&property)){bp_unref(msg);bp_error_free(&error);return -1;}
 void *reply=bp_reply(conn,msg,200,&error);bp_unref(msg);int state=-1;
 if(reply&&!bp_error_set(&error)&&bp_iter_init(reply,&iter)&&bp_iter_type(&iter)=='v'){
  bp_recurse(&iter,&variant);if(bp_iter_type(&variant)=='s'){const char *s=0;bp_basic(&variant,&s);if(s)state=bt_play_state(s);}
 }
 /* BlueZ owns s: interpret/copy BEFORE unref. Stock 40db2c did the reverse. */
 if(reply)bp_unref(reply);bp_error_free(&error);bp_queries++;return state;
}
static int bp_send(const char *mac,int intent){
 void *conn=(void *)(unsigned long)PC_WORD(0x83e170);if(!conn)return -1;
 char path[100];bp_path(path,sizeof(path),mac,0);void *msg=bp_message("org.bluez",path,"org.bluez.MediaControl1",intent==1?"Play":"Pause");if(!msg)return -1;
 BtDbusStorage error={0};bp_error_init(&error);void *reply=bp_reply(conn,msg,200,&error);bp_unref(msg);int result=reply&&!bp_error_set(&error)?0:-1;
 if(reply)bp_unref(reply);bp_error_free(&error);return result;
}
static void bp_report(unsigned t){
 if(t-bp_last_report<200u)return;bp_last_report=t;char b[320];int n=snprintf(b,sizeof(b),"version=1.3.32\npid=%d\nconnected=%d\nremote_state=%d\npending=%d\nintent=%d\nattempts=%u\nresult=%d\nrequests=%u\nqueries=%u\nevents=%u\n",getpid(),bp_device[0]!=0,bp_state,bp_job.pending,bp_job.intent,bp_job.attempts,bp_job.result,bp_requests,bp_queries,bp_events);
 int f=open("/tmp/disc-runtime/bt-playback.state.new",1|0x100|0x200,0600);if(f>=0){int ok=write(f,b,n)==n;close(f);if(ok)rename("/tmp/disc-runtime/bt-playback.state.new","/tmp/disc-runtime/bt-playback.state");}
}
static int bp_click(void *request){
 if(dlna_track_click(request))return 0;
 if(!request||!pc_enabled()||pc_workmode()!=2||((unsigned *)request)[4]!=0)return PC_FN(0x4ed84c,int (*)(void *))(request);
 pthread_mutex_lock(bp_mutex);bp_requests++;bp_job=(DiscBtPlay){0};bp_job.pending=2;bp_job.deadline=bp_ms()+1800;bp_target[0]=0;pthread_mutex_unlock(bp_mutex);return 0;
}
static void bp_poll(void){
 if(pthread_mutex_trylock(bp_mutex))return;unsigned t=bp_ms();char mac[20]={0};
 if(!pc_enabled()||pc_workmode()!=2){bp_job.pending=0;bp_device[0]=0;bp_state=-1;bp_probe_attempts=0;bp_report(t);pthread_mutex_unlock(bp_mutex);return;}
 bp_apply_signal();
 /* Native connected-device discovery can enumerate D-Bus objects. Never
  * perform it on every 100ms tick: initial/recovery scan or a new click only. */
 if(bp_job.pending==2||(!bp_device[0]&&(int)(t-bp_next_device)>=0)){
  PC_FN(0x40fb30,int (*)(char *,unsigned))(mac,18);bp_next_device=t+1000;
  if(strcmp(mac,bp_device)){snprintf(bp_device,sizeof(bp_device),"%s",mac);bp_state=-1;bp_probe_attempts=0;bp_next_probe=t;}
 }else snprintf(mac,sizeof(mac),"%s",bp_device);
 t=bp_ms(); /* Native discovery can block: use elapsed time, not old tick. */
 if(bp_job.pending&&bp_target[0]&&strcmp(mac,bp_target)){bp_job.pending=0;bp_job.result=-4;}
 if(bp_job.pending&&(int)(t-bp_job.deadline)>=0){bp_job.pending=0;bp_job.result=-2;}
 if(mac[0]&&((bp_job.pending&&(int)(t-bp_job.next)>=0)||(!bp_job.pending&&bp_state<0&&bp_probe_attempts<3&&(int)(t-bp_next_probe)>=0))){
  if(!PC_WORD(0x83e170))PC_FN(0x40b330,int (*)(void))();
  int state=bp_get_state(mac);bp_state=state;bp_apply_signal();state=bp_state;t=bp_ms();
  if(bp_job.pending&&(int)(t-bp_job.deadline)>=0){bp_job.pending=0;bp_job.result=-2;}
  if(bp_job.pending==2){unsigned deadline=bp_job.deadline;bt_play_begin(&bp_job,t,state);bp_job.deadline=deadline;snprintf(bp_target,sizeof(bp_target),"%s",mac);}
  if(bp_job.pending){int intent=bt_play_due(&bp_job,t,state);if(intent){
    if(intent==1&&PC_BYTE(0x83a75a)==6&&PC_BYTE(0x83a764)){PC_FN(0x4e8b94,void (*)(unsigned))(1);PC_FN(0x4e8888,void (*)(void))();PC_FN(0x4e8b94,void (*)(unsigned))(0);PC_BYTE(0x83a764)=0;PC_FN(0x4eb284,void (*)(void))();}
    bp_job.result=bp_send(mac,intent);bp_next_probe=t+450;
   }}else{bp_probe_attempts++;bp_next_probe=t+450;}
 }
 if(bp_job.pending==1&&bp_job.attempts&&bp_state==(bp_job.intent==1?1:0)){bp_job.pending=0;bp_job.result=1;}
 bp_report(t);pthread_mutex_unlock(bp_mutex);
}
static void bp_install(void){
 if(PC_WORD(0x83a3f8)!=0x4ed84c)return;
#define BP_LOAD(name,symbol) name=(void *)dlsym((void *)0,symbol);if(!name)return
 BP_LOAD(bp_error_init,"dbus_error_init");BP_LOAD(bp_error_free,"dbus_error_free");BP_LOAD(bp_error_set,"dbus_error_is_set");BP_LOAD(bp_unref,"dbus_message_unref");BP_LOAD(bp_message,"dbus_message_new_method_call");BP_LOAD(bp_reply,"dbus_connection_send_with_reply_and_block");BP_LOAD(bp_iter_append_init,"dbus_message_iter_init_append");BP_LOAD(bp_append,"dbus_message_iter_append_basic");BP_LOAD(bp_iter_init,"dbus_message_iter_init");BP_LOAD(bp_iter_type,"dbus_message_iter_get_arg_type");BP_LOAD(bp_recurse,"dbus_message_iter_recurse");BP_LOAD(bp_basic,"dbus_message_iter_get_basic");
 BP_LOAD(bp_next,"dbus_message_iter_next");BP_LOAD(bp_is_signal,"dbus_message_is_signal");BP_LOAD(bp_message_path,"dbus_message_get_path");BP_LOAD(bp_real_pop,"dbus_connection_pop_message");
#undef BP_LOAD
 PC_WORD(PLAYER_DBUS_POP_GOT)=(unsigned)bp_pop;PC_WORD(0x83a3f8)=(unsigned)bp_click;pc_transport_tick=bp_poll;bp_installed=1;bp_poll();
}
