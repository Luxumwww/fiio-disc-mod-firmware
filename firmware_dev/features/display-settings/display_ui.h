/* V2.57 display folder. Reuse complete factory rows and factory child pages;
 * original settings objects stay alive so language refresh and scroll survive. */
#define ds_choice_row FN(0x46d5c0,LvObj *(*)(LvObj *,const char *,unsigned,void (*)(void *)))
#define ds_choice_check FN(0x46d848,void (*)(LvObj *,int))
static LvObj *ds_origin,*ds_entry,*ds_page,*ds_native_child,*ds_status_bar,*ds_native_owned[2];
static LvObj *ds_values[9],*ds_error_row,*ds_error_label;
static unsigned ds_status_hidden,ds_status_mode;
static int ds_view,ds_pending,ds_pending_value;
static char ds_last_error[100];
enum {DS_OPEN=1,DS_BRIGHTNESS,DS_SCREEN_OFF,DS_WHITEPOINT,DS_VALUE,DS_BACK};
static void ds_make(int);
static void ds_deleted(void *e){
 LvObj *o=lv_event_target(e);
 if(o==ds_origin){ds_origin=ds_entry=0;}
 if(o==ds_entry)ds_entry=0;
 if(o==ds_page){ds_page=0;ds_error_row=ds_error_label=0;for(int i=0;i<9;i++)ds_values[i]=0;}
 if(o==ds_native_child)ds_native_child=0;
 for(int i=0;i<2;i++)if(o==ds_native_owned[i])ds_native_owned[i]=0;
}
static int ds_has_action(LvObj *o,unsigned action){
 /* Verified factory event descriptor: callback/user_data/filter, 12 bytes;
  * lv_obj spec_attr +0x0c events, +0x20 upper half event count. */
 if(!o)return 0;unsigned *s=*(unsigned **)((char *)o+8);if(!s)return 0;
 unsigned n=s[8]>>16,*ev=(unsigned *)s[3];if(n>32)return 0;
 for(unsigned k=0;ev&&k<n;k++)if(ev[k*3]==0x4575f0&&ev[k*3+1]==action&&(ev[k*3+2]&255)==DISC_LV_CLICKED)return 1;
 return 0;
}
static void ds_click(void *e){
 if(!stationary_clicked(e))return;
 unsigned value=(unsigned long)FN(0x4e394c,void *(*)(void *))(e);
 ds_pending=(int)(value&255);ds_pending_value=(int)(value>>8);
}
static void ds_back(void *e){if(FN(0x488860,int (*)(void *))(e)==3)ds_pending=DS_BACK;}
static void display_settings_prepare(void){
 if(ds_page&&lv_active_screen(lv_disp_default())==ds_page)toolbox_clear_native_layers();
 if(!page||!lv_valid(page))return;
 LvObj *bright=0,*off=0;
 for(int i=0;i<16;i++)if(stock_rows[i]&&lv_valid(stock_rows[i])){
  if(ds_has_action(stock_rows[i],1))bright=stock_rows[i];
  if(ds_has_action(stock_rows[i],2))off=stock_rows[i];
 }
 if(!bright||!off)return;
 if(ds_origin!=page||!ds_entry||!lv_valid(ds_entry)){
  unsigned n=child_count(page);original_row(page,"显示设置",DS_OPEN,ds_click,0);
  LvObj *entry=child(page,(int)n);if(!entry)return;
  ds_origin=page;ds_entry=entry;protect_tap(entry);lv_add_cb(entry,ds_deleted,DISC_LV_DELETE,0);
  lv_add_cb(page,ds_deleted,DISC_LV_DELETE,0);
  /* Insert before the identified original brightness row; never identify a
   * stock action by its position after extensions have inserted children. */
  for(unsigned i=0;i<n;i++)if(child(page,(int)i)==bright){move_index(entry,(int)i);break;}
 }
 lv_add_flag(bright,DISC_LV_HIDDEN);lv_add_flag(off,DISC_LV_HIDDEN);
 /* 457e6c rewrites children 1..16 on language/page refresh. settings.c first
  * restores all 16 original labels, then restores this extension label. */
 LvObj *label=child(ds_entry,0);const char *s=label?*(const char **)((char *)label+44):0;
 if(label&&(!s||strcmp(s,"显示设置")))lv_text(label,"显示设置");
}
static void ds_restore_status(void){
 LvObj *b=LV_GLOBAL(0x8de408);
 if(b&&b==ds_status_bar&&lv_valid(b)&&*(unsigned *)0x8e16d4==ds_status_mode&&(*(unsigned *)((char *)b+36)&1)!=ds_status_hidden)
  FN(ds_status_hidden?0x479090:0x4790b0,void (*)(void))();
}
static void ds_close(int restore){
 LvObj *old=ds_page;ds_page=0;ds_error_row=ds_error_label=0;ds_pending=0;
 if(restore&&ds_origin&&lv_valid(ds_origin)&&*(unsigned *)0x8e16d4==ds_status_mode){
  toolbox_clear_native_layers();screen_load(ds_origin);ds_restore_status();
 }
 if(old&&lv_valid(old))screen_delete(&old);
 for(int i=0;i<9;i++)ds_values[i]=0;
}
static LvObj *ds_screen(const char *title){
 LvObj *p=lv_create(0);if(!p)return 0;screen_register(p);lv_size(p,360,360);
 lv_radius(p,0,0);lv_border(p,0,0);lv_bg(p,0xff000000,0);bg_opa(p,255,0);
 pad_left(p,0,0);pad_right(p,0,0);pad_top(p,0,0);pad_bottom(p,0,0);
 FN(0x4f6b30,void (*)(LvObj *,int,unsigned))(p,5,0);
 FN(0x4f0268,void (*)(LvObj *,int))(p,0);FN(0x4f02c8,void (*)(LvObj *,int))(p,12);
 FN(0x5030a0,void (*)(LvObj *,int))(p,1);FN(0x5030e4,void (*)(LvObj *,int,int,int))(p,0,0,2);
 native_title(p,title,0xb4fb);lv_add_cb(p,ds_deleted,DISC_LV_DELETE,0);
 lv_add_cb(p,ds_back,1,0);lv_add_cb(p,ds_back,2,0);lv_add_cb(p,ds_back,8,0);return p;
}
static void ds_footer(LvObj *p){
 LvObj *o=lv_create(p);if(!o)return;lv_size(o,360,40);lv_border(o,0,0);bg_opa(o,0,0);
 pad_top(o,0,0);pad_bottom(o,0,0);lv_clear_flag(o,DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);lv_add_flag(o,0x4000);
}
static void ds_menu_row(LvObj *p,const char *text,unsigned action){
 unsigned n=child_count(p);original_row(p,text,action,ds_click,0);LvObj *o=child(p,(int)n);if(o)protect_tap(o);
}
static void ds_update_error(void){
 if(!ds_error_row||!ds_error_label)return;const char *text=ds_error_text();
 if(!strcmp(text,ds_last_error))return;snprintf(ds_last_error,sizeof ds_last_error,"%s",text);
 lv_text(ds_error_label,text);if(text[0]){
  lv_clear_flag(ds_error_row,DISC_LV_HIDDEN);
  FN(0x4f273c,void (*)(LvObj *,int))(ds_error_row,0);
 }else lv_add_flag(ds_error_row,DISC_LV_HIDDEN);
}
static void ds_make(int view){
 LvObj *old=ds_page,*p=ds_screen(view?"降低白点值":"显示设置");if(!p)return;
 ds_page=p;ds_view=view;ds_error_row=ds_error_label=0;ds_last_error[0]=0;
 for(int i=0;i<9;i++)ds_values[i]=0;
 if(!view){
  ds_menu_row(p,"亮度",DS_BRIGHTNESS);ds_menu_row(p,"熄屏时间",DS_SCREEN_OFF);ds_menu_row(p,"降低白点值",DS_WHITEPOINT);
 }else{
  unsigned n=child_count(p);original_row(p,"",0,ds_click,0);ds_error_row=child(p,(int)n);
  if(ds_error_row){ds_error_label=child(ds_error_row,0);LvObj *arrow=child(ds_error_row,1);if(arrow)lv_add_flag(arrow,DISC_LV_HIDDEN);lv_clear_flag(ds_error_row,DISC_LV_CLICKABLE);lv_add_flag(ds_error_row,DISC_LV_HIDDEN);}
  int current=ds_get_reduction();
  for(int i=0;i<9;i++){
   char text[24];if(i)snprintf(text,sizeof text,"%d%%",i*10);else snprintf(text,sizeof text,"关闭");
   ds_values[i]=ds_choice_row(p,text,DS_VALUE|((unsigned)(i*10)<<8),ds_click);
   if(ds_values[i]){protect_tap(ds_values[i]);ds_choice_check(ds_values[i],current==i*10);}
  }
 }
 ds_footer(p);toolbox_clear_native_layers();screen_load(p);if(old&&lv_valid(old))screen_delete(&old);
 if(view){
  int selected=ds_get_reduction()/10;if(selected<0)selected=0;if(selected>8)selected=8;
  if(ds_values[selected])FN(0x4f273c,void (*)(LvObj *,int))(ds_values[selected],0);
  ds_update_error();
 }
}
/* Both factory pages receive this exact return callback. They call it after
 * the verified left-edge gesture succeeds, then delete their own page/timers.
 * No nav-stack entry, substitute settings writer, or callback interception. */
