/* Native styled submenu: UI thread only, independent mode preferences. */
#include "../audio-dsp/mode_preferences.h"
#include "dsp_order.h"
static LvObj *dsp_page,*dsp_origin,*dsp_rows[16],*dsp_pills[16],*dsp_knobs[16],*dsp_message;
static DiscToolbarStatus dsp_toolbar_status;
static LvObj *dsp_button,*dsp_glyph,*dsp_caption;
static unsigned dsp_ui_masks[4],dsp_ui_mode,dsp_ui_updated;
static int dsp_back_pending,dsp_swiping;
static unsigned dsp_ui_error,dsp_ui_error_at;
static unsigned dsp_order[DSP_ORDER_COUNT],dsp_saved_order[DSP_ORDER_COUNT];
static int dsp_drag=-1,dsp_drag_offset,dsp_drag_y;
static unsigned dsp_drag_scroll_at;
#include "../mobile-remote/native_shared_ui.h"
#include "dsp_presets.h"
#include "../dsp-resample/ui.h"
static int dsp_scroll_y(void){void *spec=*(void **)((char *)dsp_page+8);return spec?-*(int *)((char *)spec+0x14):0;}
static void dsp_row_move(LvObj *row,int y){lv_align(row,1,0,y);}
static void dsp_order_layout(int animated){
 int scroll=dsp_scroll_y();
 for(unsigned pos=0;pos<DSP_ORDER_COUNT;pos++){unsigned id=dsp_order[pos];if(id==1||(int)id==dsp_drag)continue;
  LvObj *row=dsp_rows[id];int to=DSP_ROW_TOP+(int)pos*DSP_ROW_PITCH;
  int from=*(int *)((char *)row+24)-*(int *)((char *)dsp_page+24)+scroll;
  if(from==to)continue;
  if(animated){unsigned animation[32];FN(0x51b3e0,void (*)(void *))(animation);animation[0]=(unsigned)row;animation[1]=(unsigned)dsp_row_move;animation[8]=animation[9]=(unsigned)from;animation[10]=(unsigned)to;animation[11]=160;FN(0x51b7b0,void *(*)(void *))(animation);}
  else {FN(0x51b4b8,int (*)(void *,void *))(row,dsp_row_move);dsp_row_move(row,to);}
 }
}
static int dsp_order_save(void){char b[96];int n=0;for(unsigned i=0;i<DSP_ORDER_COUNT;i++)n+=snprintf(b+n,sizeof(b)-n,"%u%s",dsp_order[i],i+1==DSP_ORDER_COUNT?"\n":" ");
 const char *tmp="/usr/data/disc-runtime/module-data/disc-audio-dsp/display-order.new",*dest="/usr/data/disc-runtime/module-data/disc-audio-dsp/display-order";
 int fd=open(tmp,1|0x100|0x200,0600);if(fd<0)return 0;int ok=write(fd,b,n)==n;close(fd);return ok&&!rename(tmp,dest);
}
static void dsp_drag_finish(int save){if(dsp_drag<0)return;
 int id=dsp_drag;dsp_drag=-1;control_moved=1;control_target=0;lv_add_flag(dsp_page,16);
 lv_bg(dsp_rows[id],0xff000000,0);lv_radius(dsp_rows[id],0,0);
 int ok=!save||dsp_order_save();if(!save||!ok)for(unsigned i=0;i<DSP_ORDER_COUNT;i++)dsp_order[i]=dsp_saved_order[i];
 dsp_order_layout(1);if(!ok)lv_text(dsp_message,"排序保存失败");
}
static void dsp_drag_update(void){int scroll=dsp_scroll_y(),y=dsp_drag_y+scroll-dsp_drag_offset;
 if(y<DSP_ROW_TOP)y=DSP_ROW_TOP;if(y>DSP_ROW_TOP+(DSP_ORDER_COUNT-1)*DSP_ROW_PITCH)y=DSP_ROW_TOP+(DSP_ORDER_COUNT-1)*DSP_ROW_PITCH;
 dsp_row_move(dsp_rows[dsp_drag],y);if(dsp_order_move(dsp_order,(unsigned)dsp_drag,dsp_order_slot(y+25)))dsp_order_layout(1);
}
static void dsp_row_touch(void *e){unsigned code=lv_event_code(e);LvObj *target=lv_event_target(e);control_touch(e);
 if(dsp_drag>=0){if(target!=dsp_rows[dsp_drag])return;
  if(code==2){int pt[2];void *input=FN(0x4e42e8,void *(*)(void))();if(!input)return;FN(0x4e45bc,void (*)(void *,int *))(input,pt);dsp_drag_y=pt[1];dsp_drag_update();}
  else if(code==8)dsp_drag_finish(1);else if(code==3)dsp_drag_finish(0);return;
 }
 if(code!=5||control_moved||control_x<60||dsp_swiping)return;
 for(unsigned i=0;i<16;i++)if(target==dsp_rows[i]){
  for(unsigned j=0;j<DSP_ORDER_COUNT;j++)dsp_saved_order[j]=dsp_order[j];
  dsp_drag=(int)i;control_moved=1;dsp_drag_y=control_y;dsp_drag_offset=control_y-*(int *)((char *)target+24);dsp_drag_scroll_at=now();
  FN(0x51b4b8,int (*)(void *,void *))(target,dsp_row_move);lv_clear_flag(dsp_page,16);
  FN(0x4f8304,void (*)(LvObj *,int))(target,(int)FN(0x4f889c,unsigned (*)(LvObj *))(dsp_page)-1);
  lv_bg(target,0xff17394e,0);opa(target,255,0);lv_radius(target,8,0);break;
 }
}
#include "dsp_lifecycle.h"
static void dsp_ui_deleted(void *e){LvObj *o=lv_event_target(e);if(o==dsp_page)dsp_forget_page();if(o==dsp_origin)dsp_origin=0;if(o==dsp_button)dsp_button=dsp_glyph=0;if(o==dsp_caption)dsp_caption=0;}
static void dsp_watch_origin(LvObj *o){
 unsigned *spec=*(unsigned **)((char *)o+8);if(spec){unsigned count=spec[8]>>16,*events=(unsigned *)spec[3];
  if(count<=128)for(unsigned i=0;events&&i<count;i++)if(events[i*3]==(unsigned)dsp_ui_deleted&&(events[i*3+2]&255)==33)return;
 }
 lv_add_cb(o,dsp_ui_deleted,33,0);
}
static void dsp_ui_read_masks(void){char b[64]={0};textread("/usr/data/disc-runtime/module-data/disc-audio-dsp/settings",b,sizeof(b));disc_dsp_read_masks(b,dsp_ui_masks);DiscChain c;unsigned enabled;int shared=dsp_shared_ui_read(&c,&enabled);if(shared){unsigned mask=0;if(shared>0&&enabled){mask=4096;for(unsigned i=0;i<c.count;i++)if(c.nodes[i].enabled&&(c.nodes[i].type<12||c.nodes[i].type==14||c.nodes[i].type==15||c.nodes[i].type==16))mask|=1u<<c.nodes[i].type;}for(unsigned i=0;i<4;i++)dsp_ui_masks[i]=mask;}}
static void dsp_ui_load(void){dsp_ui_read_masks();char order[96]={0};dsp_order_default(dsp_order);if(textread("/usr/data/disc-runtime/module-data/disc-audio-dsp/display-order",order,sizeof(order)))dsp_order_parse(order,dsp_order);}
static unsigned dsp_ui_index(void){return dsp_ui_mode==0?3:dsp_ui_mode==5?2:dsp_ui_mode==1?1:0;}
static void dsp_ui_refresh(void){
 const unsigned bit[16]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,16384,32768,65536};unsigned mask=dsp_ui_masks[dsp_ui_index()];
 for(unsigned i=0;i<16;i++)if(dsp_pills[i]){int on=(mask&bit[i])!=0;lv_bg(dsp_pills[i],on?0xff54baff:0xff555555,0);lv_align(dsp_knobs[i],1,on?27:3,3);}
 if(dsp_message){if(dsp_ui_error&&(unsigned)(now()-dsp_ui_error_at)<4000){lv_text(dsp_message,dsp_ui_error==2?"已保存16项，请先移除一项":dsp_ui_error==3?"最多同时开启8个音效":"音效未保存，请稍后重试");return;}dsp_ui_error=0;char state[640];unsigned dsd=0,rate=0,provider=0,enabled=0;if(textread("/tmp/disc-runtime/audio-dsp.state",state,sizeof(state)))for(unsigned i=0;state[i];i++)if(i==0||state[i-1]=='\n'){unsigned v;if(sscanf(state+i,"DoP_bypass=%u",&v)==1)dsd=v;else if(sscanf(state+i,"rate=%u",&v)==1)rate=v;else if(sscanf(state+i,"provider=%u",&v)==1)provider=v;else if(sscanf(state+i,"enabled=%u",&v)==1)enabled=v;}
  const char *message=!enabled||!provider?"音效模块未加载":dsd?"DSD 保持原输出，DSP 已旁路":rate>384000?"高采样率：仅压限器可用":(mask&4096u)?"自定义信号链已启用":"可同时开启多个音效";lv_text(dsp_message,message);}

}
static void dsp_ui_click(void *e){
 if(dsp_drag>=0||!control_clicked(e)||!dsp_page)return;const unsigned bit[16]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,16384,32768,65536};LvObj *target=lv_event_target(e);unsigned index=dsp_ui_index();
 for(unsigned i=0;i<16;i++)if(target==dsp_rows[i]){
  DiscChain shared;unsigned master;if(i>=13||dsp_shared_ui_read(&shared,&master)){int ok=dsp_shared_ui_toggle(i==15?DISC_WARMGLOSS_TYPE:i==14?DISC_STEREOFX_TYPE:i==13?DISC_FFHAAS_TYPE:i,index);dsp_ui_error=ok?0:dsp_shared_ui_error;dsp_ui_error_at=now();dsp_ui_read_masks();dsp_ui_refresh();return;}dsp_ui_read_masks();unsigned wanted=(dsp_ui_masks[index]&~4096u)^bit[i];char b[64];int n=snprintf(b,sizeof(b),"%u %u %u %u\n",index==0?wanted:dsp_ui_masks[0],index==1?wanted:dsp_ui_masks[1],index==2?wanted:dsp_ui_masks[2],index==3?wanted:dsp_ui_masks[3]);
  int f=open("/usr/data/disc-runtime/module-data/disc-audio-dsp/settings.new",1|0x100|0x200,0600);if(f<0){lv_text(dsp_message,"设置保存失败");return;}
  int ok=write(f,b,n)==n;close(f);if(!ok||rename("/usr/data/disc-runtime/module-data/disc-audio-dsp/settings.new","/usr/data/disc-runtime/module-data/disc-audio-dsp/settings")){lv_text(dsp_message,"设置保存失败");return;}dsp_ui_masks[index]=wanted;dsp_ui_refresh();return;
 }
}
static void dsp_ui_swipe(void *e){if(dsp_drag>=0)return;int result=FN(0x488860,int (*)(void *))(e);if(result==1)dsp_swiping=1;else if(result==3){dsp_swiping=0;dsp_back_pending=1;}else if(result==4)dsp_swiping=0;}
static void dsp_ui_poll(void){src_ui_poll();dsp_cleanup_inactive();if(!dsp_page||src_ui_active())return;
 if(dsp_drag>=0){unsigned t=now();if(t-dsp_drag_scroll_at>=16){unsigned dt=t-dsp_drag_scroll_at;if(dt>48)dt=48;dsp_drag_scroll_at=t;int scroll=dsp_scroll_y(),step=(int)dt/2,to=scroll;
 if(dsp_drag_y<44)to-=step;else if(dsp_drag_y>316)to+=step;if(to<0)to=0;if(to>DSP_ROW_TOP+DSP_ORDER_COUNT*DSP_ROW_PITCH+24-360)to=DSP_ROW_TOP+DSP_ORDER_COUNT*DSP_ROW_PITCH+24-360;
 if(to!=scroll){FN(0x4f15ac,void (*)(LvObj *,int,int,int))(dsp_page,0,scroll-to,0);dsp_drag_update();}}
 return;}
 if(dsp_back_pending){dsp_close_page(1);return;}unsigned t=now();if(t-dsp_ui_updated>=1000){dsp_ui_updated=t;dsp_ui_read_masks();dsp_ui_refresh();dsp_presets_refresh();}}
