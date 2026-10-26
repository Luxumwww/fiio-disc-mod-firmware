/* Same native controls and callbacks; sort positions by stable owned IDs.
 * Separate USB-DAC/BT preferences. No native child-index identification. */
#include "visual_toolbar_order.h"
extern int fsync(int);
static unsigned vt_orders[2][6],vt_loaded[2],vt_saved[6],vt_mode,vt_notice_until;
static LvObj *vt_buttons[6],*vt_labels[6],*vt_shade,*vt_clip,*vt_hint,*vt_owner;
/* The native shade is dragged manually, so LVGL can still emit LONG_PRESSED
 * while it moves. Track the whole press, not just the final finger position. */
static struct {LvObj *target,*owner;int x,y,shade_y,moved;} vt_hold;
static int vt_drag=-1,vt_dx,vt_dy,vt_down_x,vt_down_y,vt_blank,vt_moved,vt_arming,vt_was_folded,vt_layout_dirty;
static int vt_from_x[6],vt_from_y[6],vt_to_x[6],vt_to_y[6],vt_progress[6];
static unsigned vt_index(void){return vt_mode==3?0:1;}
static void vt_event(void *),vt_deleted(void *),vt_hint_click(void *);
static void vt_native_gate(void *e){if(!vt_editing)FN(0x47f74c,void (*)(void *))(e);}
static void vt_gate(int install){if(!vt_shade||!lv_valid(vt_shade))return;unsigned *s=*(unsigned **)((char *)vt_shade+8);if(!s)return;unsigned n=s[8]>>16,*e=(unsigned *)s[3];if(n>64)return;for(unsigned i=0;e&&i<n;i++)if(e[i*3]==(install?0x47f74c:(unsigned)vt_native_gate))e[i*3]=install?(unsigned)vt_native_gate:0x47f74c;}
static int vt_id(LvObj *o){if(!o)return -1;for(unsigned i=0;i<6;i++)if(o==vt_buttons[i]||o==vt_labels[i])return (int)i;return -1;}
static int vt_wiggle(unsigned id){static const int a[8]={0,1,2,1,0,-1,-2,-1};return vt_editing&&vt_drag!=(int)id?a[(now()/70+id)&7]:0;}
static void vt_move(LvObj *o,int p){int id=vt_id(o);if(id<0||o!=vt_buttons[id])return;vt_progress[id]=p;int x=vt_from_x[id]+(vt_to_x[id]-vt_from_x[id])*p/256+vt_wiggle((unsigned)id),y=vt_from_y[id]+(vt_to_y[id]-vt_from_y[id])*p/256;lv_align(o,1,x,y);if(vt_labels[id])lv_align(vt_labels[id],1,x-28,y+66);}
static void vt_layout(int animate_positions){if(!vt_clip||!lv_valid(vt_clip))return;int *box=(int *)((char *)vt_clip+20);for(unsigned i=0;i<6;i++){LvObj *o=vt_buttons[i];if(!o||!lv_valid(o)||(int)i==vt_drag)continue;unsigned slot=vt_order_position(vt_orders[vt_index()],i);FN(0x51b4b8,int (*)(void *,void *))(o,vt_move);int *b=(int *)((char *)o+20);vt_from_x[i]=b[0]-box[0];vt_from_y[i]=b[1]-box[1];vt_to_x[i]=vt_slot_x(slot);vt_to_y[i]=vt_slot_y(slot);vt_progress[i]=animate_positions?0:256;if(animate_positions)animate(o,vt_move,0,256);else vt_move(o,256);}vt_layout_dirty=0;}
static const char *vt_order_path(void){return vt_mode==3?"/usr/data/disc-runtime/module-data/disc-bt-meter/bt-toolbar-order":"/usr/data/disc-runtime/module-data/disc-bt-meter/usbdac-toolbar-order";}
static int vt_save(void){char b[64],tmp[192];unsigned *o=vt_orders[vt_index()];int n=snprintf(b,sizeof b,"%u %u %u %u %u %u\n",o[0],o[1],o[2],o[3],o[4],o[5]);snprintf(tmp,sizeof tmp,"%s.new",vt_order_path());int f=open(tmp,1|0x100|0x200,0600);if(f<0)return 0;int ok=write(f,b,(unsigned)n)==n&&!fsync(f);close(f);return ok&&!rename(tmp,vt_order_path());}
static void vt_finish(int save){vt_hold.target=0;if(!vt_editing)return;unsigned index=vt_index();int changed=memcmp(vt_saved,vt_orders[index],sizeof vt_saved)!=0,ok=!save||!changed||vt_save();if(!save||!ok)for(unsigned i=0;i<6;i++)vt_orders[index][i]=vt_saved[i];vt_editing=0;vt_drag=-1;vt_blank=vt_arming=0;control_target=0;control_moved=1;vt_gate(0);if(vt_mode==3)folded=vt_was_folded;else dac_folded=vt_was_folded;
 for(unsigned i=0;i<6;i++)if(vt_buttons[i]&&lv_valid(vt_buttons[i])){FN(0x51b4b8,int (*)(void *,void *))(vt_buttons[i],vt_move);lv_border(vt_buttons[i],0,0);lv_radius(vt_buttons[i],32,0);opa(vt_buttons[i],0,0);}vt_layout(0);show(vt_hint,0);vt_notice_until=0;if(!ok&&vt_hint){lv_text(vt_hint,"排序保存失败，已恢复");vt_notice_until=now()+2000;show(vt_hint,1);}}
