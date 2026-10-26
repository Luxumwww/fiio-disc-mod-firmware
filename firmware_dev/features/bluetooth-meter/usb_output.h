/* Native output command only. Input mode and PCM ownership remain native. */
extern int ioctl(int,unsigned,...),unlink(const char *);
static LvObj *usb_button,*usb_caption,*usb_glyph;
static int usb_phase,usb_previous=6,usb_actual=-1,usb_route=-1;
static int usb_pcm_open,usb_player_state;
static unsigned usb_at,usb_checked;static char usb_text[64];
static unsigned usb_retry_at,usb_retries;
static unsigned usb_owner_mode=99;
#include "../usb-output-reconnect/usb_port_state.h"
static unsigned usb_requested_at,usb_host_at,usb_host_checked,usb_source_retries;
static int usb_was_present,usb_host_ready,usb_attach_seen,usb_sink_done;
static int hastext(const char *b,const char *s){for(unsigned i=0;b[i];i++){unsigned j=0;while(s[j]&&b[i+j]&&s[j]==b[i+j])j++;if(!s[j])return 1;}return 0;}
static void usb_command(unsigned output){
 FN(0x48b28c,void (*)(const char *,unsigned,const char *))((const char *)0x69dcf0,output,0);
 *(unsigned char *)0x8e1728=output;
 /* The reviewed PLAYER callback switches only its native output track.
  * Do not repeat SET_INPUT_MODE: it tears down the BT connection/server. */
}
static int usb_token(const char *path,char *b,unsigned size){
 int n=textread(path,b,size);while(n>0&&(b[n-1]=='\n'||b[n-1]=='\r'))b[--n]=0;return n;
}
static int usb_role_is(int host){char b[24];return usb_token("/sys/class/usb_role/13500000.otg_new-role-switch/role",b,sizeof(b))&&!strcmp(b,host?"host":"device");}
static int usb_role_write(int host){
 if(usb_role_is(host))return 1;
 int f=open("/sys/class/usb_role/13500000.otg_new-role-switch/role",1);if(f<0)return 0;
 const char *role=host?"host":"device";unsigned len=host?4:6;int r=write(f,role,len);close(f);return r==(int)len&&usb_role_is(host);
}
static int usb_type_request(int host){
 int f=open("/dev/aw35615",2);if(f<0)return 0;
 int r=ioctl(f,host?0x20004e27u:0x20004e26u,0);close(f);if(host)usb_sink_done=0;return r>=0;
}
static int usb_role(int host){
 /* Source requests reset Type-C negotiation even if the ioctl returns zero.
  * Startup uses the native player's one request, followed by readback. */
 if(!host&&usb_sink_done&&usb_role_is(0))return 1;
 int ready=usb_type_request(host)&&usb_role_write(host);if(ready)usb_sink_done=!host;return ready;
}
static int usb_port_released(unsigned t){
 char b[768];
 if(!textread("/tmp/disc-runtime/state",b,sizeof(b))||!usb_port_state_ready(b,usb_owner_mode,usb_requested_at,t))return 0;
 if(!access("/dev/usb-ffs/adb/ep0",0)||!access("/dev/android_adb",0))return 0;
 int f=open("/sys/kernel/config/usb_gadget/disc_adb/UDC",0);if(f<0)return 1;
 int n=read(f,b,sizeof(b)-1);close(f);if(n<0)return 0;
 for(int i=0;i<n;i++)if(b[i]!='\n'&&b[i]!='\r'&&b[i]!=' '&&b[i]!='\t')return 0;
 return 1;
}
static int usb_typec_state(void){
 char b[64];int state=-1;
 return usb_token("/sys/bus/i2c/devices/2-0022/control/typec_state",b,sizeof(b))&&sscanf(b,"%d",&state)==1?state:-1;
}
static int usb_host_poll(unsigned t,int present){
 /* Let echo_get_otg_status observe output3 first (native 330ms worker).
  * A second immediate source ioctl used to restart that same negotiation. */
 if(t-usb_host_at<750)return 0;
 if(t-usb_host_checked>=1000){
  usb_host_checked=t;
  if(!usb_port_released(t))return 0;
  int state=usb_typec_state();if(state==4||state==5||state==6)usb_attach_seen=1;
  /* port_type remains drp after forced source and cannot confirm power.
   * An idle, unplugged port is allowed to stay Unattached indefinitely.
   * Retry only after an observed attachment fails back to Unattached/sink;
   * never reset a negotiation that is still AttachWaitSource. */
  if(!present&&usb_attach_seen&&(state==2||state==4)&&t-usb_host_at>=2500&&usb_source_retries<2){
   usb_source_retries++;usb_attach_seen=0;usb_type_request(1);usb_host_at=t;
  }
  if(!usb_role_write(1))return 0;
 }
 return usb_role_is(1);
}
static void usb_caption_set(const char *s,int on){
 if(strcmp(s,usb_text)){snprintf(usb_text,sizeof(usb_text),"%s",s);if(usb_caption)lv_text(usb_caption,s);}
 if(usb_glyph)src(usb_glyph,on?&usb_icon_1:&usb_icon_0);
}
/* Explicit debug ON owns the physical port until the next audio request.
 * Runs only on the UI thread; native commands retain PCM/profile ownership. */
