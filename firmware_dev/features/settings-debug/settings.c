/* Original V2.57 UI extension. Only the UI main thread creates LVGL objects. */
typedef unsigned size_t;
typedef long ssize_t;
extern int open(const char *,int,...),close(int),read(int,void *,size_t),write(int,const void *,size_t);
extern int access(const char *,int),rename(const char *,const char *),unlink(const char *),fsync(int);
extern ssize_t readlink(const char *,char *,size_t);
extern int memcmp(const void *,const void *,size_t),strcmp(const char *,const char *);
extern void *dlsym(void *,const char *);
extern char *getenv(const char *);
extern int setenv(const char *,const char *,int);
extern int pthread_mutex_trylock(void *),pthread_mutex_unlock(void *);
extern int clock_gettime(int,void *),getpid(void),snprintf(char *,size_t,const char *,...);
extern int socket(int,int,int),ioctl(int,unsigned,...);
extern long strtol(const char *,char **,int);
extern unsigned long long strtoull(const char *,char **,int);
#include "ui_api.h"
#include "guard.h"
#define FN(a,t) LV_PTR(a,t)
#define child FN(0x4f8850,LvObj *(*)(LvObj *,int))
#define child_count FN(0x4f889c,unsigned (*)(LvObj *))
#define move_index FN(0x4f8304,void (*)(LvObj *,int))
#define original_row FN(0x485ef8,void (*)(LvObj *,const char *,unsigned,void (*)(void *),const int *))
#define bg_opa FN(0x4f6b60,void (*)(LvObj *,unsigned,unsigned))
#define pad_top FN(0x4f6af0,void (*)(LvObj *,int,unsigned))
#define pad_bottom FN(0x4f6b00,void (*)(LvObj *,int,unsigned))
#define pad_left FN(0x4f6b10,void (*)(LvObj *,int,unsigned))
#define pad_right FN(0x4f6b20,void (*)(LvObj *,int,unsigned))
#define native_title FN(0x46d88c,void (*)(LvObj *,const char *,unsigned))
#define screen_load FN(0x4e2734,void (*)(LvObj *))
#define screen_delete FN(0x4f7e98,void (*)(LvObj **))
#define screen_register FN(0x486620,void (*)(LvObj *))
#define indev_active FN(0x4e42e8,void *(*)(void))
#define indev_point FN(0x4e45bc,void (*)(void *,int *))
static int (*real_usleep)(unsigned);
static LvObj *page,*row,*debug_page,*switch_row[2],*pill[2],*state_label[2],*info,*load_button,*load_label;
static char load_text[160];
static LvObj *module_row,*storage_row,*auto_label,*manager_info,*manager_toggle,*module_result,*storage_info[2];
static LvObj *backup_row,*wifi_parent,*wifi_saved_row,*debug_origin,*stock_rows[16];
static int view,requested_view,module_index,module_count;
static struct {char id[65],version[65],kind[12],location[12],state[16];int enabled;} module_items[96];
static LvObj *module_list,*managed_rows[96],*managed_labels[96],*managed_states[96];
static int managed_count;
static char selected_module[65];
static unsigned bt_poll_at,settings_poll_at;
static void (*real_display_refresh)(void *);
static void *display_timer;
static int preparing_frame;
static long heartbeat_at;
static int last_state[2]={-1,-1},reentrant,pending_open,pending_back,swipe;
static char last_info[128];
static int actual_usb,actual_wireless,actual_rescue,actual_fresh,actual_reconnect,actual_paused,actual_priority,actual_priority_error;
#include "debug_view.h"
static DiscDebugView debug_view;
static LvObj *tap_target;
static int tap_x,tap_y,tap_moved=1,tap_released;
static unsigned tap_started,tap_duration;
static int stationary_clicked(void *);
static void protect_tap(LvObj *);
static void display_settings_prepare(void);
static void pu_prepare(void);
static void read_actual(void) {
 char b[512];int f,r,i;long updated=0,t[2];actual_usb=actual_wireless=actual_rescue=actual_fresh=actual_reconnect=actual_paused=actual_priority=actual_priority_error=0;
 f=open("/tmp/disc-runtime/state",0);if(f<0)return;
 r=read(f,b,sizeof(b)-1);close(f);if(r<0)return;b[r]=0;
 for(i=0;i<r;i++)if(i==0||b[i-1]=='\n'){
  if(r-i>=12&&!memcmp(b+i,"usb_active=",11))actual_usb=b[i+11]=='1';
  if(r-i>=17&&!memcmp(b+i,"wireless_active=",16))actual_wireless=b[i+16]=='1';
  if(r-i>=8&&!memcmp(b+i,"rescue=",7))actual_rescue=b[i+7]=='1';
  if(r-i>=14&&!memcmp(b+i,"pause_reason=",13)&&b[i+13]!='\n')actual_paused=1;
  if(r-i>=9&&!memcmp(b+i,"updated=",8))updated=strtol(b+i+8,0,10);
  if(r-i>=19&&!memcmp(b+i,"transition=wireless",19))actual_reconnect=1;
  if(r-i>=14&&!memcmp(b+i,"usb_priority=",13)&&b[i+13]!='\n')actual_priority=1;
  if(r-i>=10&&!memcmp(b+i,"usb_error=",10)&&b[i+10]!='\n')actual_priority_error=1;
 }
 if(!clock_gettime(1,t)&&updated>0&&t[0]>=updated&&t[0]-updated<=5)actual_fresh=1;
}
static void heartbeat(void) {
 long t[2];char b[64];int n,f;if(clock_gettime(1,t))return;
 if(heartbeat_at && t[0]-heartbeat_at<10)return;
 n=snprintf(b,sizeof(b),"%d %ld\n",getpid(),t[0]);
 f=open("/tmp/disc-settings.heartbeat",1|0x100|0x200,0600);
 if(f>=0){if(write(f,b,n)==n)heartbeat_at=t[0];close(f);}
}

static void note(const char *s) {
 int f=open("/usr/data/disc-settings.log",1|0x100|0x8,0600); /* MIPS O_CREAT=0x100 */
 if(f>=0){unsigned n=0;while(s[n])n++;write(f,s,n);close(f);}
}
/* RAM-only stage trail; no extra thread, log loop or flash writes. Records
 * cold-start constructors and the first real UI loop in each process. */
