#include "pure_toolbar_order.h"
#include "pure_control_callbacks.h"
static unsigned pure_order[11],pure_saved_order[11],pure_order_loaded;
static int pure_drag=-1,pure_drag_dx,pure_drag_dy,pure_pointer_x,pure_pointer_y,pure_blank_press,pure_blank_moved;
static unsigned pure_drag_scroll_at,pure_notice_until;
static int pure_from_x[11],pure_from_y[11],pure_to_x[11],pure_to_y[11],pure_progress[11];
static LvObj *pure_control(unsigned id){return id==10?pure_usb_button:id==9?pure_button:FN(0x4f8850,LvObj *(*)(LvObj *,int))(pure_grid,(int)id);}
static int pure_control_id(LvObj *target){
 for(unsigned depth=0;target&&depth<3;depth++,target=parent(target))for(unsigned i=0;i<11;i++)if(target==pure_control(i))return (int)i;
 return -1;
}
static int pure_wiggle(unsigned id){const int offset[8]={0,1,2,1,0,-1,-2,-1};return pure_editing&&pure_drag!=(int)id?offset[(now()/70+id)&7]:0;}
static void pure_control_move(LvObj *o,int progress){int id=pure_control_id(o);if(id<0)return;pure_progress[id]=progress;
 int x=pure_from_x[id]+(pure_to_x[id]-pure_from_x[id])*progress/256,y=pure_from_y[id]+(pure_to_y[id]-pure_from_y[id])*progress/256;
 lv_align(o,1,x+pure_wiggle((unsigned)id),y);
}
static void pure_order_layout(int animated){
 int *grid=(int *)((char *)pure_grid+20);
 for(unsigned i=0;i<11;i++){LvObj *o=pure_control(i);if(!o||(int)i==pure_drag)continue;unsigned slot=pure_order_position(pure_order,i);
  FN(0x51b4b8,int (*)(void *,void *))(o,pure_control_move);
  int *box=(int *)((char *)o+20);pure_from_x[i]=box[0]-grid[0];pure_from_y[i]=box[1]-grid[1];pure_to_x[i]=pure_slot_x(slot);pure_to_y[i]=pure_slot_y(slot);
  pure_progress[i]=animated?0:256;if(animated)animate(o,pure_control_move,0,256);else pure_control_move(o,256);
 }
}
static void pure_stock_edit_event(void *e){if(pure_editing)return;
 FN(lv_event_code(e)==5?0x47f8f0:0x480228,void (*)(void *))(e);
}
static void pure_block_controls(int block){for(unsigned i=0;i<9;i++){
 LvObj *o=pure_control(i);if(!o)continue;unsigned *spec=*(unsigned **)((char *)o+8);if(!spec)continue;
 pure_control_event_patch((unsigned *)spec[3],spec[8]>>16,block,(unsigned)pure_stock_edit_event);
}}
static int pure_order_save(void){char b[96];int n=snprintf(b,sizeof(b),"%u %u %u %u %u %u %u %u %u %u %u\n",pure_order[0],pure_order[1],pure_order[2],pure_order[3],pure_order[4],pure_order[5],pure_order[6],pure_order[7],pure_order[8],pure_order[9],pure_order[10]);
 const char *next="/usr/data/disc-runtime/module-data/disc-bt-meter/pure-toolbar-order.new",*dest="/usr/data/disc-runtime/module-data/disc-bt-meter/pure-toolbar-order";
 int f=open(next,1|0x100|0x200,0600);if(f<0)return 0;int ok=write(f,b,n)==n;close(f);return ok&&!rename(next,dest);
}
static void pure_edit_finish(int save){if(!pure_editing)return;pure_drag=-1;int ok=!save||pure_order_save();
 if(!save||!ok)for(unsigned i=0;i<11;i++)pure_order[i]=pure_saved_order[i];
 pure_editing=0;pure_tracking=0;pure_blank_press=0;control_target=0;control_moved=1;pure_block_controls(0);
 for(unsigned i=0;i<11;i++){LvObj *o=pure_control(i);if(!o)continue;FN(0x51b4b8,int (*)(void *,void *))(o,pure_control_move);lv_border(o,0,0);lv_radius(o,i>=9?32:0,0);opa(o,0,0);}
 lv_add_flag(pure_shade,16);pure_order_layout(0);show(pure_edit_hint,0);
 if(!ok&&pure_edit_hint){lv_text(pure_edit_hint,"排序保存失败，已恢复");pure_notice_until=now()+2000;show(pure_edit_hint,1);}
}
static void pure_edit_done(void *e){if(pure_editing&&lv_event_code(e)==7)pure_edit_finish(1);}
static void pure_edit_begin(void){if(pure_editing)return;
 for(unsigned i=0;i<11;i++)pure_saved_order[i]=pure_order[i];pure_editing=1;pure_drag=-1;pure_tracking=0;pure_blank_press=0;pure_block_controls(1);
 for(unsigned i=0;i<11;i++){LvObj *o=pure_control(i);if(o){lv_radius(o,32,0);lv_border(o,1,0);lv_bg(o,0xff1b3044,0);opa(o,128,0);}}
 if(!pure_edit_hint){ensurefonts();pure_edit_hint=lv_label(pure_shade);stylezero(pure_edit_hint);smallfont(pure_edit_hint,font16);lv_size(pure_edit_hint,300,18);lv_align(pure_edit_hint,1,30,18);textalign(pure_edit_hint,2,0);lv_color(pure_edit_hint,0xff54baff,0);lv_clear_flag(pure_edit_hint,16);lv_add_flag(pure_edit_hint,2);lv_add_cb(pure_edit_hint,pure_deleted,33,0);lv_add_cb(pure_edit_hint,pure_edit_done,7,0);}
 lv_text(pure_edit_hint,"拖动排序 · 点此或空白完成");show(pure_edit_hint,1);pure_order_layout(0);
}
static void pure_edit_long(void *e){
 if(*(unsigned *)0x8e16d4!=0||!active||!pure_installed||!*(unsigned char *)0x8dee8c)return;
 LvObj *target=lv_event_target(e);if(target==pure_shade||target==pure_grid)pure_edit_begin();
}
static void pure_drag_update(void){if(pure_drag<0)return;int *grid=(int *)((char *)pure_grid+20);
 int x=pure_pointer_x-pure_drag_dx-grid[0],y=pure_pointer_y-pure_drag_dy-grid[1];if(x<0)x=0;if(x>296)x=296;if(y<0)y=0;if(y>260)y=260;
 lv_align(pure_control((unsigned)pure_drag),1,x,y);
 if(pure_order_move(pure_order,(unsigned)pure_drag,pure_nearest_slot(x,y)))pure_order_layout(1);
}
static void pure_edit_touch(void *e,int *point){unsigned code=lv_event_code(e);LvObj *target=lv_event_target(e);
 if(code==1){int id=pure_control_id(target);pure_pointer_x=point[0];pure_pointer_y=point[1];pure_down_x=point[0];pure_down_y=point[1];pure_blank_moved=0;pure_blank_press=id<0&&(target==pure_grid||target==pure_shade);
  if(id>=0){pure_drag=id;LvObj *o=pure_control((unsigned)id);int *box=(int *)((char *)o+20);pure_drag_dx=point[0]-box[0];pure_drag_dy=point[1]-box[1];pure_drag_scroll_at=now();FN(0x51b4b8,int (*)(void *,void *))(o,pure_control_move);lv_clear_flag(pure_shade,16);control_target=0;control_moved=1;}
 }else if(code==2){int dx=point[0]-pure_down_x,dy=point[1]-pure_down_y;if(dx*dx+dy*dy>196)pure_blank_moved=1;pure_pointer_x=point[0];pure_pointer_y=point[1];pure_drag_update();}
 else if(code==8||code==3){
  if(pure_drag>=0){pure_drag=-1;lv_add_flag(pure_shade,16);control_target=0;control_moved=1;pure_order_layout(1);pure_blank_press=0;}
  else if(code==8&&pure_blank_press&&!pure_blank_moved)pure_edit_finish(1);
  pure_blank_press=0;
 }
}
static void pure_edit_prepare(void){
 if(pure_editing&&((*(unsigned *)((char *)pure_shade+36)&1)||*(int *)((char *)pure_shade+24)<=-360))pure_edit_finish(1);
 if(pure_editing){
  if(pure_drag>=0){unsigned t=now();if(t-pure_drag_scroll_at>=16){pure_drag_scroll_at=t;int scroll=pure_scroll(pure_shade),to=scroll;
    if(pure_pointer_y<44)to-=8;else if(pure_pointer_y>316)to+=8;if(to<0)to=0;if(to>244)to=244;
    if(to!=scroll){FN(0x4f15ac,void (*)(LvObj *,int,int,int))(pure_shade,0,scroll-to,0);pure_drag_update();}
  }}
  for(unsigned i=0;i<11;i++)if((int)i!=pure_drag)pure_control_move(pure_control(i),pure_progress[i]);
 }
 if(pure_edit_hint&&!pure_editing&&(!pure_notice_until||now()>=pure_notice_until)){show(pure_edit_hint,0);pure_notice_until=0;}
}
static void pure_edit_object_deleted(LvObj *o){
 if(o==pure_edit_hint)pure_edit_hint=0;
 if(o==pure_grid||o==pure_shade){if(pure_editing)for(unsigned i=0;i<11;i++)pure_order[i]=pure_saved_order[i];pure_editing=0;pure_drag=-1;pure_blank_press=0;if(o==pure_shade)pure_edit_hint=0;else show(pure_edit_hint,0);}
}

