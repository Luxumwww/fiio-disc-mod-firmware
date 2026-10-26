static LvObj *remote_bt_parent,*remote_bt_row,*remote_bt_switch;
static int remote_bt_on=-1;
static void remote_bt_deleted(void *e){if(lv_event_target(e)==remote_bt_parent){remote_bt_parent=remote_bt_row=remote_bt_switch=0;remote_bt_on=-1;}}
static void remote_bt_click(void *e){
 if(!stationary_clicked(e)||!access("/tmp/disc-runtime/module-command",0))return;
 int on=!access("/usr/data/disc-runtime/components/disc-remote-control/enabled",0)&&access("/usr/data/disc-runtime/components/disc-remote-control/disabled",0);
 char data[64];int n=snprintf(data,sizeof(data),"%s|disc-remote-control\n",on?"disable":"enable");int f=open("/tmp/disc-runtime/module-command.new",1|0x100|0x200,0600);if(f<0)return;int ok=write(f,data,n)==n;close(f);if(ok)rename("/tmp/disc-runtime/module-command.new","/tmp/disc-runtime/module-command");
}
static void remote_bt_prepare(void){
 if(access("/usr/data/disc-runtime/components/disc-remote-control/current/ENTRY",0))return;
 LvObj *p=lv_active_screen(lv_disp_default());if(!p||!lv_valid(p))return;
 if(p!=remote_bt_parent){
  if(child_count(p)<6)return;LvObj *title=child(child(p,0),0);if(!title||*(unsigned *)title!=0x7ffd00)return;
  const char *text=*(const char **)((char *)title+44);if(!text||(strcmp(text,"蓝牙")&&strcmp(text,"Bluetooth")))return;
  LvObj *row=child(p,1);unsigned *spec=*(unsigned **)((char *)row+8);if(!spec)return;unsigned count=spec[8]>>16,*events=(unsigned *)spec[3];int native=0;
  if(count>32)return;for(unsigned i=0;events&&i<count;i++)if(events[i*3]==0x459470&&(events[i*3+2]&255)==7)native=1;if(!native)return;
  /* Reproduce the Bluetooth power row's native constructor (0x45a2b0-
   * 0x45a49c), including its switch constructor, colors and text style.
   * The generic submenu helper has different padding/font and an arrow. */
  remote_bt_row=lv_create(p);if(!remote_bt_row)return;
  lv_size(remote_bt_row,360,60);lv_border(remote_bt_row,2,0);
  FN(0x4f6c20,void (*)(LvObj *,unsigned,unsigned))(remote_bt_row,0xff000000,0);
  bg_opa(remote_bt_row,0,0);pad_left(remote_bt_row,5,0);pad_right(remote_bt_row,5,0);pad_top(remote_bt_row,5,0);pad_bottom(remote_bt_row,5,0);
  FN(0x4f0268,void (*)(LvObj *,int))(remote_bt_row,0);lv_add_flag(remote_bt_row,0x4000);protect_tap(remote_bt_row);lv_add_cb(remote_bt_row,remote_bt_click,7,0);
  remote_bt_switch=FN(0x533960,LvObj *(*)(LvObj *))(remote_bt_row);
  if(remote_bt_switch){
   FN(0x4eb4a4,void (*)(LvObj *,int,int))(remote_bt_switch,20,6);
   lv_size(remote_bt_switch,60,33);lv_align(remote_bt_switch,9,70,0);
   lv_bg(remote_bt_switch,0xffff0000,0x20001);lv_bg(remote_bt_switch,0xff565656,0);
   lv_clear_flag(remote_bt_switch,2|0x2000);protect_tap(remote_bt_switch);lv_add_cb(remote_bt_switch,remote_bt_click,7,0);
  }
  LvObj *label=lv_label(remote_bt_row);lv_text(label,"BLE 蓝牙控制");lv_align(label,9,-80,0);lv_color(label,0xff9d9d9d,0);lv_add_style(label,(void *)0x83ac54,0);
  remote_bt_parent=p;remote_bt_on=-1;lv_add_cb(p,remote_bt_deleted,33,0);
 }
 /* Keep native child indexes intact; arrange the new row directly after the
  * Bluetooth power row, like the saved-network extension of the Wi-Fi page. */
 if(p==remote_bt_parent&&remote_bt_row){FN(0x4f6f10,void (*)(LvObj *,int,unsigned))(p,0,0);int y=0;unsigned count=child_count(p);
  for(unsigned j=0;j<count;j++){LvObj *item=j==2?remote_bt_row:child(p,j<2?j:j-1);if(!item||(*(unsigned *)((char *)item+0x24)&1))continue;
   int h[3]={0};int has=FN(0x4f5ea4,int (*)(LvObj *,unsigned,int *,unsigned))(item,4,h,0);int height=has==1&&h[0]>=0&&h[0]<10000?h[0]:*(int *)((char *)item+0x20)-*(int *)((char *)item+0x18)+1;if(height<0||height>10000)continue;lv_align(item,2,0,y);y+=height;
  }
 }
}
static void remote_bt_refresh(void){
 if(!remote_bt_parent||!lv_valid(remote_bt_parent))return;
 int on=!access("/usr/data/disc-runtime/components/disc-remote-control/enabled",0)&&access("/usr/data/disc-runtime/components/disc-remote-control/disabled",0);
 if(remote_bt_switch){if(on)FN(0x4e98f8,void (*)(LvObj *,unsigned))(remote_bt_switch,1);else FN(0x4e9918,void (*)(LvObj *,unsigned))(remote_bt_switch,1);remote_bt_on=on;}
}