static void startup_trace(const char *stage){
 long t[2];char b[140];if(clock_gettime(1,t))return;
 int n=snprintf(b,sizeof(b),"%ld.%03ld pid=%d %s\n",t[0],t[1]/1000000,getpid(),stage);
 int fd=open("/tmp/disc-module-startup.log",1|0x100|0x8,0600);if(fd>=0){write(fd,b,n);close(fd);}
}
#include "startup_gate.h"
static DiscStartupGate startup_gate;
static int startup_is_app(void){
 char exe[128];int n=readlink("/proc/self/exe",exe,sizeof(exe)-1);
 if(n<0||n>=(int)sizeof(exe)-1)return 0;exe[n]=0;
 return !strcmp(exe,"/usr/bin/mq_ui")||!strcmp(exe,"/usr/bin/mq_player");
}
#include "cache_sync.h"
#include "diag_bridge.h"
#include "bt_bridge.h"
static int identity(void) {
 char exe[128],buf[4096];unsigned n,total=0,fnv=2166136261u,i;int f;
 n=readlink("/proc/self/exe",exe,sizeof(exe)-1);if(n>=sizeof(exe)-1)return 0;exe[n]=0;
 if(strcmp(exe,"/usr/bin/mq_ui"))return 0;
 f=open(exe,0);if(f<0)return 0;
 for(;;){int r=read(f,buf,sizeof(buf));if(r<0){close(f);return 0;}if(!r)break;total+=r;for(i=0;i<(unsigned)r;i++)fnv=(fnv^(unsigned char)buf[i])*16777619u;}
 close(f);if(total!=UI_SIZE||fnv!=UI_FNV)return 0;
 for(i=0;i<sizeof(guards)/sizeof(guards[0]);i++)if(memcmp((void *)guards[i].address,guards[i].bytes,16))return 0;
 return 1;
}
static int player_identity(void){
 char exe[128],b[4096];int n=readlink("/proc/self/exe",exe,sizeof(exe)-1);if(n<0||n>=127)return 0;exe[n]=0;
 if(strcmp(exe,"/usr/bin/mq_player"))return 0;int f=open(exe,0);if(f<0)return 0;unsigned total=0,hash=2166136261u;
 while((n=read(f,b,sizeof(b)))>0){total+=n;for(int k=0;k<n;k++)hash=(hash^(unsigned char)b[k])*16777619u;}
 close(f);return n==0&&total==PLAYER_SIZE&&hash==PLAYER_FNV;
}
static void enable_wifi_capture(void){
 const char prefix[]="/usr/data/disc-runtime/service/wifi-tools";if(access("/usr/data/disc-runtime/service/wifi-tools/wpa_passphrase",1))return;
 char path[1200];const char *old=getenv("PATH");if(old){unsigned k=0;while(k<sizeof(prefix)-1&&old[k]&&old[k]==prefix[k])k++;if(k==sizeof(prefix)-1)return;}
 int n=snprintf(path,sizeof(path),"%s:%s",prefix,old?old:"/usr/sbin:/usr/bin:/sbin:/bin");if(n>0&&n<(int)sizeof(path))setenv("PATH",path,1);
}
static int config(int *usb,int *wireless) {
 char b[256];int f,r,i;*usb=1;*wireless=0;
 f=open("/usr/data/disc-runtime/debug.conf",0);if(f<0)return -1;
 r=read(f,b,sizeof(b)-1);close(f);if(r<0)return -1;b[r]=0;
 for(i=0;i<r;i++)if(i==0||b[i-1]=='\n'){
  if(r-i>=5&&!memcmp(b+i,"usb=",4)&&(b[i+4]=='0'||b[i+4]=='1'))*usb=b[i+4]-'0';
  if(r-i>=10&&!memcmp(b+i,"wireless=",9)&&(b[i+9]=='0'||b[i+9]=='1'))*wireless=b[i+9]-'0';
 }
 return 0;
}
static void reset_debug(void) {
 debug_page=info=load_button=load_label=0;swipe=pending_back=0;last_info[0]=load_text[0]=0;
 auto_label=manager_info=manager_toggle=module_result=0;
 module_list=0;managed_count=0;
 for(int i=0;i<2;i++){switch_row[i]=pill[i]=state_label[i]=0;last_state[i]=-1;}
 unlink("/tmp/disc-runtime/wifi-secret");
 tap_target=0;tap_moved=1;tap_released=0;
}
static void deleted(void *e) {
 LvObj *target=lv_event_target(e);
 if(target==page){page=row=module_row=storage_row=backup_row=0;for(int i=0;i<16;i++)stock_rows[i]=0;pending_open=0;}
 if(target==wifi_parent){wifi_parent=wifi_saved_row=0;}
 if(target==debug_origin)debug_origin=0;
 if(target==debug_page)reset_debug();
}
static void network_text(char b[128],int wireless) {
 char carrier[2];unsigned char req[64]={0};int f,r;
 if(!wireless){snprintf(b,128,"无线调试已关闭");return;}
 f=open("/sys/class/net/wlan0/carrier",0);r=f>=0?read(f,carrier,1):-1;if(f>=0)close(f);
 if(r!=1||carrier[0]!='1'){snprintf(b,128,"Wi-Fi 未连接\n连接后可用无线调试");return;}
 const char name[]="wlan0";for(unsigned i=0;i<sizeof(name);i++)req[i]=name[i];
 f=socket(2,2,0);r=f>=0?ioctl(f,0x8915,req):-1;if(f>=0)close(f);
 if(r||!(req[20]|req[21]|req[22]|req[23])){snprintf(b,128,"Wi-Fi 已连接\n等待获取地址");return;}
 snprintf(b,128,"%u.%u.%u.%u:5555\n同一 Wi-Fi 可连接",req[20],req[21],req[22],req[23]);
}
static void refresh_extra(void);
static void refresh(void) {
 int requested[2],values[2];char b[128];if(!debug_page||!lv_valid(debug_page))return;
 if(view){refresh_extra();return;}
 if(config(&requested[0],&requested[1])<0)requested[0]=requested[1]=-1;
 read_actual();int mode=*(volatile int *)0x8e16d4;
 int paused=(mode==1||mode==2||mode==4||(actual_fresh&&actual_paused))&&!actual_rescue&&!actual_priority;
 long clock[2]={0};clock_gettime(1,clock);
 int active[2]={actual_usb,actual_wireless};
 for(int i=0;i<2;i++){
  values[i]=disc_debug_value(&debug_view,(unsigned)clock[0],i,requested[i],active[i],actual_fresh,paused,actual_rescue,actual_reconnect);
 }
 if(actual_fresh&&actual_priority_error)values[0]=5;
 for(int i=0;i<2;i++)if(state_label[i]&&values[i]!=last_state[i]){
  last_state[i]=values[i];lv_text(state_label[i],values[i]==1?"开启":values[i]==0?"关闭":values[i]==2?"暂停":values[i]==3?"切换中":values[i]==4?"救援":values[i]==6?"重连中":values[i]==5?"未就绪":"异常");
  lv_bg(pill[i],values[i]==1||values[i]==4?0xff187bbb:0xff555555,0);
 }
 if(actual_fresh&&actual_rescue)snprintf(b,128,"TF 救援已启用\nUSB 调试优先");
 else if(actual_fresh&&actual_priority_error)snprintf(b,128,"USB 音频未能释放\n关闭后可重新开启调试");
 else if(values[0]==5||values[1]==5)snprintf(b,128,actual_fresh?"调试连接尚未就绪\n可关闭后重新开启":"调试服务没有响应\n状态更新已超时");
 else if(values[0]==6)snprintf(b,128,"无线调试切换中\nUSB 开关保持开启");
 else if(values[1]==3)snprintf(b,128,"调试状态切换中\n稍候自动更新");
 else if(values[0]==2&&requested[1]!=1)snprintf(b,128,"%s模式占用 USB\n退出后自动恢复调试",mode==1?"USB DAC ":mode==2?"USB AUDIO ":"存储");
 else if(values[0]==3)snprintf(b,128,"USB 调试正在恢复\n稍候自动更新");
 else network_text(b,requested[1]==1&&actual_wireless&&actual_fresh);
 if(strcmp(b,last_info)){unsigned n=0;do{last_info[n]=b[n];}while(b[n++]);lv_text(info,b);}
 if(load_label){
  char result[160];int f=open("/tmp/disc-runtime/tf-load.result",0),r=-1;
  if(f>=0){r=read(f,result,sizeof(result)-1);close(f);}
  if(r>0){result[r]=0;while(r>0&&(result[r-1]=='\n'||result[r-1]=='\r'))result[--r]=0;}
  else snprintf(result,sizeof(result),"加载 TF 模块");
  if(strcmp(result,load_text)){snprintf(load_text,sizeof(load_text),"%s",result);lv_text(load_label,load_text);}
 }
}
static void load_clicked(void *e){
 if(!stationary_clicked(e))return;
 int f=open("/tmp/disc-runtime/tf-load.request",1|0x100|0x200,0600);
 if(f>=0){close(f);note("TF module import requested\n");}
}
static void clicked(void *e) {
 int u,w,f,r;char b[]="usb=1\nwireless=0\n";LvObj *target=lv_event_target(e);
 if(!stationary_clicked(e)||pending_back||config(&u,&w)<0)return;
 if(target==switch_row[0])u=!u;else if(target==switch_row[1])w=!w;else return;
 b[4]='0'+u;b[15]='0'+w;
 f=open("/usr/data/disc-runtime/debug.conf.settings-new",1|0x100|0x200,0600);
 if(f<0){note("config open failed\n");return;}
 r=write(f,b,sizeof(b)-1);if(r==(int)sizeof(b)-1)r=fsync(f);else r=-1;close(f);
 if(r||rename("/usr/data/disc-runtime/debug.conf.settings-new","/usr/data/disc-runtime/debug.conf")){
  unlink("/usr/data/disc-runtime/debug.conf.settings-new");note("config commit failed\n");return;
 }
 if(target==switch_row[0]){
  unlink("/tmp/disc-runtime/debug-usb-priority.ack");
  if(u){long time[2];if(!clock_gettime(1,time)){char request[96];int n=snprintf(request,sizeof(request),"%d %u %ld\n",getpid(),*(unsigned *)0x8e16d4,time[0]);int fd=open("/tmp/disc-runtime/debug-usb-priority.new",1|0x100|0x200,0600);if(fd>=0){int ok=write(fd,request,n)==n;close(fd);if(ok)rename("/tmp/disc-runtime/debug-usb-priority.new","/tmp/disc-runtime/debug-usb-priority");}}}
  else unlink("/tmp/disc-runtime/debug-usb-priority");
 }
 note(target==switch_row[0]?(u?"USB on requested\n":"USB off requested\n"):(w?"wireless on requested\n":"wireless off requested\n"));refresh();
 if(target==switch_row[1]&&w){int f=open("/usr/data/disc-settings.wifi-enable",1|0x100|0x200,0600);if(f>=0)close(f);}
}