static void vt_begin(void){if(vt_editing||!active||!menu_open||!vt_shade||!vt_clip||shade_visible_height(vt_shade)!=360)return;for(unsigned i=0;i<6;i++)if(!vt_buttons[i]||!vt_labels[i])return;
 for(unsigned i=0;i<6;i++)vt_saved[i]=vt_orders[vt_index()][i];vt_owner=lv_active_screen(lv_disp_default());vt_was_folded=vt_mode==3?folded:dac_folded;if(vt_mode==3)folded=0;else dac_folded=0;vt_editing=1;vt_drag=-1;vt_blank=0;control_target=0;control_moved=1;vt_gate(1);
 for(unsigned i=0;i<6;i++){lv_border(vt_buttons[i],1,0);lv_radius(vt_buttons[i],32,0);lv_bg(vt_buttons[i],0xff1b3044,0);opa(vt_buttons[i],128,0);}if(!vt_hint){ensurefonts();vt_hint=lv_label(vt_shade);stylezero(vt_hint);smallfont(vt_hint,font16);lv_size(vt_hint,320,18);lv_align(vt_hint,1,20,18);textalign(vt_hint,2,0);lv_color(vt_hint,0xff54baff,0);lv_add_flag(vt_hint,2);lv_add_cb(vt_hint,vt_hint_click,7,0);lv_add_cb(vt_hint,vt_deleted,33,0);}lv_text(vt_hint,"拖动排序 · 点此或空白完成");show(vt_hint,1);vt_layout(0);}
static void vt_hint_click(void *e){if(lv_event_code(e)==7&&vt_editing)vt_finish(1);}
static void vt_start_drag(int id,const int pt[2]){vt_drag=id;int *b=(int *)((char *)vt_buttons[id]+20);vt_dx=pt[0]-b[0];vt_dy=pt[1]-b[1];FN(0x51b4b8,int (*)(void *,void *))(vt_buttons[id],vt_move);}
static void vt_drag_update(const int pt[2]){if(vt_drag<0||!vt_clip)return;int *b=(int *)((char *)vt_clip+20),x=pt[0]-vt_dx-b[0],y=pt[1]-vt_dy-b[1];if(x<0)x=0;if(x>296)x=296;if(y<0)y=0;if(y>98)y=98;lv_align(vt_buttons[vt_drag],1,x,y);lv_align(vt_labels[vt_drag],1,x-28,y+66);if(vt_order_move(vt_orders[vt_index()],(unsigned)vt_drag,vt_nearest_slot(x,y)))vt_layout(1);}
static void vt_event(void *e){LvObj *target=lv_event_target(e),*current=FN(0x4e392c,LvObj *(*)(const void *))(e);if(current!=target||!active||!menu_open||*(unsigned *)0x8e16d4!=vt_mode)return;int id=vt_id(target);if(id<0&&target!=vt_clip&&target!=vt_shade)return;unsigned code=lv_event_code(e);int pt[2];void *input=FN(0x4e42e8,void *(*)(void))();if(!input)return;FN(0x4e45bc,void (*)(void *,int *))(input,pt);
 if(!vt_editing){
  if(code==1){vt_hold.target=shade_visible_height(vt_shade)==360?target:0;vt_hold.owner=lv_active_screen(lv_disp_default());vt_hold.x=pt[0];vt_hold.y=pt[1];vt_hold.shade_y=vt_shade?*(int *)((char *)vt_shade+24):0;vt_hold.moved=0;}
  else if((code==2||code==5)&&vt_hold.target==target){
   int dx=pt[0]-vt_hold.x,dy=pt[1]-vt_hold.y;
   if(dx*dx+dy*dy>100||shade_visible_height(vt_shade)!=360||*(int *)((char *)vt_shade+24)!=vt_hold.shade_y||lv_active_screen(lv_disp_default())!=vt_hold.owner)vt_hold.moved=1;
   if(code==5&&!vt_hold.moved){vt_hold.target=0;vt_begin();if(vt_editing)vt_arming=1;}
  }else if(code==8||code==3)vt_hold.target=0;
  return;
 }
 if(vt_arming){if(code==8||code==3)vt_arming=0;return;}
 if(code==1){vt_down_x=pt[0];vt_down_y=pt[1];vt_moved=0;vt_blank=id<0;if(id>=0)vt_start_drag(id,pt);}
 else if(code==2){int dx=pt[0]-vt_down_x,dy=pt[1]-vt_down_y;if(dx*dx+dy*dy>196)vt_moved=1;vt_drag_update(pt);}
 else if(code==8||code==3){if(vt_drag>=0){vt_drag=-1;vt_layout(1);}else if(code==8&&vt_blank&&!vt_moved)vt_finish(1);vt_blank=0;control_target=0;control_moved=1;}}
