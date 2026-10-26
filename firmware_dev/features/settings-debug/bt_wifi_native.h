#include "bt_wifi_policy.h"
extern int sscanf(const char *,const char *,...);
extern int mkdir(const char *,unsigned);
extern int system(const char *);
static DiscBtWifiPolicy bt_wifi_policy;
static int bt_wifi_loaded;
#define BT_WIFI_SAVE "/usr/data/disc-runtime/module-data/disc-bt-meter/bt-wifi-policy"
static int bt_wifi_store(void){
 char b[180];int n=snprintf(b,sizeof(b),"BT-WIFI-1 %u %u %u %u %u\n",bt_wifi_policy.active,bt_wifi_policy.restore,bt_wifi_policy.pending,bt_wifi_policy.manual,bt_wifi_policy.attempts);
 mkdir("/usr/data/disc-runtime/module-data/disc-bt-meter",0700);
 int f=open(BT_WIFI_SAVE ".new",1|0x100|0x200,0600);if(f<0)return -1;
 int ok=write(f,b,n)==n;if(ok)ok=fsync(f)==0;close(f);
 if(ok&&rename(BT_WIFI_SAVE ".new",BT_WIFI_SAVE)==0)return 0;
 unlink(BT_WIFI_SAVE ".new");return -1;
}
static void bt_wifi_load(void){
 if(bt_wifi_loaded)return;bt_wifi_loaded=1;
 int f=open(BT_WIFI_SAVE,0);if(f<0)return;
 char b[180];int n=read(f,b,sizeof(b)-1);close(f);if(n<=0)return;b[n]=0;
 DiscBtWifiPolicy p={0};
 if(sscanf(b,"BT-WIFI-1 %u %u %u %u %u",&p.active,&p.restore,&p.pending,&p.manual,&p.attempts)==5&&disc_bt_wifi_valid(&p)){
  p.attempts=0;bt_wifi_policy=p;
 }
}
static void bt_wifi_publish(const char *result){
 char b[340];int n=snprintf(b,sizeof(b),"version=1.3.146\npid=%d\nupdated_ms=%u\nmode=%u\nnative_wifi=%u\nactive=%u\nrestore=%u\npending=%u\nmanual_override=%u\nattempts=%u\nresult=%s\n",getpid(),milliseconds(),*(volatile unsigned *)0x8e16d4,*(volatile unsigned char *)0x8e171d,bt_wifi_policy.active,bt_wifi_policy.restore,bt_wifi_policy.pending,bt_wifi_policy.manual,bt_wifi_policy.attempts,result);
 int f=open("/tmp/disc-runtime/bt-wifi.state.new",1|0x100|0x200,0600);if(f<0)return;
 int ok=write(f,b,n)==n;close(f);if(ok)rename("/tmp/disc-runtime/bt-wifi.state.new","/tmp/disc-runtime/bt-wifi.state");
}
static void bt_wifi_manual_request(void){
 bt_wifi_load();
 if(bt_wifi_policy.active||bt_wifi_policy.pending){disc_bt_wifi_manual(&bt_wifi_policy);bt_wifi_store();bt_wifi_publish("manual_override");}
}
static void bt_wifi_tick(void){
 static unsigned started;unsigned clock=milliseconds();
 /* Stock reload first restores its saved mode/Wi-Fi globals. Do not mistake
  * that initialization for a user toggling the radio after our decision. */
 if(!started){started=clock;return;}if(clock-started<5000)return;
 int first=!bt_wifi_loaded;bt_wifi_load();if(first)bt_wifi_publish("loaded");DiscBtWifiPolicy before=bt_wifi_policy;
 unsigned mode=*(volatile unsigned *)0x8e16d4,wifi=*(volatile unsigned char *)0x8e171d;
 unsigned busy=*(volatile unsigned char *)0x8de000,now=milliseconds();
 int interface_up=-1;
 if(bt_wifi_policy.pending){
  char flags[24];int f=open("/sys/class/net/wlan0/flags",0),n=-1;
  if(f>=0){n=read(f,flags,sizeof(flags)-1);close(f);}
  if(n>0){flags[n]=0;interface_up=!!(strtol(flags,0,16)&1);}
 }
 static unsigned stale_since;int resume_native=0,close_native=0;
 if(mode==3&&bt_wifi_policy.pending==1&&wifi&&busy&&interface_up==0){busy=0;close_native=1;}
 /* Stock busy may survive a UI/mode rebuild until the Wi-Fi page refreshes.
  * Only an owned restore, OFF native intent and a physically DOWN interface
  * may bypass it; never clear the stock busy flag or interrupt a live start. */
 if(mode!=3&&bt_wifi_policy.pending==2&&busy){
  if(interface_up==0){
   if(!stale_since)stale_since=now;
   if(now-stale_since>=2000){busy=0;resume_native=1;}
  }else stale_since=0;
 }else stale_since=0;
 /* Confirm physical startup rather than treating a changed UI bit as success. */
 unsigned effective=bt_wifi_policy.pending==2?wifi&&interface_up==1:wifi;
 int action=disc_bt_wifi_step(&bt_wifi_policy,mode,effective,busy,now);
 if(action==1)bt_wifi_policy.next=now+10000;
 int changed=memcmp(&before,&bt_wifi_policy,sizeof(before));
 if(changed&&bt_wifi_store()){
  bt_wifi_policy=before;bt_wifi_publish("save_failed");return;
 }
 if(action>=0&&(wifi!=(unsigned)action||(action==1&&interface_up==0))){
  if(action==0&&close_native){
   FN(0x48b28c,void (*)(const char *,unsigned,const char *))((const char *)0x69fd40,0,0);
   *(volatile unsigned char *)0x8e171d=0;FN(0x481bec,void (*)(int))(0);
  }else if(action==1&&(resume_native||wifi)){
   /* Exact asynchronous ON transaction from verified stock 0x470128.
    * No network daemon launch, resolver rewrite or database write here. */
   FN(0x48b28c,void (*)(const char *,unsigned,const char *))((const char *)0x69fd48,0,0);
   *(volatile unsigned char *)0x8e171d=1;FN(0x481bec,void (*)(int))(1);
  }else FN(0x470128,void (*)(void))();
  if(action==1&&bt_wifi_policy.attempts>=2&&interface_up==0){
   char command[280];snprintf(command,sizeof(command),"nohup sh /usr/data/disc-runtime/components/disc-bt-meter/current/bt-wifi-restore.sh %d >/tmp/disc-runtime/bt-wifi-worker.log 2>&1 </dev/null &",getpid());system(command);
  }
  bt_wifi_publish(action?"restore_requested":"close_requested");
 }else if(changed)bt_wifi_publish(bt_wifi_policy.manual?"manual_override":bt_wifi_policy.pending?"pending":"idle");
}