#include "bt_wifi_native.h"
/* Explicit requests only. Use the stock asynchronous Wi-Fi command and UI
 * state, never infer OFF from a temporarily unassociated wlan0 interface. */
static void wifi_request(void){
 if(access("/usr/data/disc-settings.wifi-enable",0))return;
 if(*(volatile unsigned char *)0x8de000)return; /* Native operation in progress. */
 bt_wifi_manual_request();
 if(!*(volatile unsigned char *)0x8e171d)FN(0x470128,void (*)(void))();
 else {
  char carrier=0;int f=open("/sys/class/net/wlan0/carrier",0);if(f>=0){read(f,&carrier,1);close(f);}
  if(carrier!='1')FN(0x48b28c,void (*)(const char *,unsigned,const char *))((const char *)0x69fd48,0,0);
  FN(0x481bec,void (*)(int))(1);
 }
 unlink("/usr/data/disc-settings.wifi-enable");note("Wi-Fi enabled through native control\n");
}
static void open_clicked(void *e){
 if(!stationary_clicked(e))return;
 LvObj *target=lv_event_target(e);
 debug_origin=target==wifi_saved_row?wifi_parent:page;
 requested_view=target==module_row?1:target==storage_row?3:target==backup_row?5:target==wifi_saved_row?8:0;pending_open=1;
}
static void back_clicked(void *e){if(stationary_clicked(e))pending_back=1;}
static void back_swipe(void *e) {
 int result=FN(0x488860,int (*)(void *))(e);
 if(result==1)swipe=1;
 else if(result==3){swipe=0;pending_back=1;}
 else if(result==4)swipe=0;
}
static unsigned milliseconds(void){long t[2];if(clock_gettime(1,t))return 0;return (unsigned)t[0]*1000u+(unsigned)t[1]/1000000u;}
static void tap_guard(void *e){
 unsigned code=lv_event_code(e);LvObj *target=lv_event_target(e);int point[2];void *input=indev_active();
 if(code==1){tap_target=target;tap_moved=1;tap_released=0;tap_started=milliseconds();if(!input)return;indev_point(input,point);tap_x=point[0];tap_y=point[1];tap_moved=0;}
 else if(target==tap_target){
  if(input){indev_point(input,point);int dx=point[0]-tap_x,dy=point[1]-tap_y;if(dx>8||dx< -8||dy>8||dy< -8)tap_moved=1;}
  else tap_moved=1;
  if(code==8){tap_released=1;tap_duration=milliseconds()-tap_started;}
 }
}
static void protect_tap(LvObj *obj){
 lv_add_flag(obj,0x4000); /* Bubble gesture events through buttons to the page. */
 lv_add_cb(obj,tap_guard,1,0);lv_add_cb(obj,tap_guard,2,0);lv_add_cb(obj,tap_guard,8,0);
}
static int stationary_clicked(void *e){return lv_event_code(e)==DISC_LV_CLICKED&&!pending_back&&tap_target==lv_event_target(e)&&tap_released&&!tap_moved;}
static void make_debug(void) {
 view=0;
 debug_page=lv_create(0);if(!debug_page)return;
 screen_register(debug_page);lv_add_cb(debug_page,deleted,DISC_LV_DELETE,0);
 lv_size(debug_page,360,360);lv_border(debug_page,0,0);lv_bg(debug_page,0xff000000,0);
 pad_top(debug_page,0,0);pad_bottom(debug_page,0,0);pad_left(debug_page,0,0);pad_right(debug_page,0,0);
 lv_clear_flag(debug_page,DISC_LV_SCROLLABLE);
 native_title(debug_page,"调试",0xb4fb);
 LvObj *title=child(debug_page,0);lv_align(title,1,0,44);lv_color(title,0xff54baff,0);
 lv_add_cb(debug_page,back_swipe,1,0);lv_add_cb(debug_page,back_swipe,8,0);
 lv_add_cb(debug_page,back_swipe,2,0);
 for(int i=0;i<2;i++){
  int layout[3]={44,180,0};unsigned n=child_count(debug_page);
  original_row(debug_page,i?"无线调试":"USB 调试",i,clicked,layout);
  switch_row[i]=child(debug_page,n);
  protect_tap(switch_row[i]);
  /* Bound the pressed background as well as the text; keep clear of title/IP. */
  lv_size(switch_row[i],360,52);pad_top(switch_row[i],0,0);pad_bottom(switch_row[i],0,0);
  lv_align(switch_row[i],1,0,92+i*68);
  LvObj *arrow=child(switch_row[i],1);if(arrow)lv_add_flag(arrow,DISC_LV_HIDDEN);
  pill[i]=lv_create(switch_row[i]);lv_size(pill[i],62,30);lv_align(pill[i],7,235,0);
  lv_border(pill[i],0,0);lv_radius(pill[i],15,0);bg_opa(pill[i],255,0);
  pad_top(pill[i],0,0);pad_bottom(pill[i],0,0);pad_left(pill[i],0,0);pad_right(pill[i],0,0);
  lv_clear_flag(pill[i],DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);
  state_label[i]=lv_label(pill[i]);lv_add_style(state_label[i],(void *)0x83ac5c,0);
  lv_color(state_label[i],0xffffffff,0);lv_align(state_label[i],9,0,0);last_state[i]=-1;
 }
 info=lv_label(debug_page);lv_add_style(info,(void *)0x83ac5c,0);lv_color(info,0xffaaaaaa,0);
 lv_align(info,9,0,61);
 LvObj *back=lv_btn(debug_page);lv_size(back,100,30);lv_align(back,9,0,123);
 lv_border(back,0,0);lv_radius(back,17,0);lv_bg(back,0xff333333,0);
 LvObj *label=lv_label(back);lv_add_style(label,(void *)0x83ac5c,0);lv_color(label,0xffffffff,0);lv_text(label,"返回");lv_align(label,9,0,0);
 lv_add_cb(back,back_clicked,7,0);refresh();screen_load(debug_page);note("debug submenu opened\n");
 protect_tap(back);
}
static int read_data(const char *path,char *b,int size){
 int f=open(path,0),r=-1;if(f>=0){r=read(f,b,size-1);close(f);}if(r<0)r=0;b[r]=0;return r;
}
static LvObj *text_at(LvObj *p,const char *s,int y,unsigned color){
 LvObj *l=lv_label(p);lv_add_style(l,(void *)0x83ac5c,0);lv_color(l,color,0);lv_text(l,s);lv_align(l,9,0,y);return l;
}
static LvObj *button_at(LvObj *p,const char *s,int x,int y,int width,void (*cb)(void *)){
 LvObj *o=lv_btn(p);lv_size(o,width,34);lv_align(o,9,x,y);lv_border(o,0,0);lv_radius(o,17,0);lv_bg(o,0xff333333,0);
 text_at(o,s,0,0xffffffff);protect_tap(o);lv_add_cb(o,cb,DISC_LV_CLICKED,0);return o;
}
static void automatic_clicked(void *e){
 if(!stationary_clicked(e))return;
 const char *path="/usr/data/disc-runtime/modules-autoload";
 if(!access(path,0))unlink(path);else {int f=open(path,1|0x100|0x200,0600);if(f>=0){fsync(f);close(f);}}
 refresh_extra();
}
static void manager_clicked(void *e){if(stationary_clicked(e)){requested_view=2;pending_open=1;}}
static void module_toggle_clicked(void *e){
 if(!stationary_clicked(e)||!module_count)return;
 LvObj *target=lv_event_target(e);
 for(int i=0;i<managed_count;i++)if(target==managed_rows[i])module_index=i;
 char b[96];int n=snprintf(b,sizeof(b),"%s|%s\n",module_items[module_index].enabled?"disable":"enable",module_items[module_index].id);
 int f=open("/tmp/disc-runtime/module-command.new",1|0x100|0x200,0600);
 if(f<0)return;int r=write(f,b,n);close(f);
 if(r==n)rename("/tmp/disc-runtime/module-command.new","/tmp/disc-runtime/module-command");
}
static void module_detail_clicked(void *e){
 if(!stationary_clicked(e))return;
 LvObj *target=lv_event_target(e);
 for(int i=0;i<managed_count;i++)if(target==managed_rows[i]){module_index=i;snprintf(selected_module,sizeof(selected_module),"%s",module_items[i].id);requested_view=4;pending_open=1;break;}
}
static void read_modules(void){
 static char b[16384];int r=read_data("/tmp/disc-runtime/modules.list",b,sizeof(b));module_count=0;
 for(int i=0;i<r&&module_count<96;){
  char *fields[6];int start=i,n=1;fields[0]=b+i;
  while(i<r&&b[i]!='\n'){if(b[i]=='|'&&n<6){b[i]=0;fields[n++]=b+i+1;}i++;}
  if(i<r)b[i++]=0;
  if(n!=6||i-start>192)continue;
  int k=module_count++;snprintf(module_items[k].id,65,"%s",fields[0]);module_items[k].enabled=fields[1][0]=='1';
  snprintf(module_items[k].kind,12,"%s",fields[2]);snprintf(module_items[k].location,12,"%s",fields[3]);
  snprintf(module_items[k].state,16,"%s",fields[4]);snprintf(module_items[k].version,65,"%s",fields[5]);
 }
 if(module_index>=module_count)module_index=0;
}
static void number_text(char *b,unsigned long long value,int gib){
 unsigned hundredth=(unsigned)(gib?(value*100)>>20:(value*100)>>10);
 snprintf(b,32,"%u.%02u",hundredth/100,hundredth%100);
}
#include "personal_ui.h"
static void refresh_extra(void){
 char b[256];
 if(view>=5){refresh_personal();return;}
 if(view==1){
  lv_text(auto_label,access("/usr/data/disc-runtime/modules-autoload",0)?"关闭":"开启");
  int n=read_data("/tmp/disc-runtime/tf-load.result",b,sizeof(b));while(n>0&&(b[n-1]=='\n'||b[n-1]=='\r'))b[--n]=0;
  lv_text(module_result,n?b:"默认安装到内置存储");
 }else if(view==2){
  read_modules();
  if(!module_count){lv_text(manager_info,"暂无已安装模块");return;}
  if(managed_count!=module_count){requested_view=2;pending_open=1;return;}
  for(int k=0;k<module_count;k++){
   const char *state=module_items[k].state;
   const char *status=!strcmp(state,"running")?"运行中":!strcmp(state,"missing")?"缺卡":!strcmp(state,"safe")?"安全模式":!module_items[k].enabled?"已停用":"就绪";
   char name[40];int n=0;while(module_items[k].id[n])n++;
   if(!strcmp(module_items[k].id,"disc-audio-xy"))snprintf(name,sizeof(name),"李萨如图");else if(!strcmp(module_items[k].id,"disc-hwaccel"))snprintf(name,sizeof(name),"硬件加速组件");else if(!strcmp(module_items[k].id,"disc-bt-meter"))snprintf(name,sizeof(name),"蓝牙声道电平");else if(n>16)snprintf(name,sizeof(name),"%.10s…%s",module_items[k].id,module_items[k].id+n-4);else snprintf(name,sizeof(name),"%s",module_items[k].id);
   snprintf(b,sizeof(b),"%s\n%s · %s",name,!strcmp(module_items[k].location,"TF")?"TF 卡":"内置",status);
   lv_text(managed_labels[k],b);lv_text(managed_states[k],module_items[k].enabled?"开启":"关闭");
  }
 }else if(view==4){
  read_modules();int found=0;for(int i=0;i<module_count;i++)if(!strcmp(selected_module,module_items[i].id)){module_index=i;found=1;break;}
  if(!found){lv_text(manager_info,"模块已移除");lv_text(manager_toggle,"不可操作");return;}
  int k=module_index;const char *state=module_items[k].state;
  const char *status=!strcmp(state,"running")?"运行中":!strcmp(state,"missing")?"TF 卡未挂载":!strcmp(state,"safe")?"安全模式暂停":!module_items[k].enabled?"已停用":!strcmp(module_items[k].kind,"tool")?"已启用，按需调用":"已启用，等待运行";
  snprintf(b,sizeof(b),"%.24s\n版本 %.20s\n存放：%s\n%s",module_items[k].id,module_items[k].version,!strcmp(module_items[k].location,"TF")?"TF 卡":"内置存储",status);
  lv_text(manager_info,b);lv_text(manager_toggle,access("/tmp/disc-runtime/module-command",0)?(module_items[k].enabled?"关闭此模块":"启用此模块"):"正在处理");
 }else if(view==3){
  char data[256];read_data("/tmp/disc-runtime/storage.state",data,sizeof(data));
  for(int k=0;k<2;k++){
   char *p=data;const char *prefix=k?"tf=":"nand=";int len=k?3:5;
   while(*p){if(!memcmp(p,prefix,len))break;while(*p&&*p!='\n')p++;if(*p)p++;}
   if(!*p||p[len]=='a'){snprintf(b,sizeof(b),k?"TF 卡\n未挂载":"内置存储\n正在读取");}
   else{
    char *end;unsigned long long total=strtoull(p+len,&end,10),used=0,free=0;
    if(*end==',')used=strtoull(end+1,&end,10);if(*end==',')free=strtoull(end+1,&end,10);
    int gib=total>=1048576;char a[32],u[32],f[32];number_text(a,total,gib);number_text(u,used,gib);number_text(f,free,gib);
    snprintf(b,sizeof(b),"%s（%s）\n总 %s · 已用 %s\n剩余 %s",k?"TF 卡":"内置存储",gib?"GiB":"MiB",a,u,f);
   }
   lv_text(storage_info[k],b);
  }
 }
}
static void make_extra(int kind){
 if(kind>=5){make_personal(kind);return;}
 view=kind;debug_page=lv_create(0);screen_register(debug_page);lv_add_cb(debug_page,deleted,DISC_LV_DELETE,0);
 lv_size(debug_page,360,360);lv_border(debug_page,0,0);lv_bg(debug_page,0xff000000,0);
 pad_top(debug_page,0,0);pad_bottom(debug_page,0,0);pad_left(debug_page,0,0);pad_right(debug_page,0,0);
 if(kind!=2)lv_clear_flag(debug_page,DISC_LV_SCROLLABLE);
 else {FN(0x4f0268,void (*)(LvObj *,int))(debug_page,0);FN(0x4f02c8,void (*)(LvObj *,int))(debug_page,12);}
 native_title(debug_page,kind==1?"模块":kind==2?"模块管理器":kind==4?"模块信息":"存储空间",0xb4fb);lv_align(child(debug_page,0),1,0,44);lv_color(child(debug_page,0),0xff54baff,0);
 lv_add_cb(debug_page,back_swipe,1,0);lv_add_cb(debug_page,back_swipe,8,0);
 lv_add_cb(debug_page,back_swipe,2,0);
 if(kind==1){
  const char *names[3]={"开机自动加载","手动加载 TF 模块","模块管理器"};
  void (*callbacks[3])(void *)={automatic_clicked,load_clicked,manager_clicked};
  for(int i=0;i<3;i++){
   int layout[3]={44,180,0};unsigned n=child_count(debug_page);original_row(debug_page,names[i],i,callbacks[i],layout);
   LvObj *r=child(debug_page,n);lv_size(r,360,50);pad_top(r,0,0);pad_bottom(r,0,0);
   protect_tap(r);
   lv_align(r,1,0,90+i*56);
   if(i<2)lv_add_flag(child(r,1),DISC_LV_HIDDEN);
   if(i==0){auto_label=text_at(r,"",0,0xff54baff);lv_align(auto_label,7,235,0);}
  }
  module_result=text_at(debug_page,"",90,0xffaaaaaa);
 }else if(kind==2){
  read_modules();manager_info=module_count?0:text_at(debug_page,"暂无已安装模块",0,0xffdddddd);
  FN(0x5030a0,void (*)(LvObj *,int))(debug_page,1);
  FN(0x5030e4,void (*)(LvObj *,int,int,int))(debug_page,0,0,2);
  LvObj *top=lv_create(debug_page);lv_size(top,360,40);lv_border(top,0,0);lv_bg(top,0xff000000,0);lv_clear_flag(top,DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);move_index(top,0);
  module_list=debug_page;
  for(int i=0;i<module_count;i++){
   int layout[3]={44,180,0};unsigned n=child_count(module_list);original_row(module_list,"",i,module_detail_clicked,layout);
   LvObj *r=child(module_list,n);lv_size(r,360,76);managed_rows[i]=r;managed_labels[i]=child(r,0);
   protect_tap(r);
   managed_states[i]=text_at(r,"",0,0xff54baff);lv_align(managed_states[i],7,235,0);
  }
  managed_count=module_count;
 }else if(kind==4){
  manager_info=text_at(debug_page,"正在读取",-20,0xffdddddd);
  LvObj *toggle=button_at(debug_page,"",0,78,190,module_toggle_clicked);manager_toggle=child(toggle,0);
 }else {storage_info[0]=text_at(debug_page,"正在读取",-45,0xffdddddd);storage_info[1]=text_at(debug_page,"正在读取",48,0xffdddddd);}
 if(kind==2){
  LvObj *footer=lv_create(debug_page);lv_size(footer,360,70);lv_border(footer,0,0);lv_bg(footer,0xff000000,0);lv_clear_flag(footer,DISC_LV_SCROLLABLE);
  lv_add_flag(footer,0x4000);
  button_at(footer,"返回",0,0,100,back_clicked);pad_bottom(debug_page,80,0);
 }else button_at(debug_page,"返回",0,125,100,back_clicked);
 refresh_extra();screen_load(debug_page);
 note(kind==1?"modules page opened\n":kind==2?"module manager opened\n":"storage page opened\n");
}
static void close_debug(int load_parent) {
 LvObj *old=debug_page;
 if(load_parent&&debug_origin&&lv_valid(debug_origin))screen_load(debug_origin);
 reset_debug();if(old&&lv_valid(old))screen_delete(&old);
 note("debug submenu closed\n");
}
static void repair_native_labels(void){
 if(!page||!module_row||!lv_valid(page))return;
 LvObj *label=child(module_row,0);if(!label)return;
 /* Verified V2.57 lv_label text field from FUN_0052f2bc/FUN_0052f4f0. */
 const char *current=*(const char **)((char *)label+0x2c);
 if(!current||!strcmp(current,"模块"))return;
 char text[16][128];
 for(int i=0;i<16;i++){
  LvObj *l=child(child(page,i+1),0);
  const char *s=*(const char **)((char *)l+0x2c);snprintf(text[i],128,"%s",s?s:"");
 }
 for(int i=0;i<16;i++)if(stock_rows[i]&&lv_valid(stock_rows[i]))lv_text(child(stock_rows[i],0),text[i]);
 lv_text(label,"模块");note("native labels refreshed after first-row insertion\n");
 if(backup_row)lv_text(child(backup_row,0),"数据备份与恢复");
}
static void prepare_system_rows(void) {
 LvObj *p=LV_GLOBAL(0x8dd4f0);unsigned count;
 if(p&&lv_valid(p)&&p!=page){
  count=child_count(p);if(count!=18)return;
  for(int i=0;i<16;i++)stock_rows[i]=child(p,i+1);
  /* Index 0 is title, 1..16 are stock rows, 17 is stock bottom spacer. */
  original_row(p,"模块",16,open_clicked,0);module_row=child(p,18);if(!module_row)return;move_index(module_row,1);
  protect_tap(module_row);
  original_row(p,"数据备份与恢复",19,open_clicked,0);backup_row=child(p,19);if(!backup_row)return;move_index(backup_row,17);
  protect_tap(backup_row);
  original_row(p,"调试",17,open_clicked,0);row=child(p,20);if(!row)return;move_index(row,19);
  protect_tap(row);
  original_row(p,"存储空间",18,open_clicked,0);storage_row=child(p,21);if(!storage_row)return;move_index(storage_row,20);page=p;
  protect_tap(storage_row);
  lv_add_cb(page,deleted,DISC_LV_DELETE,0);
  note("system settings debug folder appended\n");
 }
 repair_native_labels();
 display_settings_prepare();
 pu_prepare();
}
static int connect_saved_native_wifi(void *e){
 if(!stationary_clicked(e)||personal_busy())return 1;
 void *data=FN(0x4e394c,void *(*)(void *))(e);if(!data)return 1;
 const char *ssid=*(const char **)data;if(!ssid)return 1;
 read_personal_list(8);
 for(int k=0;k<personal_count;k++)if(!strcmp(ssid,wifi_items[k].ssid)){
  /* Route saved scan results through the existing credential manager. */
  personal_send("connect",wifi_items[k].key);
  return 1;
 }
 return 0;
}
static void native_wifi_click(void *e){if(!connect_saved_native_wifi(e))FN(0x46ea1c,void (*)(void *))(e);}
static void native_connected_wifi_click(void *e){
 /* The connected-row callback opens native IP/network details. It does not
  * reconnect or disconnect the AP; saved-network routing must not consume it. */
 if(stationary_clicked(e))FN(0x46ec0c,void (*)(void *))(e);
}
static void bind_wifi_scan_rows(void){
 for(int c=0;c<2;c++){
 LvObj *container=LV_GLOBAL(c?0x8de020:0x8de01c);if(!container||!lv_valid(container))continue;
 unsigned count=child_count(container);if(count>100)return;
 for(unsigned k=0;k<count;k++){
  LvObj *item=child(container,k);if(!item)continue;
  /* Verified V2.57 lv_obj_add_event_cb: spec_attr +8, descriptor array +12,
   * count bits 16..21 at +32, each descriptor callback/user/filter is 12 bytes.
   * Replace only the exact stock AP click callback; preserve user data and
   * stock deletion/free callbacks. All writes occur on the UI thread/lock. */
  char *spec=*(char **)((char *)item+8);if(!spec)continue;
  unsigned n=(*(unsigned *)(spec+32)>>16)&63;unsigned *events=*(unsigned **)(spec+12);
  if(!events||n>20)continue;
  int changed=0;
  for(unsigned j=0;j<n;j++)if(events[j*3]==(unsigned)(c?0x46ec0c:0x46ea1c)&&((unsigned char *)(events+j*3))[8]==7){events[j*3]=(unsigned)(c?native_connected_wifi_click:native_wifi_click);changed=1;}
  if(changed)protect_tap(item);
 }
 }
}
static void prepare_wifi_rows(void){
 LvObj *p=LV_GLOBAL(0x8de024);
 if(p&&lv_valid(p)&&p!=wifi_parent&&child_count(p)>=2){
  unsigned n=child_count(p);original_row(p,"已保存 Wi-Fi",20,open_clicked,0);
  /* Native Wi-Fi refresh uses fixed stock child indexes. Append after all
   * stock objects; never insert before the connected/scan containers. */
  wifi_saved_row=child(p,n);if(!wifi_saved_row)return;
  protect_tap(wifi_saved_row);
  wifi_parent=p;lv_add_cb(p,deleted,DISC_LV_DELETE,0);
 }
 if(p&&p==wifi_parent&&wifi_saved_row&&lv_valid(p)){
  /* Visual order is independent from the stock object's child indexes.
   * Native refresh keeps using its original title/switch/container pointers.
   * Read numeric height style 4 (verified lv_obj_set_height), plus HIDDEN at
   * +0x24 (verified lv_obj_add/clear_flag); do not edit either raw field. */
  FN(0x4f6f10,void (*)(LvObj *,int,unsigned))(p,0,0);
  int y=0;unsigned count=child_count(p);
  for(unsigned j=0;j<count;j++){
   LvObj *item=j==2?wifi_saved_row:child(p,j<2?j:j-1);
   if(!item||(*(unsigned *)((char *)item+0x24)&DISC_LV_HIDDEN))continue;
   int h[3]={0};int has=FN(0x4f5ea4,int (*)(LvObj *,unsigned,int *,unsigned))(item,4,h,0);
   int height=has==1&&h[0]>=0&&h[0]<10000?h[0]:*(int *)((char *)item+0x20)-*(int *)((char *)item+0x18)+1;
   if(height<0||height>10000)continue;
   lv_align(item,2,0,y);y+=height;
  }
  bind_wifi_scan_rows();
 }
}
/* Verified V2.57 default display +4 refresh timer; timer +8 is its callback.
 * Prepare native extensions under the existing UI lock before LVGL computes
 * layout/draws. No file polling or debug-service work runs in this callback. */