static void vt_deleted(void *e){LvObj *o=lv_event_target(e);int relevant=o==vt_shade||o==vt_clip;if(o==vt_hold.target)vt_hold.target=0;for(unsigned i=0;i<6;i++){if(o==vt_buttons[i]){vt_buttons[i]=0;relevant=1;}if(o==vt_labels[i]){vt_labels[i]=0;relevant=1;}}if(o==vt_hint)vt_hint=0;if(o==vt_shade)vt_shade=0;if(o==vt_clip)vt_clip=0;if(relevant){vt_finish(0);vt_layout_dirty=1;}}
static void vt_bind(LvObj *o){if(!o||!lv_valid(o))return;unsigned *s=*(unsigned **)((char *)o+8);if(!s)return;unsigned n=s[8]>>16,*e=(unsigned *)s[3];if(n>64)return;for(unsigned i=0;e&&i<n;i++)if(e[i*3]==(unsigned)vt_event)return;const unsigned codes[]={1,2,3,5,8};for(unsigned i=0;i<5;i++)lv_add_cb(o,vt_event,codes[i],0);lv_add_cb(o,vt_deleted,33,0);}
static void vt_guard(unsigned mode){if(!active||mode!=vt_mode||!vt_shade||shade_visible_height(vt_shade)!=360)vt_hold.target=0;if(vt_editing&&(!active||mode!=vt_mode||lv_active_screen(lv_disp_default())!=vt_owner||!vt_clip||!vt_shade||shade_visible_height(vt_shade)!=360))vt_finish(0);if(!vt_editing&&vt_mode&&mode!=vt_mode){show(vt_hint,0);vt_layout_dirty=1;}if(vt_notice_until&&now()>=vt_notice_until){show(vt_hint,0);vt_notice_until=0;}}
static void vt_prepare(unsigned mode,LvObj *shade,LvObj *clip,LvObj **buttons,LvObj **labels){if((mode!=1&&mode!=3)||!shade||!clip)return;
 if(vt_mode!=mode||vt_shade!=shade||vt_clip!=clip){vt_finish(0);if(vt_hint&&lv_valid(vt_hint))show(vt_hint,0);vt_mode=mode;vt_shade=shade;vt_clip=clip;vt_layout_dirty=1;}
 unsigned index=vt_index();if(!vt_loaded[index]){char b[64];vt_order_default(vt_orders[index]);if(textread(vt_order_path(),b,sizeof b))vt_order_parse(b,vt_orders[index]);vt_loaded[index]=1;}
 for(unsigned i=0;i<6;i++){if(vt_buttons[i]!=buttons[i]||vt_labels[i]!=labels[i])vt_layout_dirty=1;vt_buttons[i]=buttons[i];vt_labels[i]=labels[i];vt_bind(buttons[i]);vt_bind(labels[i]);}lv_add_flag(clip,2);vt_bind(clip);vt_bind(shade);
 if(vt_layout_dirty&&!vt_editing)vt_layout(0);if(vt_editing)for(unsigned i=0;i<6;i++)if((int)i!=vt_drag)vt_move(vt_buttons[i],vt_progress[i]);}