static void ds_native_return(void){
 ds_native_child=0;ds_pending=0;
 if(ds_page&&lv_valid(ds_page)&&ds_origin&&lv_valid(ds_origin)&&*(unsigned *)0x8e16d4==ds_status_mode){
  toolbox_clear_native_layers();screen_load(ds_page);ds_restore_status();
 }
}
static void display_settings_poll(void){
 display_settings_prepare();LvObj *active=lv_active_screen(lv_disp_default());
 if(ds_native_child&&!lv_valid(ds_native_child))ds_native_child=0;
 if(ds_page&&(!ds_origin||!lv_valid(ds_origin)||*(unsigned *)0x8e16d4!=ds_status_mode)){ds_close(0);return;}
 /* The display folder is intentionally retained while its factory child is
  * visible; its exact scroll offset remains in the same live LVGL object. */
 if(ds_page&&active!=ds_page&&active!=ds_native_child){ds_close(0);return;}
 int action=ds_pending,value=ds_pending_value;ds_pending=0;
 if(action==DS_OPEN&&active==ds_origin&&!ds_page){
  ds_status_bar=LV_GLOBAL(0x8de408);ds_status_mode=*(unsigned *)0x8e16d4;
  ds_status_hidden=ds_status_bar&&lv_valid(ds_status_bar)?*(unsigned *)((char *)ds_status_bar+36)&1:0;ds_make(0);
 }else if(ds_page&&active==ds_page){
  if(action==DS_BACK){if(ds_view)ds_make(0);else ds_close(1);}
  else if(action==DS_WHITEPOINT&&!ds_view)ds_make(1);
  else if((action==DS_BRIGHTNESS||action==DS_SCREEN_OFF)&&!ds_view){
   unsigned global=action==DS_BRIGHTNESS?0x8de04c:0x8de144;
   int which=action==DS_BRIGHTNESS?0:1;LvObj *existing=LV_GLOBAL(global);
   if(existing&&existing==ds_native_owned[which]&&lv_valid(existing)){
    /* A native child can survive an external screen/mode transition. It has
     * our verified return callback; reuse it without leaking/overwriting the
     * original singleton or forcing an old parent back onto another screen. */
    ds_native_child=existing;toolbox_clear_native_layers();screen_load(existing);
   }else if(!existing||!lv_valid(existing)){
    /* Never overwrite a factory singleton owned by a different entry point. */
    toolbox_clear_native_layers();FN(action==DS_BRIGHTNESS?0x471108:0x473054,void (*)(void (*)(void)))(ds_native_return);
    ds_native_child=LV_GLOBAL(global);if(ds_native_child&&lv_valid(ds_native_child)){ds_native_owned[which]=ds_native_child;lv_add_cb(ds_native_child,ds_deleted,DISC_LV_DELETE,0);}
   }
  }else if(action==DS_VALUE&&ds_view){
   ds_set_reduction(value);int current=ds_get_reduction();
   for(int i=0;i<9;i++)if(ds_values[i])ds_choice_check(ds_values[i],current==i*10);
  }
 }
 if(ds_page&&ds_view&&lv_active_screen(lv_disp_default())==ds_page)ds_update_error();
}