static void usb_poll(unsigned);
static unsigned usb_debug_checked,usb_debug_attempts;static char usb_debug_done[96],usb_debug_pending[96];
static void usb_debug_poll(unsigned t){
 if(t-usb_debug_checked<250)return;usb_debug_checked=t;
 char b[96];if(!textread("/tmp/disc-runtime/debug-usb-priority",b,sizeof(b)))return;
 if(!strcmp(b,usb_debug_done))return;
 if(strcmp(b,usb_debug_pending)){snprintf(usb_debug_pending,sizeof(usb_debug_pending),"%s",b);usb_debug_attempts=0;}
 unsigned pid,mode,stamp;if(sscanf(b,"%u %u %u",&pid,&mode,&stamp)!=3||pid!=(unsigned)getpid()||mode!=*(unsigned *)0x8e16d4)return;
 if(t/1000<stamp||t/1000-stamp>15)return;
 discover(t);unsigned config,p,route;
 if(!copy(0x83a758,&config,4)||!copy(0x83dfe4,&p,4)||!copy(p+0x58,&route,4))return;
 if((mode==0||mode==3)&&(((config>>16)&255)==3||usb_phase)){
  if(usb_phase!=4){usb_previous=usb_previous==4?4:6;usb_command(usb_previous);usb_phase=4;usb_at=t;usb_caption_set("USB 正在关闭",0);}
  usb_poll(t);return;
 }
 if(usb_debug_attempts++>=3||!usb_role(0))return;
 unlink("/tmp/disc-runtime/bt-usb.request");usb_phase=0;usb_owner_mode=99;usb_caption_set("USB 输出 关",0);
 unsigned n=0;while(b[n])n++;int fd=open("/tmp/disc-runtime/debug-usb-priority.ack.new",1|0x100|0x200,0600);
 if(fd>=0){int ok=write(fd,b,n)==(int)n;close(fd);if(ok&&!rename("/tmp/disc-runtime/debug-usb-priority.ack.new","/tmp/disc-runtime/debug-usb-priority.ack")){snprintf(usb_debug_done,sizeof(usb_debug_done),"%s",b);usb_debug_attempts=0;}}
}
static void usb_click(void *e){
 unsigned mode=*(unsigned *)0x8e16d4;if(pure_editing||!control_clicked(e)||(mode!=0&&mode!=3))return;
 unlink("/tmp/disc-runtime/debug-usb-priority");unlink("/tmp/disc-runtime/debug-usb-priority.ack");usb_debug_attempts=0;
 if(mode==0)discover(now());
 if(usb_phase){usb_command(usb_previous);usb_phase=4;usb_at=now();usb_caption_set("USB 正在关闭",0);return;}
 char b[768];
 unsigned config;if(!copy(0x83a758,&config,4)){usb_caption_set("USB 暂不可用",0);return;}
 usb_previous=(config>>16)&255;int close_existing=mode==0&&usb_previous==3;if(close_existing)usb_previous=6;
 if(usb_previous!=4&&usb_previous!=6){usb_caption_set("USB 输出占用",0);return;}
 int f=open("/tmp/disc-runtime/bt-usb.request.new",1|0x100|0x200,0600);if(f<0)return;
 int n=snprintf(b,sizeof(b),"%d\n",getpid());int r=write(f,b,n);close(f);if(r!=n||rename("/tmp/disc-runtime/bt-usb.request.new","/tmp/disc-runtime/bt-usb.request"))return;
 usb_owner_mode=mode;usb_phase=1;usb_at=usb_requested_at=now();usb_retry_at=usb_retries=0;usb_source_retries=0;usb_host_ready=usb_was_present=usb_attach_seen=usb_sink_done=0;usb_caption_set("USB 准备中",1);
 if(close_existing){usb_command(usb_previous);usb_phase=4;usb_caption_set("USB 正在关闭",0);}
}
static void usb_poll(unsigned t){
 if(t-usb_checked<250)return;usb_checked=t;
 unsigned config,p,route;if(copy(0x83a758,&config,4))usb_actual=(config>>16)&255;else usb_actual=-1;
 if(copy(0x83dfe4,&p,4)&&copy(p+0x58,&route,4))usb_route=(int)route;else usb_route=-1;
 unsigned ready=0,handle=0,player=0,state=0;usb_pcm_open=copy(0x83dfe4,&p,4)&&copy(p+3*0x54+0x68,&ready,4)&&copy(p+3*0x54+0x74,&handle,4)&&ready&&handle;
 usb_player_state=copy(0x83dfc4,&player,4)&&copy(player+0x48,&state,4)?(int)state:0;
 if(!usb_phase)return;
 unsigned mode=*(unsigned *)0x8e16d4;
 if((mode!=0&&mode!=3)||mode!=usb_owner_mode){unlink("/tmp/disc-runtime/bt-usb.request");usb_phase=0;usb_owner_mode=99;usb_caption_set("USB 输出 关",0);return;}
 if(!active&&usb_phase!=4){usb_command(usb_previous);usb_phase=4;usb_at=t;}
 if(usb_phase==1){
  if(usb_port_released(t)){
   int state=usb_typec_state();usb_attach_seen=state==4||state==5||state==6;
   usb_command(3);usb_phase=2;usb_at=usb_host_at=t;usb_host_checked=t;
  }else if(t-usb_at>15000){unlink("/tmp/disc-runtime/bt-usb.request");usb_phase=0;usb_caption_set("USB 准备失败",0);}
 }else if(usb_phase==2){
  if(usb_actual==3){if(usb_host_poll(t,0)){usb_phase=3;usb_caption_set("USB 待连接",1);}else if(t-usb_at>10000){usb_phase=5;usb_caption_set("USB 主机失败",1);}}
  else if(t-usb_at>10000){usb_phase=5;usb_caption_set("USB 切换失败",1);}
 }else if(usb_phase==3){
  char b[2048];int present=textread("/proc/asound/cards",b,sizeof(b))&&hastext(b,"USB-Audio");
  if(present!=usb_was_present){usb_retry_at=t;usb_retries=0;if(!present){usb_source_retries=0;usb_attach_seen=0;usb_host_at=t;}usb_was_present=present;}
  usb_host_ready=usb_host_poll(t,present);
  unsigned pid=0,updated=0;int fresh=textread("/tmp/disc-runtime/player-usb.state",b,sizeof(b))&&usb_state_unsigned(b,"pid",&pid)&&pid==(unsigned)playerpid&&usb_state_unsigned(b,"updated",&updated)&&updated<=t/1000&&t/1000-updated<=3;
  int routed=usb_host_ready&&usb_actual==3&&usb_route==3&&fresh&&usb_state_is(b,"usb_ready","1");
  int idle=usb_host_ready&&usb_actual==3&&usb_route==3&&fresh&&usb_state_is(b,"usb_present","1")&&usb_state_is(b,"usb_recovering","0")&&usb_state_is(b,"usb_error","0")&&(usb_state_is(b,"player_state","0")||usb_state_is(b,"player_state","3"));
  if(present&&!routed&&!idle&&t-usb_retry_at>=2000&&usb_retries<3){usb_retry_at=t;usb_retries++;usb_command(3);}
  usb_caption_set(present&&routed?"USB 输出 开":present&&idle?"USB 已连接":present?"USB 正在连接":usb_host_ready?"USB 待连接":"USB 正在启动",1);
 }else if(usb_phase==4){
  if(usb_actual==usb_previous&&usb_route==usb_previous){if(usb_role(0)){unlink("/tmp/disc-runtime/bt-usb.request");usb_phase=0;usb_caption_set("USB 输出 关",0);}else {usb_phase=5;usb_caption_set("USB 关闭失败",0);}}
  else if(t-usb_at>10000){usb_phase=5;usb_caption_set("USB 关闭失败",0);}
 }
}