#include "../mobile-remote/native_eq.h"
#include "../mobile-remote/native_ui.h"
#include "../usb-tools/toolbox_ui.h"
#include "../library-scope/scanner_hook.h"
#include "../library-scope/scope_ui.h"
#include "../display-settings/whitepoint_control.h"
#include "../display-settings/display_ui.h"
#include "../player-utilities/settings_ui.h"
#include "../mode-transition/transition_ui.h"
static void online_frame_tick(void);
#include "frame_probe.h"
static void before_display_refresh(void *timer) {
 unsigned probe_start=frame_probe_begin();
 if(!preparing_frame){preparing_frame=1;prepare_system_rows();prepare_wifi_rows();remote_bt_prepare();toolbox_prepare();library_scope_prepare();if(bt_prepare)bt_prepare();preparing_frame=0;}
 toolbox_frame_tick();
 online_frame_tick();
 mode_transition_prepare();
 real_display_refresh(timer);
 if(bt_rendered)bt_rendered();else if(bt_after)bt_after();hw_bridge_pump();
 frame_probe_end(probe_start,timer);
}
static void bind_display_refresh(void) {
 void *disp=lv_disp_default();if(!disp)return;
 void *timer=*(void **)((char *)disp+4);if(!timer||timer==display_timer)return;
 void (**callback)(void *)=(void (**)(void *))((char *)timer+8);
 if(*callback!=FN(0x4fa364,void (*)(void *)))return;
 real_display_refresh=*callback;display_timer=timer;*callback=before_display_refresh;
 note("native pre-draw menu preparation attached\n");
}
#include "../dlna/ui.h"
#include "../online-music/ui.h"
static void poll(void) {
 frame_probe_poll();
 remote_eq_tick();remote_bt_prepare();remote_bt_refresh();
 dlna_poll();
 online_poll();
 toolbox_poll();
 library_scope_poll();
 ds_whitepoint_poll();
 display_settings_poll();
 pu_poll();
 bind_display_refresh();
 bt_bridge_tick();
 bt_wifi_tick();
 wifi_request();
 prepare_system_rows();
 prepare_wifi_rows();
 if(view!=9)unlink("/tmp/disc-runtime/wifi-secret");
 if(pending_back){int parent=view==4?2:view==2?1:view==6?5:view==7?12:view==9?8:view==10?9:view==11?5:view==12?6:view==13?12:0;pending_back=0;close_debug(1);if(parent)make_extra(parent);}
 if(debug_page&&lv_valid(debug_page)&&lv_active_screen(lv_disp_default())!=debug_page)close_debug(0);
 if(pending_open){pending_open=0;if(debug_origin&&lv_valid(debug_origin)){if(debug_page)close_debug(1);if(requested_view)make_extra(requested_view);else make_debug();}}
 if(debug_page&&lv_valid(debug_page))refresh();
 if(!access("/tmp/disc-settings.ui-query",0)){
  unlink("/tmp/disc-settings.ui-query");char b[300];LvObj *active=lv_active_screen(lv_disp_default());
  int n=snprintf(b,sizeof(b),"view=%d\nwifi_active=%d\nsystem_active=%d\nnative_wifi=%d\nsecret_visible=%d\n",debug_page&&active==debug_page?view:-1,active==wifi_parent,active==page,*(volatile unsigned char *)0x8e171d,debug_page&&view==9?secret_visible:0);
  int f=open("/tmp/disc-settings.ui-state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}
 }
 /* Explicit one-shot diagnostic request, never opened automatically at boot. */
 if(!access("/usr/data/disc-settings.open",0)){
  unlink("/usr/data/disc-settings.open");FN(0x457e6c,void (*)(void))();
 }
}
__attribute__((noinline,visibility("hidden"))) int disc_usleep(unsigned usec) {
 unsigned caller=(unsigned)__builtin_return_address(0);
 if(caller==0x40397c&&usec==5000&&((bt_fast&&bt_fast())||hw_active))usec=1000;
 int r=real_usleep(usec);
 if(caller==0x40397c&&!reentrant){
  static int first_loop;if(!first_loop){first_loop=1;startup_trace("ui.first-loop");}
  unsigned t=milliseconds();int bt_due=bt_poll&&(!bt_poll_at||t-bt_poll_at>=16),hw_due=hw_active&&(!hw_pump_at||t-hw_pump_at>=16),settings_due=!settings_poll_at||t-settings_poll_at>=200;
  /* Keep the fast native timer loop, but don't acquire its lock and poll
   * extensions for every 1ms wakeup. Work deadlines use elapsed time. */
  if(bt_due||hw_due||settings_due){
  if(!pthread_mutex_trylock((void *)0x8d9e70)){
   reentrant=1;
   /* Native main has initialized LVGL, its mutex, display and loader before
    * this verified call site. Constructors only attach the sleep hook. */
   if(startup_claim(&startup_gate,caller,lv_disp_default()!=0,1)){
    startup_trace("ui.module-load.begin");
    enable_wifi_capture();startup_trace("ui.environment-ready");
    hw_bridge_refresh();startup_trace("ui.hardware-checked");
    bt_bridge_refresh();startup_trace("ui.bluetooth-checked");
    startup_complete(&startup_gate);startup_trace("ui.modules-ready");
   }
   if(bt_due){bt_poll_at=t;bt_poll();if(bt_after)bt_after();}
   if(bt_due||hw_due){hw_pump_at=t;hw_bridge_pump();}
   if(settings_due){settings_poll_at=t;heartbeat();poll();}
   reentrant=0;pthread_mutex_unlock((void *)0x8d9e70);
  }
  }
 }
 return r;
}
#include "../bluetooth-meter/player_controls.h"
#include "../bluetooth-meter/bt_playback.h"
#include "../embedded-cover/player_binding.h"
#include "tf_mount_guard.h"
__attribute__((constructor)) static void start(void) {
 if(access("/usr/data/disc-settings.enabled",0))return;
 if(!startup_is_app())return;
 startup_trace("constructor.enter");
 if(!identity()){if(player_identity()){startup_trace("player.identity-ok");enable_wifi_capture();diag_start();pc_install();library_scope_player_install();if(pc_real_usleep){bp_install();cover_install();}startup_trace("player.ready");}return;}
 startup_trace("ui.identity-ok");
 if(!module_cache_sync("/usr/data/disc-settings/module-ui.so")){startup_trace("ui.cache.failed");return;}
 startup_trace("ui.cache.ok");
 remote_eq_install();
 if(!tf_guard_install()){startup_trace("ui.tf-guard.failed");return;}
 startup_trace("ui.tf-guard.ready");
 diag_start();
 real_usleep=(int (*)(unsigned))dlsym((void *)0,"usleep");
 if(!real_usleep){note("usleep resolution failed\n");return;}
 *(volatile unsigned *)USLEEP_GOT=(unsigned)disc_usleep;
 startup_gate.constructor_ready=1;
 startup_trace("ui.hook-ready");
 heartbeat();
 note("V2.57 USB settings extension loaded\n");
 startup_trace("ui.ready");
}
