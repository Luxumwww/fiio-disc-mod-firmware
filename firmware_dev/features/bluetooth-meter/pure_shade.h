/* Stock child indices remain intact; only pure-mode layout/gestures change. */
static LvObj *pure_shade,*pure_grid,*pure_button,*pure_icon,*pure_caption,*pure_footer;
static LvObj *pure_edit_hint;
static LvObj *pure_usb_button,*pure_usb_icon,*pure_usb_caption;static int pure_usb_seen=-1;
static int pure_control_id(LvObj *);
static void pure_edit_touch(void *,int *),pure_edit_long(void *),pure_edit_object_deleted(LvObj *),pure_edit_bind(void);
static unsigned pure_grid_flags,pure_footer_flags;
static int pure_installed,pure_fold=-1,pure_height,pure_down_x,pure_down_y,pure_down_scroll,pure_tracking;
static LvObj *pure_placed[11];static unsigned pure_placed_slot[11];
static int pure_scroll(LvObj *o){void *s=*(void **)((char *)o+8);return s?-*(int *)((char *)s+20):0;}
static void pure_height_set(LvObj *o,int h){pure_height=h;lv_size(o,360,h);}
static void pure_deleted(void *e){LvObj *o=lv_event_target(e);validation_count=0;
 pure_edit_object_deleted(o);
 if(o==pure_shade){pure_shade=0;pure_fold=-1;pure_tracking=0;pure_installed=0;}
 if(o==pure_grid){pure_grid=0;pure_fold=-1;for(unsigned i=0;i<11;i++)pure_placed[i]=0;}
 if(o==pure_footer)pure_footer=0;
 if(o==pure_button)pure_button=pure_icon=0;if(o==pure_caption)pure_caption=0;
 if(o==pure_usb_button){pure_usb_button=pure_usb_icon=0;pure_usb_seen=-1;}if(o==pure_usb_caption)pure_usb_caption=0;
}
static void pure_shade_event(void *e){
 if(*(unsigned *)0x8e16d4!=0||!*(unsigned char *)0x8dee8c){FN(0x47f74c,void (*)(void *))(e);return;}
 unsigned code=lv_event_code(e);int pt[2];void *input=FN(0x4e42e8,void *(*)(void))();if(!input)return;
 FN(0x4e45bc,void (*)(void *,int *))(input,pt);
 if(pure_editing){LvObj *target=lv_event_target(e);if(target!=pure_grid&&pure_control_id(target)<0)pure_edit_touch(e,pt);return;}
 if(code==1){pure_down_x=pt[0];pure_down_y=pt[1];pure_down_scroll=pure_scroll(pure_shade);pure_tracking=1;*(unsigned char *)0x8dee7c=0;}
 else if(code==8&&pure_tracking){pure_tracking=0;
  int dy=pt[1]-pure_down_y,dx=pt[0]-pure_down_x,scroll=pure_scroll(pure_shade);
  if(dy*dy<=dx*dx)return;
  /* Up scroll reveals row four. Downward pull at the top folds the grid;
   * another upward swipe at the content bottom closes the shade. */
  if(dy>60&&pure_down_scroll<=0)*(unsigned char *)0x8dee8c=0;
  else if(dy<-60&&pure_down_scroll>170&&scroll>=pure_down_scroll-4)
   animate(pure_shade,FN(0x4eb600,void (*)(LvObj *,int)),*(int *)((char *)pure_shade+24),-360);
 }
}
static void pure_grid_event(void *e){
 if(pure_editing)return;
 if(*(unsigned *)0x8e16d4!=0||!*(unsigned char *)0x8dee8c)FN(0x47fa6c,void (*)(void *))(e);
}
static void pure_events(LvObj *o,unsigned original,void (*replacement)(void *),int install){
 if(!o||!lv_valid(o))return;unsigned *spec=*(unsigned **)((char *)o+8);if(!spec)return;
 unsigned count=spec[8]>>16;if(count>32)return;unsigned *events=(unsigned *)spec[3];
 for(unsigned i=0;events&&i<count;i++)if(events[i*3]==(install?original:(unsigned)replacement))events[i*3]=install?(unsigned)replacement:original;
}
#include "pure_toolbar_edit.h"
static void pure_positions(LvObj *grid,int added){
 for(unsigned i=0;i<(added?11:9);i++){LvObj *o=i>=9?pure_control(i):FN(0x4f8850,LvObj *(*)(LvObj *,int))(grid,(int)i);if(!o)continue;
  unsigned slot=added?pure_order_position(pure_order,i):i;int x=pure_slot_x(slot),y=pure_slot_y(slot);
  /* Parent motion changes absolute coordinates before the layout pass. Do
   * not reassign all eleven unchanged local positions on every moving frame. */
  if(pure_placed[i]!=o||pure_placed_slot[i]!=slot){lv_align(o,1,x,y);pure_placed[i]=o;pure_placed_slot[i]=slot;}
 }
}
static void pure_restore_flags(LvObj *o,unsigned saved){lv_clear_flag(o,(16|0x2000|0x4000)&~saved);lv_add_flag(o,saved);}
static void pure_prepare(unsigned mode){
 if(mode!=0||!active){
  if(pure_editing)pure_edit_finish(1);
  if(!pure_installed)return;
  if(pure_shade&&lv_valid(pure_shade)){
   pure_events(pure_shade,0x47f74c,pure_shade_event,0);FN(0x4f02c8,void (*)(LvObj *,unsigned))(pure_shade,0);
   FN(0x4f15ac,void (*)(LvObj *,int,int,int))(pure_shade,0,pure_scroll(pure_shade),0);padbottom(pure_shade,0,0);
  }
  if(pure_grid&&lv_valid(pure_grid)){
   pure_events(pure_grid,0x47fa6c,pure_grid_event,0);FN(0x51b4b8,int (*)(void *,void *))(pure_grid,pure_height_set);
   pure_positions(pure_grid,0);pure_restore_flags(pure_grid,pure_grid_flags);lv_size(pure_grid,360,*(unsigned char *)0x8dee8c?240:70);
  }
  if(pure_footer&&lv_valid(pure_footer)){lv_align(pure_footer,1,0,*(unsigned char *)0x8dee8c?330:160);pure_restore_flags(pure_footer,pure_footer_flags);}
  show(pure_button,0);show(pure_caption,0);show(pure_usb_button,0);show(pure_usb_caption,0);pure_fold=-1;pure_installed=0;return;
 }
 LvObj *shade=LV_GLOBAL(0x8deee4),*grid=LV_GLOBAL(0x8dee94);if(!shade||!grid||!lv_valid(shade)||!lv_valid(grid))return;
 if(!pure_order_loaded){char b[96];pure_order_default(pure_order);if(textread("/usr/data/disc-runtime/module-data/disc-bt-meter/pure-toolbar-order",b,sizeof(b)))pure_order_parse(b,pure_order);pure_order_loaded=1;}
 if(shade!=pure_shade){pure_shade=shade;lv_add_cb(shade,pure_deleted,33,0);lv_add_cb(shade,pure_edit_long,5,0);lv_add_cb(shade,pure_shade_event,3,0);pure_fold=-1;}
 if(grid!=pure_grid){pure_grid=grid;lv_add_cb(grid,pure_deleted,33,0);lv_add_cb(grid,pure_edit_long,5,0);pure_fold=-1;}
 if(!pure_installed)pure_grid_flags=*(unsigned *)((char *)grid+36)&(16|0x2000|0x4000);
 pure_events(shade,0x47f74c,pure_shade_event,1);pure_events(grid,0x47fa6c,pure_grid_event,1);
 int expanded=!!*(unsigned char *)0x8dee8c;
 if(pure_fold!=expanded){
  FN(0x51b4b8,int (*)(void *,void *))(grid,(void *)0x47f740);FN(0x51b4b8,int (*)(void *,void *))(grid,pure_height_set);
  int target=expanded?324:70;if(pure_fold<0)pure_height_set(grid,target);else animate(grid,pure_height_set,pure_height,target);
  pure_fold=expanded;FN(0x4f15ac,void (*)(LvObj *,int,int,int))(shade,0,pure_scroll(shade),0);
  FN(0x4f02c8,void (*)(LvObj *,unsigned))(shade,expanded?12:0);FN(0x4f0268,void (*)(LvObj *,unsigned))(shade,0);padbottom(shade,30,0);
 }
 lv_clear_flag(grid,16);lv_add_flag(grid,0x2000|0x4000);pure_eq_bind(grid);
 int *gb=(int *)((char *)grid+20);if(gb[3]-gb[1]+1!=pure_height)lv_size(grid,360,pure_height);
 LvObj *bright=LV_GLOBAL(0x8deeb4),*footer=bright&&lv_valid(bright)?parent(bright):0;
 if(footer&&lv_valid(footer)){
  if(footer!=pure_footer){pure_footer=footer;lv_add_cb(footer,pure_deleted,33,0);}
  if(!pure_installed)pure_footer_flags=*(unsigned *)((char *)footer+36)&(16|0x2000|0x4000);
  int *fb=(int *)((char *)footer+20),*sb=(int *)((char *)shade+20),target=80+pure_height+10;
  if(fb[1]!=sb[1]+target-pure_scroll(shade))lv_align(footer,1,0,target);
  lv_add_flag(footer,0x2000|0x4000);lv_clear_flag(footer,16);
 }
 if(!pure_button){ensurefonts();control(grid,&pure_button,&pure_icon,&pure_caption,247,84,open_dsp,&dsp_icon_0,"DSP");
  lv_size(pure_caption,120,20);lv_align(pure_caption,1,219,148);lv_add_cb(pure_button,pure_deleted,33,0);lv_add_cb(pure_caption,pure_deleted,33,0);
 }
 if(!pure_usb_button){ensurefonts();control(grid,&pure_usb_button,&pure_usb_icon,&pure_usb_caption,148,252,usb_click,&usb_icon_0,"USB 输出");lv_add_cb(pure_usb_button,pure_deleted,33,0);lv_add_cb(pure_usb_caption,pure_deleted,33,0);pure_usb_seen=-1;}
 if(!pure_editing)pure_positions(grid,1);
 int usb_on=(!strcmp(usb_text,"USB 输出 开")||!strcmp(usb_text,"USB 已连接"))?1:usb_phase&&usb_phase!=5?(now()/500)&1:0;
 if(pure_usb_icon&&usb_on!=pure_usb_seen){src(pure_usb_icon,usb_on?&usb_icon_1:&usb_icon_0);pure_usb_seen=usb_on;}
 /* The stock pure toolbar is icon-only; do not add one isolated caption. */
 show(pure_button,expanded||pure_order_position(pure_order,9)<3);show(pure_caption,0);show(pure_usb_button,expanded||pure_order_position(pure_order,10)<3);show(pure_usb_caption,0);pure_installed=1;pure_edit_bind();pure_edit_prepare();
}