static void open_dsp(void *e){
 if(pure_editing)return;
 if(!control_clicked(e)||(!visual_mode()&&*(unsigned *)0x8e16d4!=0))return;
 dsp_cleanup_inactive();
 if(dsp_page){LvObj *shade=LV_GLOBAL(0x8deee4);if(shade&&lv_valid(shade)){show(shade,0);FN(0x4eb600,void (*)(LvObj *,int))(shade,-360);}return;}
 ensurefonts();dsp_ui_load();dsp_ui_mode=*(unsigned *)0x8e16d4;
 dsp_origin=lv_active_screen(lv_disp_default());if(dsp_origin)dsp_watch_origin(dsp_origin);dsp_toolbar_status=disc_toolbar_capture();
 FN(0x479090,void (*)(void))();LvObj *shade=LV_GLOBAL(0x8deee4);if(shade&&lv_valid(shade)){show(shade,0);FN(0x4eb600,void (*)(LvObj *,int))(shade,-360);}
 dsp_page=lv_create(0);if(!dsp_page)return;FN(0x486620,void (*)(LvObj *))(dsp_page);stylezero(dsp_page);lv_size(dsp_page,360,360);lv_bg(dsp_page,0xff000000,0);opa(dsp_page,255,0);lv_add_flag(dsp_page,2|16);padbottom(dsp_page,24,0);FN(0x4f02c8,void (*)(LvObj *,int))(dsp_page,12);
 lv_add_cb(dsp_page,dsp_ui_deleted,33,0);lv_add_cb(dsp_page,dsp_ui_swipe,1,0);lv_add_cb(dsp_page,dsp_ui_swipe,2,0);lv_add_cb(dsp_page,dsp_ui_swipe,8,0);
 FN(0x46d88c,void (*)(LvObj *,const char *,unsigned))(dsp_page,"DSP",0xb4fb);LvObj *title=FN(0x4f8850,LvObj *(*)(LvObj *,int))(dsp_page,0);if(title){lv_clear_flag(title,0x40000);lv_align(title,1,0,44);lv_color(title,0xff54baff,0);}
 const char *names[16]={"音乐厅氛围","","压限器","仿电子管","V8 混响","PurestWarm3 暖声","Dragonfly 大厅","Galactic3 银河","声场扩展","延迟 / 回声","去齿音","动态 EQ","","Haas 立体声","StereoFX 扩展","暖润光泽"};
 /* All direct children participate in native full-page scrolling. */
 for(unsigned i=0;i<16;i++){
  if(i==1||(i>=8&&i!=13&&i!=14&&i!=15))continue;int layout[3]={44,180,0};unsigned count=FN(0x4f889c,unsigned (*)(LvObj *))(dsp_page);
  FN(0x485ef8,void (*)(LvObj *,const char *,unsigned,void (*)(void *),const int *))(dsp_page,names[i],i,dsp_ui_click,layout);
  LvObj *row=FN(0x4f8850,LvObj *(*)(LvObj *,int))(dsp_page,count);dsp_rows[i]=row;lv_size(row,360,50);padtop(row,0,0);padbottom(row,0,0);lv_align(row,1,0,DSP_ROW_TOP+(int)dsp_order_position(dsp_order,i)*DSP_ROW_PITCH);lv_add_flag(row,0x4000|0x2000);lv_clear_flag(row,16|0x40000);
  lv_add_cb(row,dsp_row_touch,1,0);lv_add_cb(row,dsp_row_touch,2,0);lv_add_cb(row,dsp_row_touch,8,0);lv_add_cb(row,dsp_row_touch,3,0);lv_add_cb(row,dsp_row_touch,5,0);LvObj *arrow=FN(0x4f8850,LvObj *(*)(LvObj *,int))(row,1);show(arrow,0);
  LvObj *pill=lv_create(row);dsp_pills[i]=pill;stylezero(pill);lv_size(pill,48,24);lv_align(pill,7,257,0);lv_radius(pill,12,0);opa(pill,255,0);lv_clear_flag(pill,2|16);
  LvObj *knob=lv_create(pill);dsp_knobs[i]=knob;stylezero(knob);lv_size(knob,18,18);lv_radius(knob,9,0);lv_bg(knob,0xffffffff,0);opa(knob,255,0);lv_clear_flag(knob,2|16);
 }
 dsp_message=lv_label(dsp_page);stylezero(dsp_message);smallfont(dsp_message,font16);lv_color(dsp_message,0xffaaaaaa,0);lv_clear_flag(dsp_message,0x40000);lv_align(dsp_message,2,0,DSP_ROW_TOP+DSP_ORDER_COUNT*DSP_ROW_PITCH+12);
 LvObj *hint=lv_label(dsp_page);stylezero(hint);lv_clear_flag(hint,0x40000);smallfont(hint,font16);lv_color(hint,0xff888888,0);lv_text(hint,"长按音效可拖动排序");lv_align(hint,2,0,DSP_ROW_TOP+DSP_ORDER_COUNT*DSP_ROW_PITCH+34);
 src_ui_add_entry();dsp_presets_refresh();dsp_ui_refresh();FN(0x4e2734,void (*)(LvObj *))(dsp_page);
}