/* Stock grid handlers do not forward all pointer events to their parent.
 * Bind the edit gesture to the actual grid/buttons; avoid handling a bubbled
 * event twice. This also works when the shade's native bubbling changes. */
static void pure_edit_direct(void *e){
 if(!pure_editing)return;
 LvObj *target=lv_event_target(e),*current=FN(0x4e392c,LvObj *(*)(const void *))(e);
 if(current==pure_grid&&target!=pure_grid)return;
 int pt[2];void *input=FN(0x4e42e8,void *(*)(void))();if(!input)return;
 FN(0x4e45bc,void (*)(void *,int *))(input,pt);pure_edit_touch(e,pt);
}
static void pure_edit_bind_one(LvObj *o){
 if(!o)return;unsigned *spec=*(unsigned **)((char *)o+8);if(!spec)return;
 unsigned n=spec[8]>>16,*events=(unsigned *)spec[3];if(n>64)return;
 for(unsigned i=0;events&&i<n;i++)if(events[i*3]==(unsigned)pure_edit_direct)return;
 lv_add_cb(o,pure_edit_direct,1,0);lv_add_cb(o,pure_edit_direct,2,0);
 lv_add_cb(o,pure_edit_direct,8,0);lv_add_cb(o,pure_edit_direct,3,0);
}
static void pure_edit_bind(void){pure_edit_bind_one(pure_grid);for(unsigned i=0;i<11;i++)pure_edit_bind_one(pure_control(i));}
