/* Included by settings.c. Native row/title constructors, main UI thread only. */
#include "thermal_frame.h"
#include "toolbox_icon.h"
extern int fork(void),execl(const char *,const char *,...),kill(int,int),waitpid(int,int *,int);
extern void _exit(int);
extern void *malloc(size_t),*memcpy(void *,const void *,size_t),*mmap(void *,size_t,int,int,int,long);
extern void free(void *);
extern int munmap(void *,size_t);
extern long lseek(int,long,int);
static LvObj *toolbox_menu,*toolbox_entry,*toolbox_origin,*toolbox_list,*thermal_page,*thermal_image,*thermal_status,*thermal_retry;
static LvObj *toolbox_stock_rows[32];
static unsigned toolbox_stock_count,toolbox_insert_index,toolbox_token,thermal_sequence,thermal_draw_at;
static int toolbox_open_pending,thermal_open_pending,toolbox_back_pending,thermal_child;
static ThermalFrame *thermal_map;
static unsigned char *thermal_pixels;
static LvObj *thermal_header,*thermal_fullscreen_switch;
static int thermal_fullscreen;
static struct {unsigned header,size;const void *data;} thermal_desc;
static struct {LvObj *bar;unsigned hidden,mode;} toolbox_toolbar;
static char thermal_last_status[256];
static LvObj *thermal_sheet,*thermal_menu_feedback;
static LvObj *thermal_palette_rows[8];
static const unsigned thermal_palette_values[8]={1,2,10,11,12,13,14,15};
static int thermal_palette_drawn=-2;
static int thermal_sheet_view,thermal_menu_pending,thermal_pull,thermal_pull_x,thermal_pull_y,thermal_pull_origin,thermal_palette_actual=-1;
static void thermal_sheet_make(int);
static int thermal_pull_event(void *);
static void thermal_header_event(void *e){if(thermal_page&&lv_active_screen(lv_disp_default())==thermal_page)thermal_pull_event(e);}
static void thermal_native_pull(void *e){
 if(thermal_page&&lv_active_screen(lv_disp_default())==thermal_page){thermal_pull_event(e);return;}
 FN(0x47fdac,void (*)(void *))(e);
}
static LvObj *thermal_native_strip;
static void thermal_strip_deleted(void *e){if(lv_event_target(e)==thermal_native_strip)thermal_native_strip=0;}
static void thermal_bind_native_strip(void){
 if(thermal_native_strip&&lv_valid(thermal_native_strip))return;
 void *display=lv_disp_default();if(!display)return;
 /* Verified display layer_top at +28. Locate the real factory pull strip
  * by its 0x47fdac callback, never by its child index or screen position. */
 LvObj *top=*(LvObj **)((char *)display+28);if(!top||!lv_valid(top))return;
 unsigned n=child_count(top);if(n>32)return;
 for(unsigned i=0;i<n;i++){
  LvObj *o=child(top,(int)i);if(!o||!lv_valid(o))continue;
  unsigned *s=*(unsigned **)((char *)o+8);if(!s)continue;
  unsigned count=s[8]>>16,*events=(unsigned *)s[3];if(count>32)continue;
  int match=-1;for(unsigned k=0;events&&k<count;k++)if(events[k*3]==0x47fdac)match=(int)k;
  if(match<0)continue;
  /* Merely adding a second callback lets the native handler expose its
   * global shade and steal the pointer mid-drag. Route this one verified
   * callback only while our page is active; preserve it verbatim otherwise. */
  events[(unsigned)match*3]=(unsigned long)thermal_native_pull;
  thermal_native_strip=o;lv_add_cb(o,thermal_strip_deleted,DISC_LV_DELETE,0);return;
 }
}
static void toolbox_close_thermal(int);
static LvObj *toolbox_label(LvObj *row){
 if(!row||!lv_valid(row))return 0;unsigned count=child_count(row);if(count>6)return 0;
 for(unsigned i=0;i<count;i++){LvObj *l=child(row,(int)i);if(!l||*(unsigned *)l!=0x7ffd00)continue;const char *s=*(const char **)((char *)l+44);if(s&&strcmp(s,">"))return l;}return 0;
}
static const char *toolbox_label_text(LvObj *row){LvObj *l=toolbox_label(row);return l?*(const char **)((char *)l+44):0;}
static void toolbox_deleted(void *e){
 LvObj *p=lv_event_target(e);
 if(p==toolbox_menu){toolbox_menu=toolbox_entry=0;toolbox_stock_count=0;}
 if(p==toolbox_origin)toolbox_origin=0;
 if(p==toolbox_list)toolbox_list=0;
 if(p==thermal_page){thermal_page=thermal_image=thermal_status=thermal_retry=0;unlink("/tmp/disc-runtime/toolbox.session");}
}
static void toolbox_enter(void *e){if(stationary_clicked(e))toolbox_open_pending=1;}
static void thermal_enter(void *e){if(stationary_clicked(e))thermal_open_pending=1;}
static void toolbox_back(void *e){if(thermal_page&&thermal_pull_event(e))return;int r=FN(0x488860,int (*)(void *))(e);if(r==3)toolbox_back_pending=1;}
static void toolbox_clear_native_layers(void){
 LvObj *shade=LV_GLOBAL(0x8deee4);
 if(shade&&lv_valid(shade)&&(!(*(unsigned *)((char *)shade+36)&1)||*(int *)((char *)shade+24)>-360)){
  FN(0x51b4b8,int (*)(void *,void *))(shade,0);lv_add_flag(shade,DISC_LV_HIDDEN);FN(0x4eb600,void (*)(LvObj *,int))(shade,-360);
 }
 if(hw_api){DiscAccelStats s;hw_api->stats(0,0,&s);if(s.active&&s.lease&&!strcmp(s.owner,"disc-bt-meter/page")){hw_api->release(s.lease);hw_active=0;}}
}
static void toolbox_restore_toolbar(void){
 LvObj *bar=LV_GLOBAL(0x8de408);if(!toolbox_toolbar.bar||bar!=toolbox_toolbar.bar||!lv_valid(bar)||toolbox_toolbar.mode!=*(unsigned *)0x8e16d4)return;
 if((*(unsigned *)((char *)bar+36)&1)!=toolbox_toolbar.hidden)FN(toolbox_toolbar.hidden?0x479090:0x4790b0,void (*)(void))();
}
static void toolbox_prepare(void){
 LvObj *p=lv_active_screen(lv_disp_default());if(!p)return;if(p==toolbox_list||p==thermal_page){if(p==thermal_page)thermal_bind_native_strip();toolbox_clear_native_layers();return;}
 if(toolbox_menu&&lv_valid(toolbox_menu)&&toolbox_entry&&lv_valid(toolbox_entry)){
  /* Stock menu refreshes labels by original child position. Restore their
   * original object association after inserting our row, as system rows do. */
  const char *s=toolbox_label_text(toolbox_entry);
  if(s&&strcmp(s,"工具箱")){
   char names[32][128];for(unsigned i=0;i<toolbox_stock_count;i++){const char *v=toolbox_label_text(child(toolbox_menu,(int)i));snprintf(names[i],128,"%s",v?v:"");}
   for(unsigned i=0;i<toolbox_stock_count;i++)if(toolbox_stock_rows[i]&&toolbox_label(toolbox_stock_rows[i]))lv_text(toolbox_label(toolbox_stock_rows[i]),names[i]);
   lv_text(toolbox_label(toolbox_entry),"工具箱");
  }return;
 }
 /* Locate by the real label/object, never assume a child index. No existing
  * button callback or selection ID is replaced. */
 unsigned count=child_count(p);if(count>30||count<3)return;
 for(unsigned i=0;i<count;i++){
  LvObj *r=child(p,(int)i);const char *s=toolbox_label_text(r);
  if(!s||(strcmp(s,"工作模式")&&strcmp(s,"Work mode")&&strcmp(s,"Work Mode")))continue;
  unsigned *spec=*(unsigned **)((char *)r+8);if(!spec)continue;unsigned ec=spec[8]>>16,*events=(unsigned *)spec[3];int native=0;
  if(ec>32)continue;for(unsigned k=0;events&&k<ec;k++)if(events[k*3]==0x46d480&&events[k*3+1]==0&&(events[k*3+2]&255)==7)native=1;if(!native)continue;
  for(unsigned k=0;k<count;k++)toolbox_stock_rows[k]=child(p,(int)k);
  /* Exact root-settings layout used by FUN_0046da04: factory icon, label
   * width, font, padding and arrow, rather than the plain system-menu row. */
  int layout[3]={95,200,(int)(unsigned long)&toolbox_icon};
  original_row(p,"工具箱",0,toolbox_enter,layout);if(child_count(p)!=count+1)return;
  toolbox_entry=child(p,-1);toolbox_menu=p;toolbox_stock_count=count;toolbox_insert_index=i+1;
  protect_tap(toolbox_entry);move_index(toolbox_entry,(int)toolbox_insert_index);lv_add_cb(p,toolbox_deleted,DISC_LV_DELETE,0);return;
 }
}
static LvObj *toolbox_screen(const char *title){
 LvObj *p=lv_create(0);if(!p)return 0;screen_register(p);lv_size(p,360,360);
 lv_radius(p,0,0);lv_border(p,0,0);lv_bg(p,0xff000000,0);bg_opa(p,255,0);pad_left(p,0,0);pad_right(p,0,0);pad_top(p,0,0);pad_bottom(p,0,0);
 native_title(p,title,0xb4fb);lv_add_cb(p,toolbox_deleted,DISC_LV_DELETE,0);
 lv_add_flag(child(p,0),0x4000);
 lv_add_cb(p,toolbox_back,1,0);lv_add_cb(p,toolbox_back,2,0);lv_add_cb(p,toolbox_back,8,0);return p;
}
static void toolbox_make(void){
 if(toolbox_list&&lv_valid(toolbox_list)){screen_load(toolbox_list);return;}
 toolbox_origin=lv_active_screen(lv_disp_default());if(!toolbox_origin)return;
 toolbox_toolbar.bar=LV_GLOBAL(0x8de408);toolbox_toolbar.mode=*(unsigned *)0x8e16d4;toolbox_toolbar.hidden=toolbox_toolbar.bar&&lv_valid(toolbox_toolbar.bar)?*(unsigned *)((char *)toolbox_toolbar.bar+36)&1:0;
 lv_add_cb(toolbox_origin,toolbox_deleted,DISC_LV_DELETE,0);
 toolbox_list=toolbox_screen("工具箱");if(!toolbox_list)return;
 FN(0x5030a0,void (*)(LvObj *,int))(toolbox_list,1);FN(0x5030e4,void (*)(LvObj *,int,int,int))(toolbox_list,0,0,2);
 FN(0x4f0268,void (*)(LvObj *,int))(toolbox_list,0);FN(0x4f02c8,void (*)(LvObj *,int))(toolbox_list,12);
 original_row(toolbox_list,"热成像",0,thermal_enter,0);protect_tap(child(toolbox_list,-1));
 screen_load(toolbox_list);
}
static void thermal_start(void){
 if(thermal_child>0){int status;int r=waitpid(thermal_child,&status,1);if(!r)return;thermal_child=0;}
 if(thermal_map){munmap(thermal_map,sizeof(*thermal_map));thermal_map=0;}
 if(access("/usr/data/disc-runtime/components/disc-usb-tools/enabled",0)||!access("/usr/data/disc-runtime/components/disc-usb-tools/disabled",0)){lv_text(thermal_status,"请先启用 USB 工具模块");return;}
 if(!access("/tmp/disc-runtime/bt-usb.request",0)||*(unsigned *)0x8e16d4!=0){lv_text(thermal_status,"请先退出其他 USB 模式\n并关闭 USB 输出");return;}
 toolbox_token=milliseconds();if(!toolbox_token)toolbox_token=1;
 char b[96];int n=snprintf(b,sizeof(b),"%u %u\n",(unsigned)getpid(),toolbox_token);
 int fd=open("/tmp/disc-runtime/toolbox.session.new",1|0x100|0x200,0600);if(fd<0)return;int ok=write(fd,b,n)==n;close(fd);if(!ok||rename("/tmp/disc-runtime/toolbox.session.new","/tmp/disc-runtime/toolbox.session"))return;
 char p[24],c[24];snprintf(p,sizeof(p),"%u",(unsigned)getpid());snprintf(c,sizeof(c),"%u",toolbox_token);
 int pid=fork();if(pid==0){
  /* Do not modify inherited environment or call LVGL after fork. */
  execl("/usr/data/disc-runtime/components/disc-usb-tools/current/thermal-start.sh","thermal-start.sh",p,c,(char *)0);_exit(127);
 }if(pid<0){unlink("/tmp/disc-runtime/toolbox.session");lv_text(thermal_status,"热成像启动失败");return;}
 thermal_child=pid;thermal_sequence=thermal_draw_at=0;thermal_last_status[0]=0;lv_text(thermal_status,"正在准备 USB 热成像");lv_add_flag(thermal_retry,DISC_LV_HIDDEN);
}
static void thermal_menu_click(void *e){
 if(!stationary_clicked(e))return;unsigned action=(unsigned long)FN(0x4e394c,void *(*)(void *))(e);thermal_menu_pending=(int)action;
}
static void thermal_palette_click(void *e){
 if(!stationary_clicked(e))return;unsigned value=(unsigned long)FN(0x4e394c,void *(*)(void *))(e);
 if(thermal_palette_actual<0){if(thermal_menu_feedback)lv_text(thermal_menu_feedback,"请等待设备配色就绪");return;}
 char b[96];int n=snprintf(b,sizeof(b),"%u %u palette %u\n",(unsigned)getpid(),toolbox_token,value);
 int fd=open("/tmp/disc-runtime/thermal.command.new",1|0x100|0x200,0600);if(fd<0)return;int ok=write(fd,b,n)==n;close(fd);if(ok&&!rename("/tmp/disc-runtime/thermal.command.new","/tmp/disc-runtime/thermal.command")){if(thermal_menu_feedback)lv_text(thermal_menu_feedback,"正在切换配色");}
}
static void thermal_display_layout(void){
 unsigned width=thermal_fullscreen?360u:THERMAL_W,height=thermal_fullscreen?360u:THERMAL_H;
 if(thermal_header){if(thermal_fullscreen)lv_add_flag(thermal_header,DISC_LV_HIDDEN);else lv_clear_flag(thermal_header,DISC_LV_HIDDEN);}
 if(thermal_image){thermal_desc.header=5u|(width<<10)|(height<<21);thermal_desc.size=width*height*4u;
  FN(0x52cb44,void (*)(LvObj *,const void *))(thermal_image,&thermal_desc);lv_align(thermal_image,1,thermal_fullscreen?0:90,thermal_fullscreen?0:80);FN(0x4ece1c,void (*)(LvObj *))(thermal_image);}
 thermal_sequence=0;
}
static int thermal_display_save(int on){
 mkdir("/usr/data/disc-runtime/module-data/disc-usb-tools",0700);
 int fd=open("/usr/data/disc-runtime/module-data/disc-usb-tools/display.conf.new",1|0x100|0x200,0600);if(fd<0)return 0;
 const char *text=on?"fullscreen=1\n":"fullscreen=0\n";int ok=write(fd,text,13)==13;close(fd);
 return ok&&!rename("/usr/data/disc-runtime/module-data/disc-usb-tools/display.conf.new","/usr/data/disc-runtime/module-data/disc-usb-tools/display.conf");
}
static const char *thermal_palette_name(int value){switch(value){case 1:return "白热";case 2:return "黑热";case 10:return "铁红 1";case 11:return "彩虹";case 12:return "铁红 2";case 13:return "熔岩 1";case 14:return "熔岩 2";case 15:return "融合色";default:return "设备配色";}}
static void thermal_sheet_make(int view){
 int y=0;if(thermal_sheet){y=*(int *)((char *)thermal_sheet+24);LvObj *old=thermal_sheet;thermal_sheet=0;screen_delete(&old);}
 thermal_sheet=lv_create(thermal_page);thermal_menu_feedback=0;thermal_fullscreen_switch=0;thermal_sheet_view=view;
 lv_size(thermal_sheet,360,360);lv_radius(thermal_sheet,0,0);lv_border(thermal_sheet,0,0);lv_bg(thermal_sheet,0xff000000,0);bg_opa(thermal_sheet,255,0);
 pad_left(thermal_sheet,0,0);pad_right(thermal_sheet,0,0);pad_top(thermal_sheet,0,0);pad_bottom(thermal_sheet,50,0);
 FN(0x5030a0,void (*)(LvObj *,int))(thermal_sheet,1);FN(0x5030e4,void (*)(LvObj *,int,int,int))(thermal_sheet,0,0,2);
 native_title(thermal_sheet,view==2?"色彩模式":"热成像设置",0xb4fb);lv_add_flag(thermal_sheet,0x4000);lv_add_flag(child(thermal_sheet,0),0x4000);
 if(view!=2)lv_clear_flag(thermal_sheet,DISC_LV_SCROLLABLE);
 lv_clear_flag(child(thermal_sheet,0),DISC_LV_SCROLLABLE);
 if(view==1){const char *names[]={"色彩模式","全屏显示","重新连接","关闭热成像"};const unsigned actions[]={1,4,2,3};for(unsigned i=0;i<4;i++){
  original_row(thermal_sheet,names[i],actions[i],thermal_menu_click,0);LvObj *r=child(thermal_sheet,-1);protect_tap(r);
  if(actions[i]==4){
   unsigned count=child_count(r);for(unsigned j=0;j<count;j++){LvObj *l=child(r,(int)j);if(l&&*(unsigned *)l==0x7ffd00){const char *text=*(const char **)((char *)l+44);if(text&&!strcmp(text,">"))lv_add_flag(l,DISC_LV_HIDDEN);}}
   thermal_fullscreen_switch=FN(0x533960,LvObj *(*)(LvObj *))(r);
   if(thermal_fullscreen_switch){lv_size(thermal_fullscreen_switch,60,33);lv_align(thermal_fullscreen_switch,9,100,0);lv_bg(thermal_fullscreen_switch,0xffff0000,0x20001);lv_bg(thermal_fullscreen_switch,0xff565656,0);
    lv_clear_flag(thermal_fullscreen_switch,2|0x2000);protect_tap(thermal_fullscreen_switch);lv_add_cb(thermal_fullscreen_switch,thermal_menu_click,7,(void *)4);
    FN(thermal_fullscreen?0x4e98f8:0x4e9918,void (*)(LvObj *,unsigned))(thermal_fullscreen_switch,1);}
  }
 }}
 else {for(unsigned i=0;i<8;i++){unsigned value=thermal_palette_values[i];char b[64];snprintf(b,sizeof(b),"%s%s",thermal_palette_name((int)value),thermal_palette_actual==(int)value?" · 当前":"");original_row(thermal_sheet,b,value,thermal_palette_click,0);thermal_palette_rows[i]=child(thermal_sheet,-1);protect_tap(thermal_palette_rows[i]);}thermal_palette_drawn=thermal_palette_actual;}
 thermal_menu_feedback=lv_label(thermal_sheet);lv_add_style(thermal_menu_feedback,(void *)0x83ac5c,0);lv_color(thermal_menu_feedback,0xffaaaaaa,0);lv_size(thermal_menu_feedback,280,64);FN(0x52f4f0,void (*)(LvObj *,int))(thermal_menu_feedback,0);lv_text(thermal_menu_feedback,view==2?"选择后自动保存":"");if(view==1)lv_add_flag(thermal_menu_feedback,DISC_LV_HIDDEN);
 lv_align(thermal_sheet,1,0,y);if(!view)FN(0x4eb600,void (*)(LvObj *,int))(thermal_sheet,-360);
}
static void thermal_sheet_to(int y){if(!thermal_sheet)return;unsigned a[32];FN(0x51b4b8,int (*)(void *,void *))(thermal_sheet,0);FN(0x51b3e0,void (*)(void *))(a);a[0]=(unsigned)thermal_sheet;a[1]=0x4eb600;a[8]=a[9]=*(unsigned *)((char *)thermal_sheet+24);a[10]=(unsigned)y;a[11]=200;FN(0x51b7b0,void *(*)(void *))(a);}
static int thermal_pull_event(void *e){
 if(!thermal_page||!thermal_sheet)return 0;unsigned code=lv_event_code(e);void *input=indev_active();int p[2];if(!input)return 0;indev_point(input,p);
 if(code==1||code==8){char b[128];int n=snprintf(b,sizeof(b),"event=%u x=%d y=%d sheet_y=%d pull=%d\n",code,p[0],p[1],*(int *)((char *)thermal_sheet+24),thermal_pull);int f=open("/tmp/disc-runtime/thermal.touch",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}}
 if(code==1){if(thermal_pull)return 1;if(p[0]>=60&&((*(int *)((char *)thermal_sheet+24)<=-350&&p[1]<80)||(thermal_sheet_view==1&&*(int *)((char *)thermal_sheet+24)>=-10))){thermal_pull=1;thermal_pull_x=p[0];thermal_pull_y=p[1];thermal_pull_origin=*(int *)((char *)thermal_sheet+24);}}
 else if(thermal_pull&&(code==2||code==8)){
  int dx=p[0]-thermal_pull_x,dy=p[1]-thermal_pull_y,ax=dx<0?-dx:dx,ay=dy<0?-dy:dy;
  /* Commit a full open/close once the vertical gesture is clear. Moving the
   * sheet under a pressed pointer can retarget it and lose RELEASED; never
   * leave a partially exposed sheet dependent on that final event. */
  if(ay>=32&&ay>ax){int open=dy>0;thermal_pull=0;thermal_sheet_to(open?0:-360);return 1;}
  if(code==8||ax>80){thermal_pull=0;return 0;}
 }
 return 0;
}
static void thermal_retry_click(void *e){if(stationary_clicked(e))thermal_start();}
static void thermal_make(void){
 if(thermal_page)return;
 thermal_page=toolbox_screen("热成像");if(!thermal_page)return;lv_clear_flag(thermal_page,DISC_LV_SCROLLABLE);
 /* Own this page's header scroll/press handling. The factory title's
  * default scrolling otherwise steals the pull before it reaches the page. */
 LvObj *header=child(thermal_page,0);thermal_header=header;lv_clear_flag(header,DISC_LV_SCROLLABLE|0x4000);lv_add_flag(header,DISC_LV_CLICKABLE|0x2000);
 lv_add_cb(header,thermal_header_event,1,0);lv_add_cb(header,thermal_header_event,2,0);lv_add_cb(header,thermal_header_event,8,0);
 char display_config[40];thermal_fullscreen=read_data("/usr/data/disc-runtime/module-data/disc-usb-tools/display.conf",display_config,sizeof(display_config))>0&&!strcmp(display_config,"fullscreen=1\n");
 thermal_pixels=malloc(360u*360u*4u);if(thermal_pixels){
  thermal_desc.header=5u|(THERMAL_W<<10)|(THERMAL_H<<21);thermal_desc.size=THERMAL_W*THERMAL_H*4;thermal_desc.data=thermal_pixels;
  thermal_image=FN(0x52cb10,LvObj *(*)(LvObj *))(thermal_page);FN(0x52cb44,void (*)(LvObj *,const void *))(thermal_image,&thermal_desc);lv_align(thermal_image,1,90,80);lv_add_flag(thermal_image,DISC_LV_HIDDEN|0x4000);lv_clear_flag(thermal_image,DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);
 }
 thermal_display_layout();
 thermal_status=lv_label(thermal_page);lv_add_style(thermal_status,(void *)0x83ac5c,0);lv_color(thermal_status,0xffaaaaaa,0);lv_size(thermal_status,276,96);lv_align(thermal_status,9,0,-8);FN(0x52f4f0,void (*)(LvObj *,int))(thermal_status,0);
 thermal_retry=button_at(thermal_page,"重新连接",0,85,190,thermal_retry_click);lv_add_flag(thermal_retry,DISC_LV_HIDDEN);
 thermal_sheet_make(1);FN(0x4eb600,void (*)(LvObj *,int))(thermal_sheet,-360);thermal_palette_actual=-1;
 screen_load(thermal_page);thermal_start();
}
static void toolbox_close_thermal(int restore){
 unlink("/tmp/disc-runtime/toolbox.session");
 LvObj *old=thermal_page;thermal_page=thermal_image=thermal_status=thermal_retry=thermal_sheet=thermal_menu_feedback=thermal_header=thermal_fullscreen_switch=0;thermal_pull=thermal_menu_pending=0;
 if(restore&&toolbox_list&&lv_valid(toolbox_list)){screen_load(toolbox_list);toolbox_restore_toolbar();}
 if(old&&lv_valid(old))screen_delete(&old);
 if(thermal_map){munmap(thermal_map,sizeof(*thermal_map));thermal_map=0;}free(thermal_pixels);thermal_pixels=0;thermal_sequence=0;
}
static void toolbox_frame_tick(void){
 if(!thermal_page||!thermal_image||!thermal_pixels||lv_active_screen(lv_disp_default())!=thermal_page)return;
 unsigned t=milliseconds();if(t-thermal_draw_at<33)return;thermal_draw_at=t;
 if(!thermal_map){int fd=open("/tmp/disc-runtime/thermal.frame",0);if(fd<0)return;if(lseek(fd,0,2)!=(long)sizeof(ThermalFrame)){close(fd);return;}void *m=mmap(0,sizeof(ThermalFrame),1,1,fd,0);close(fd);if(m==(void *)-1)return;thermal_map=m;}
 if(thermal_map->magic!=THERMAL_MAGIC||thermal_map->ui_pid!=(unsigned)getpid()||thermal_map->token!=toolbox_token)return;
 unsigned seq=thermal_map->sequence;if((seq&1)||!seq||seq==thermal_sequence)return;
 __sync_synchronize();
 if(!thermal_fullscreen)memcpy(thermal_pixels,thermal_map->pixels,THERMAL_W*THERMAL_H*4);
 else{const unsigned *source=(const unsigned *)thermal_map->pixels;unsigned *destination=(unsigned *)thermal_pixels;
  /* Cover the square with one uniform scale. Keep the centered square
   * of the portrait source instead of stretching its two axes unequally. */
  for(unsigned y=0;y<360;y++){unsigned sy=(THERMAL_H-THERMAL_W)/2+y*THERMAL_W/360;for(unsigned x=0;x<360;x++)destination[y*360+x]=source[sy*THERMAL_W+x*THERMAL_W/360];}}
 __sync_synchronize();if(seq!=thermal_map->sequence)return;
 thermal_sequence=seq;lv_clear_flag(thermal_image,DISC_LV_HIDDEN);lv_add_flag(thermal_status,DISC_LV_HIDDEN);lv_add_flag(thermal_retry,DISC_LV_HIDDEN);FN(0x4ece1c,void (*)(LvObj *))(thermal_image);
}
static void toolbox_poll(void){
 toolbox_prepare();
 if(toolbox_open_pending){toolbox_open_pending=0;toolbox_make();}
 if(thermal_open_pending){thermal_open_pending=0;thermal_make();}
 if(toolbox_back_pending){toolbox_back_pending=0;if(thermal_page&&thermal_sheet&&*(int *)((char *)thermal_sheet+24)>-350){if(thermal_sheet_view==2)thermal_sheet_make(1);else thermal_sheet_to(-360);}else if(thermal_page)toolbox_close_thermal(1);else if(toolbox_list){LvObj *old=toolbox_list;toolbox_list=0;if(toolbox_origin&&lv_valid(toolbox_origin)){screen_load(toolbox_origin);toolbox_restore_toolbar();}screen_delete(&old);}}
 if(thermal_menu_pending){int action=thermal_menu_pending;thermal_menu_pending=0;if(thermal_page){if(action==1)thermal_sheet_make(2);else if(action==2){thermal_sheet_to(-360);thermal_start();}else if(action==3)toolbox_close_thermal(1);else if(action==4){
  int on=!thermal_fullscreen;if(thermal_display_save(on)){thermal_fullscreen=on;thermal_display_layout();if(thermal_fullscreen_switch)FN(on?0x4e98f8:0x4e9918,void (*)(LvObj *,unsigned))(thermal_fullscreen_switch,1);thermal_sheet_to(-360);}
  else if(thermal_menu_feedback){lv_text(thermal_menu_feedback,"显示设置保存失败");lv_clear_flag(thermal_menu_feedback,DISC_LV_HIDDEN);}
 }}}
 if(thermal_page&&lv_active_screen(lv_disp_default())!=thermal_page){toolbox_close_thermal(0);}
 if(thermal_child>0){int s;if(waitpid(thermal_child,&s,1)==thermal_child)thermal_child=0;}
 if(!thermal_page||!thermal_status)return;
 char b[512];int n=read_data("/tmp/disc-runtime/thermal.state",b,sizeof(b));if(n<=0)return;
 char owner_text[48];int owner_len=snprintf(owner_text,sizeof(owner_text),"token=%u\n",toolbox_token);int match=0;for(int i=0;i<n;i++)if(n-i>=owner_len&&(i==0||b[i-1]=='\n')&&!memcmp(b+i,owner_text,(unsigned)owner_len))match=1;if(!match)return;
 char *detail=0,*phase=0;for(int i=0;i<n;i++)if(i==0||b[i-1]=='\n'){if(n-i>=7&&!memcmp(b+i,"detail=",7))detail=b+i+7;else if(n-i>=6&&!memcmp(b+i,"phase=",6))phase=b+i+6;}
 if(!detail||!phase)return;for(char *p=detail;*p;p++)if(*p=='\n'){*p=0;break;}
 int palette=-1;for(int i=0;i<n;i++)if(n-i>=8&&(i==0||b[i-1]=='\n')&&!memcmp(b+i,"palette=",8)){palette=(int)strtol(b+i+8,0,10);break;}
 thermal_palette_actual=palette;
 if(thermal_sheet_view==2&&thermal_sheet&&thermal_palette_drawn!=palette){thermal_palette_drawn=palette;for(unsigned i=0;i<8;i++)if(thermal_palette_rows[i]&&lv_valid(thermal_palette_rows[i])){char name[64];unsigned value=thermal_palette_values[i];snprintf(name,sizeof(name),"%s%s",thermal_palette_name((int)value),palette==(int)value?" · 当前":"");lv_text(toolbox_label(thermal_palette_rows[i]),name);}}
 if(thermal_menu_feedback&&thermal_sheet_view==2){if(!strcmp(detail,"该配色暂不可用"))lv_text(thermal_menu_feedback,detail);else if(!strcmp(detail,"配色已切换"))lv_text(thermal_menu_feedback,detail);}
 if(strcmp(detail,thermal_last_status)){snprintf(thermal_last_status,sizeof(thermal_last_status),"%s",detail);lv_text(thermal_status,detail);}
 if(!memcmp(phase,"error\n",6)||!memcmp(phase,"stopped\n",8)){lv_clear_flag(thermal_status,DISC_LV_HIDDEN);lv_clear_flag(thermal_retry,DISC_LV_HIDDEN);if(thermal_image)lv_add_flag(thermal_image,DISC_LV_HIDDEN);}
 else if(!memcmp(phase,"waiting\n",8)||!memcmp(phase,"preparing\n",10)){lv_clear_flag(thermal_status,DISC_LV_HIDDEN);if(thermal_image)lv_add_flag(thermal_image,DISC_LV_HIDDEN);lv_add_flag(thermal_retry,DISC_LV_HIDDEN);}
}
