/* Runs on the original UI thread; all slow operations are backend requests. */
static LvObj *personal_info,*personal_action,*personal_rows[100],*secret_text,*secret_toggle;
static char backup_ids[100][40],selected_backup[40],selected_wifi[72],selected_ssid[100],selected_kind[8];
static const void *secret_font;
static struct {char key[72],ssid[100],kind[8];} wifi_items[100];
static int personal_count,secret_visible;
static int personal_busy(void){return !access("/tmp/disc-runtime/personal-busy",0)||!access("/tmp/disc-runtime/personal-command",0);}
static void personal_send(const char *action,const char *arg){
 if(personal_busy())return;
 char b[160];int n=snprintf(b,sizeof(b),"%s|%s\n",action,arg?arg:"");
 int f=open("/tmp/disc-runtime/personal-command.new",1|0x100|0x200,0600);if(f<0)return;
 int r=write(f,b,n);close(f);if(r==n)rename("/tmp/disc-runtime/personal-command.new","/tmp/disc-runtime/personal-command");
}
static void backup_clicked(void *e){if(stationary_clicked(e)&&!personal_busy()){requested_view=11;pending_open=1;}}
static void backup_confirm_clicked(void *e){if(stationary_clicked(e))personal_send("backup","");}
static void restore_list_clicked(void *e){if(stationary_clicked(e)&&!personal_busy()){requested_view=6;pending_open=1;}}
static void restore_detail_clicked(void *e){if(stationary_clicked(e)&&!personal_busy()){requested_view=7;pending_open=1;}}
static void backup_delete_clicked(void *e){if(stationary_clicked(e)&&!personal_busy()){requested_view=13;pending_open=1;}}
static void backup_delete_confirm(void *e){if(stationary_clicked(e))personal_send("backup-delete",selected_backup);}
static void restore_confirm_clicked(void *e){
 if(!stationary_clicked(e))return;
 if(tap_duration<1000){lv_text(personal_info,"请长按确认 1 秒后松开\n尚未开始恢复");return;}
 personal_send("restore",selected_backup);
}
static void forget_confirm_clicked(void *e){if(stationary_clicked(e))personal_send("forget",selected_wifi);}
static void forget_clicked(void *e){if(stationary_clicked(e)&&!personal_busy()){requested_view=10;pending_open=1;}}
static void connect_saved_clicked(void *e){if(stationary_clicked(e)){secret_visible=0;unlink("/tmp/disc-runtime/wifi-secret");FN(0x4f6e10,void (*)(LvObj *,const void *,unsigned))(secret_text,secret_font,0);lv_text(secret_text,"密码：已隐藏");lv_text(secret_toggle,"显示密码");personal_send("connect",selected_wifi);}}
static void secret_clicked(void *e){
 if(!stationary_clicked(e)||personal_busy())return;
 if(secret_visible){secret_visible=0;unlink("/tmp/disc-runtime/wifi-secret");FN(0x4f6e10,void (*)(LvObj *,const void *,unsigned))(secret_text,secret_font,0);lv_text(secret_text,"密码：已隐藏");lv_text(secret_toggle,"显示密码");}
 else {secret_visible=1;personal_send("secret",selected_wifi);lv_text(secret_toggle,"隐藏密码");lv_text(secret_text,"正在读取密码");}
}
static int unhex(char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;}
static void read_personal_list(int kind){
 static char b[18000];int r=read_data(kind==6?"/tmp/disc-runtime/backup-list":"/tmp/disc-runtime/wifi-list",b,sizeof(b));personal_count=0;
 for(int i=0;i<r&&personal_count<100;){
  int start=i;while(i<r&&b[i]!='\n')i++;b[i++]=0;
  if(kind==6){if(i-start<16||i-start>=40)continue;snprintf(backup_ids[personal_count++],40,"%s",b+start);}
  else {
   char *key=b+start,*hex=0,*type=0;
   for(int j=start;j<i-1;j++)if(b[j]=='|'){b[j]=0;if(!hex)hex=b+j+1;else {type=b+j+1;break;}}
   if(!hex||!type)continue;int n=0;char ssid[100];
   while(hex[0]&&hex[1]&&n<32){int a=unhex(hex[0]),z=unhex(hex[1]);if(a<0||z<0)break;int c=a*16+z;ssid[n++]=c<32||c==127?'?':c;hex+=2;}
   ssid[n]=0;int k=personal_count++;snprintf(wifi_items[k].key,72,"%s",key);snprintf(wifi_items[k].ssid,100,"%s",ssid);snprintf(wifi_items[k].kind,8,"%s",type);
  }
 }
}
static void personal_select(void *e){
 if(!stationary_clicked(e)||personal_busy())return;
 LvObj *target=lv_event_target(e);
 for(int k=0;k<personal_count;k++)if(target==personal_rows[k]){
  if(view==6){snprintf(selected_backup,40,"%s",backup_ids[k]);requested_view=12;}
  else {snprintf(selected_wifi,72,"%s",wifi_items[k].key);snprintf(selected_ssid,100,"%s",wifi_items[k].ssid);snprintf(selected_kind,8,"%s",wifi_items[k].kind);requested_view=9;}
  pending_open=1;break;
 }
}
/* Stock label helpers use 0x200007d1 as LV_SIZE_CONTENT (32-bit coordinates). */
static void personal_wrap(LvObj *l,int width){lv_size(l,width,0x200007d1);FN(0x52f4f0,void (*)(LvObj *,int))(l,0);}
static void refresh_personal(void){
 char b[256];
 if(view==5||view==7||view==10||view==11||view==13){
  int n=read_data("/tmp/disc-runtime/personal-result",b,sizeof(b));while(n>0&&(b[n-1]=='\n'||b[n-1]=='\r'))b[--n]=0;
  if(view==5){
   if(n>=15&&!memcmp(b,"备份完成：",15))lv_text(personal_info,"备份完成");
   else lv_text(personal_info,n?b:"");
  }
  else if(personal_busy())lv_text(personal_info,n?b:"正在处理，请勿关机");
  else if(view==10&&n){lv_text(personal_info,b);if(n>=18&&!memcmp(b,"已删除该 Wi-Fi",18)){requested_view=8;pending_open=1;}}
  else if(view==11&&n){lv_text(personal_info,b);if(n>=15&&!memcmp(b,"备份完成：",15)){requested_view=5;pending_open=1;}}
  else if(view==13&&n){lv_text(personal_info,b);if(n>=15&&!memcmp(b,"备份已删除",15)){requested_view=6;pending_open=1;}}
  if(personal_action)lv_text(personal_action,personal_busy()?"处理中":view==7?"长按确认恢复":view==11?"确认":"确认删除");
 }else if(view==9){
  int status_n=read_data("/tmp/disc-runtime/personal-result",b,sizeof(b));while(status_n>0&&(b[status_n-1]=='\n'||b[status_n-1]=='\r'))b[--status_n]=0;
  if(personal_info)lv_text(personal_info,personal_busy()?"正在处理，请稍候":status_n&&memcmp(b,"密码",6)?b:"");
  if(!secret_visible)return;
  int n=read_data("/tmp/disc-runtime/wifi-secret",b,sizeof(b));int j=0;while(j<n&&b[j]!='\n')j++;
  if(j<n){b[j++]=0;if(!strcmp(b,selected_wifi)){
   int end=n;while(end>j&&(b[end-1]=='\n'||b[end-1]=='\r'))b[--end]=0;
   FN(0x4f6e10,void (*)(LvObj *,const void *,unsigned))(secret_text,!strcmp(selected_kind,"pass")?(const void *)(end-j>40?0x7c6bd0:0x7ca120):secret_font,0);
   lv_text(secret_text,b+j);
  }}
 else if(!personal_busy()){secret_visible=0;char error[160];read_data("/tmp/disc-runtime/personal-result",error,sizeof(error));FN(0x4f6e10,void (*)(LvObj *,const void *,unsigned))(secret_text,secret_font,0);lv_text(secret_text,error[0]?error:"密码：已隐藏");lv_text(secret_toggle,"显示密码");}
  volatile char *wipe=b;for(unsigned k=0;k<sizeof(b);k++)wipe[k]=0;
 }
}
static void personal_row(const char *name,int y,void (*callback)(void *)){
 int layout[3]={44,180,0};unsigned n=child_count(debug_page);original_row(debug_page,name,0,callback,layout);
 LvObj *r=child(debug_page,n);lv_size(r,360,52);pad_top(r,0,0);pad_bottom(r,0,0);lv_align(r,1,0,y);
 protect_tap(r);
 if(callback==backup_clicked){LvObj *arrow=child(r,1);if(arrow)lv_add_flag(arrow,DISC_LV_HIDDEN);}
}
static void personal_flow(void){
 FN(0x4f0268,void (*)(LvObj *,int))(debug_page,0);FN(0x4f02c8,void (*)(LvObj *,int))(debug_page,12);
 FN(0x5030a0,void (*)(LvObj *,int))(debug_page,1);FN(0x5030e4,void (*)(LvObj *,int,int,int))(debug_page,0,0,2);
 LvObj *top=lv_create(debug_page);lv_size(top,360,40);lv_border(top,0,0);lv_bg(top,0xff000000,0);lv_clear_flag(top,DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);move_index(top,0);
}
static LvObj *flow_text(const char *s,int height,unsigned color){
 LvObj *box=lv_create(debug_page);lv_size(box,360,height);lv_border(box,0,0);lv_bg(box,0xff000000,0);lv_clear_flag(box,DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);
 LvObj *l=text_at(box,s,0,color);personal_wrap(l,240);return l;
}
static void make_personal(int kind){
 view=kind;secret_visible=0;personal_action=secret_text=secret_toggle=personal_info=0;
 debug_page=lv_create(0);if(!debug_page)return;
 screen_register(debug_page);lv_add_cb(debug_page,deleted,DISC_LV_DELETE,0);
 lv_size(debug_page,360,360);lv_border(debug_page,0,0);lv_bg(debug_page,0xff000000,0);
 pad_top(debug_page,0,0);pad_bottom(debug_page,0,0);pad_left(debug_page,0,0);pad_right(debug_page,0,0);
 int flow=kind==6||kind==8;
 if(!flow)lv_clear_flag(debug_page,DISC_LV_SCROLLABLE);
 const char *title=kind==5?"数据备份与恢复":kind==6?"恢复数据":kind==7?"确认恢复":kind==8?"已保存 Wi-Fi":kind==9?"网络详情":kind==11?"备份数据":kind==12?"备份信息":kind==13?"删除备份":"删除 Wi-Fi";
 native_title(debug_page,title,0xb4fb);lv_align(child(debug_page,0),1,0,44);lv_color(child(debug_page,0),0xff54baff,0);
 lv_add_cb(debug_page,back_swipe,1,0);lv_add_cb(debug_page,back_swipe,2,0);lv_add_cb(debug_page,back_swipe,8,0);
 if(flow)personal_flow();
 if(kind==5){
  personal_row("备份数据",94,backup_clicked);personal_row("恢复数据",160,restore_list_clicked);
  personal_info=text_at(debug_page,"",70,0xffaaaaaa);personal_wrap(personal_info,240);
  lv_size(personal_info,240,52);FN(0x52f4f0,void (*)(LvObj *,int))(personal_info,4);lv_align(personal_info,9,0,70);
  if(!personal_busy())unlink("/tmp/disc-runtime/personal-result");
 }else if(kind==6||kind==8){
  read_personal_list(kind);
  if(!personal_count)flow_text(kind==6?"TF 卡未挂载或暂无备份":"暂无已保存网络",120,0xffaaaaaa);
  for(int k=0;k<personal_count;k++){
   char b[160];if(kind==6){char *id=backup_ids[k];snprintf(b,sizeof(b),"%.4s-%.2s-%.2s\n%.2s:%.2s:%.2s%s",id,id+4,id+6,id+9,id+11,id+13,id+15);}
   else snprintf(b,sizeof(b),"%s",wifi_items[k].ssid);
   int layout[3]={44,180,0};unsigned n=child_count(debug_page);original_row(debug_page,b,k,personal_select,layout);
   personal_rows[k]=child(debug_page,n);lv_size(personal_rows[k],360,kind==6?76:64);
   protect_tap(personal_rows[k]);
   FN(0x52f4f0,void (*)(LvObj *,int))(child(personal_rows[k],0),3);
  }
 }else if(kind==7){
  char b[256];snprintf(b,sizeof(b),"%s\n替换个人数据并重启\n先自动备份当前数据",selected_backup);
  personal_info=text_at(debug_page,b,-12,0xffdddddd);personal_wrap(personal_info,250);
  personal_action=child(button_at(debug_page,"长按确认恢复",0,76,210,restore_confirm_clicked),0);
 }else if(kind==9){
  LvObj *ssid=text_at(debug_page,selected_ssid,-72,0xffdddddd);personal_wrap(ssid,260);lv_size(ssid,260,52);
  secret_text=text_at(debug_page,"密码：已隐藏",1,0xffaaaaaa);personal_wrap(secret_text,256);lv_size(secret_text,256,80);
  unsigned font[3]={0};FN(0x4f3f10,void (*)(unsigned *,LvObj *,unsigned,unsigned))(font,secret_text,0,0x57);secret_font=(const void *)font[0];
  secret_toggle=child(button_at(debug_page,"显示密码",-72,66,136,secret_clicked),0);
  button_at(debug_page,"连接",72,66,136,connect_saved_clicked);
  button_at(debug_page,"删除网络",-63,113,118,forget_clicked);
  button_at(debug_page,"返回",63,113,118,back_clicked);
  personal_info=text_at(debug_page,"",31,0xffaaaaaa);lv_size(personal_info,260,27);FN(0x52f4f0,void (*)(LvObj *,int))(personal_info,4);
  if(!personal_busy())unlink("/tmp/disc-runtime/personal-result");
 }else if(kind==10){
  char b[256];snprintf(b,sizeof(b),"%s\n删除保存的连接信息\n若正在连接此网络会断开",selected_ssid);
  personal_info=text_at(debug_page,b,-12,0xffdddddd);personal_wrap(personal_info,240);
  personal_action=child(button_at(debug_page,"确认删除",0,76,180,forget_confirm_clicked),0);
 }else if(kind==12){
  char b[256];snprintf(b,sizeof(b),"个人数据备份\n%s\n存放：TF 卡",selected_backup);
  personal_info=text_at(debug_page,b,-37,0xffdddddd);personal_wrap(personal_info,250);
  button_at(debug_page,"恢复此备份",0,30,190,restore_detail_clicked);
  button_at(debug_page,"删除此备份",0,76,190,backup_delete_clicked);
 }else if(kind==11||kind==13){
  LvObj *modal=lv_create(debug_page);lv_size(modal,280,160);lv_align(modal,9,0,0);lv_bg(modal,0xff222222,0);lv_radius(modal,14,0);lv_border(modal,0,0);lv_clear_flag(modal,DISC_LV_SCROLLABLE);lv_add_flag(modal,0x4000);
  text_at(modal,kind==11?"确认备份数据？":"确认删除备份？",-40,0xffffffff);
  personal_info=text_at(modal,kind==11?"创建新副本，保留旧备份":selected_backup,0,0xffaaaaaa);personal_wrap(personal_info,248);
  button_at(modal,"取消",-62,49,100,back_clicked);
  personal_action=child(button_at(modal,kind==11?"确认":"确认删除",62,49,110,kind==11?backup_confirm_clicked:backup_delete_confirm),0);
 }
 if(flow){LvObj *footer=lv_create(debug_page);lv_size(footer,360,70);lv_border(footer,0,0);lv_bg(footer,0xff000000,0);lv_clear_flag(footer,DISC_LV_SCROLLABLE);lv_add_flag(footer,0x4000);button_at(footer,"返回",0,0,100,back_clicked);pad_bottom(debug_page,80,0);}
 else if(kind!=9)button_at(debug_page,"返回",0,125,100,back_clicked);
 // Clear prior operation text when opening a destructive confirmation.
 if(kind==7||kind==10||kind==11||kind==13)unlink("/tmp/disc-runtime/personal-result");
 refresh_personal();screen_load(debug_page);note("personal data settings page opened\n");
}
