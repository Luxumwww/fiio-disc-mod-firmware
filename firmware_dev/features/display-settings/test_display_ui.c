/* Real display_ui.h exercised against a bounded MIPS32 factory-UI model.
 * This checks ownership/return invariants, not actual LVGL font rendering. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef struct Obj LvObj;
typedef struct {LvObj *target;unsigned code,data;} Event;
struct Obj {
 unsigned klass;LvObj *parent;unsigned *spec;unsigned spare[6];unsigned flags,unused;
 const char *text;int valid,scroll,selected,n,events;LvObj *children[48];
 struct {void (*fn)(void *);unsigned code;} cb[16];unsigned attr[9],event_data[3];
};
static LvObj pool[320],*active,*page,*stock_rows[16],*native_bright,*native_off,*bar;
static unsigned used,mode=0,focus_count,layer_clear_count;
static int reduction,fail_save;static const char *error="";static void (*native_return)(void);
enum {DISC_LV_HIDDEN=1,DISC_LV_CLICKABLE=2,DISC_LV_SCROLLABLE=16,DISC_LV_CLICKED=7,DISC_LV_DELETE=33};
static void require(int value,const char *why){if(!value){printf("FAIL %s\n",why);fflush(stdout);__builtin_trap();}}
static LvObj *lv_create(LvObj *parent){
 require(used<320,"object pool bound");LvObj *o=&pool[used++];memset(o,0,sizeof *o);o->valid=1;o->parent=parent;o->spec=o->attr;o->flags=18;
 if(parent){require(parent->n<48,"child bound");parent->children[parent->n++]=o;}return o;
}
static int lv_valid(const LvObj *o){return o&&o->valid;}
static void *lv_disp_default(void){return (void *)1;}
static LvObj *lv_active_screen(void *d){(void)d;return active;}
static LvObj *lv_event_target(void *e){return ((Event *)e)->target;}
static int stationary_clicked(void *e){return ((Event *)e)->code==DISC_LV_CLICKED;}
static unsigned child_count(LvObj *o){return (unsigned)o->n;}
static LvObj *child(LvObj *o,int n){return o&&n>=0&&n<o->n?o->children[n]:0;}
static void lv_add_flag(LvObj *o,unsigned f){o->flags|=f;}
static void lv_clear_flag(LvObj *o,unsigned f){o->flags&=~f;}
static void lv_text(LvObj *o,const char *s){o->text=s;}
static void lv_add_cb(LvObj *o,void (*fn)(void *),unsigned code,void *data){
 (void)data;require(o->events<16,"event bound");o->cb[o->events].fn=fn;o->cb[o->events++].code=code;
}
static void delete_one(LvObj *o){
 if(!lv_valid(o))return;for(int i=0;i<o->n;i++)delete_one(o->children[i]);
 Event event={o,DISC_LV_DELETE,0};for(int i=0;i<o->events;i++)if(o->cb[i].code==DISC_LV_DELETE)o->cb[i].fn(&event);o->valid=0;
}
static void screen_delete(LvObj **o){delete_one(*o);*o=0;}
static void screen_load(LvObj *o){require(lv_valid(o),"load live screen");active=o;}
static void screen_register(LvObj *o){(void)o;}
static void protect_tap(LvObj *o){(void)o;}
static void move_index(LvObj *o,int index){
 LvObj *p=o->parent;int from=0;while(from<p->n&&p->children[from]!=o)from++;require(from<p->n,"move child exists");
 if(from>index)memmove(p->children+index+1,p->children+index,(unsigned)(from-index)*sizeof(void *));
 else memmove(p->children+from,p->children+from+1,(unsigned)(index-from)*sizeof(void *));p->children[index]=o;
}
static void original_row(LvObj *p,const char *text,unsigned data,void (*fn)(void *),const int *layout){
 (void)layout;LvObj *o=lv_create(p);lv_text(lv_create(o),text);lv_create(o);
 o->attr[8]=1u<<16;o->attr[3]=(unsigned)o->event_data;o->event_data[0]=(unsigned)fn;o->event_data[1]=data;o->event_data[2]=DISC_LV_CLICKED;
}
static LvObj *choice_row(LvObj *p,const char *s,unsigned data,void (*fn)(void *)){unsigned n=child_count(p);original_row(p,s,data,fn,0);return child(p,(int)n);}
static void choice_check(LvObj *o,int checked){o->selected=checked;}
static void native_title(LvObj *p,const char *text,unsigned color){(void)color;LvObj *o=lv_create(p);lv_text(lv_create(o),text);}
static void toolbox_clear_native_layers(void){layer_clear_count++;}
static int ds_get_reduction(void){return reduction;}
static int ds_set_reduction(int v){if(fail_save){error="保存失败，请重试";return 0;}reduction=v;error="";return 1;}
static const char *ds_error_text(void){return error;}
static void no_style(){ }
static void focus(LvObj *o,int anim){(void)anim;require(lv_valid(o),"focus live");focus_count++;}
static void *event_user(void *e){return (void *)((Event *)e)->data;}
static int gesture(void *e){return ((Event *)e)->code==8?3:0;}
static void native_brightness(void (*back)(void)){native_return=back;native_bright=lv_create(0);screen_load(native_bright);}
static void native_screen_off(void (*back)(void)){native_return=back;native_off=lv_create(0);screen_load(native_off);}
static void status_hide(void){bar->flags|=1;}
static void status_show(void){bar->flags&=~1u;}
static void *fake_fn(unsigned a){
 switch(a){
 case 0x46d5c0:return choice_row;case 0x46d848:return choice_check;
 case 0x4f273c:return focus;case 0x4e394c:return event_user;case 0x488860:return gesture;
 case 0x471108:return native_brightness;case 0x473054:return native_screen_off;
 case 0x479090:return status_hide;case 0x4790b0:return status_show;
 default:return no_style;
 }
}
static LvObj **fake_global(unsigned a){
 switch(a){case 0x8de408:return &bar;case 0x8de04c:return &native_bright;case 0x8de144:return &native_off;default:require(0,"known global");return 0;}
}
#define FN(a,t) ((t)fake_fn((unsigned)(a)))
#define LV_GLOBAL(a) (*fake_global(a))
#define lv_size(o,w,h) ((void)(o),(void)(w),(void)(h))
#define lv_radius(o,w,h) lv_size(o,w,h)
#define lv_border(o,w,h) lv_size(o,w,h)
#define lv_bg(o,w,h) lv_size(o,w,h)
#define bg_opa(o,w,h) lv_size(o,w,h)
#define pad_left(o,w,h) lv_size(o,w,h)
#define pad_right(o,w,h) lv_size(o,w,h)
#define pad_top(o,w,h) lv_size(o,w,h)
#define pad_bottom(o,w,h) lv_size(o,w,h)
/* Production mode global is the only non-object direct global in this UI.
 * Map its verified address in the QEMU test; no device is accessed. */
#include "display_ui.h"
extern void *mmap(void *,unsigned,int,int,int,long);
static void set_mode(unsigned v){*(unsigned *)0x8e16d4=v;mode=v;}
int main(void){
 require(sizeof(void *)==4&&__builtin_offsetof(LvObj,text)==44&&__builtin_offsetof(LvObj,flags)==36,"verified 32-bit model layout");
 require(mmap((void *)0x8e1000,4096,3,0x812,-1,0)==(void *)0x8e1000,"map model mode global");set_mode(0);
 bar=lv_create(0);bar->flags|=1;page=lv_create(0);native_title(page,"系统设置",0);page->scroll=387;
 for(int i=0;i<16;i++){unsigned n=child_count(page);original_row(page,"stock",(unsigned)i,(void (*)(void *))0x4575f0,0);stock_rows[i]=child(page,(int)n);}
 screen_load(page);display_settings_prepare();
 require(child_count(page)==18&&ds_entry&&child(page,2)==ds_entry,"folder inserted at identified brightness action");
 require((stock_rows[1]->flags&1)&&(stock_rows[2]->flags&1),"factory brightness/screen-off hidden");
 require(!(stock_rows[0]->flags&1)&&ds_has_action(stock_rows[3],3),"other original actions retained");
 ds_pending=DS_OPEN;display_settings_poll();LvObj *folder=ds_page;require(active==folder&&ds_view==0,"folder opens");folder->scroll=71;
 ds_pending=DS_BRIGHTNESS;display_settings_poll();require(active==native_bright&&ds_page==folder,"complete factory brightness child");display_settings_poll();require(ds_page==folder,"retained parent while factory child active");
 native_return();screen_delete(&native_bright);require(active==folder&&folder->scroll==71,"brightness returns to same folder and position");
 ds_pending=DS_SCREEN_OFF;display_settings_poll();require(active==native_off,"factory screen-off child");native_return();screen_delete(&native_off);require(active==folder&&folder->scroll==71,"screen-off returns same folder");
 ds_pending=DS_WHITEPOINT;display_settings_poll();require(ds_view==1&&ds_values[0]->selected,"default off radio");
 ds_pending=DS_VALUE;ds_pending_value=60;display_settings_poll();require(reduction==60&&ds_values[6]->selected&&!ds_values[0]->selected,"value save/readback");
 fail_save=1;ds_pending=DS_VALUE;ds_pending_value=70;display_settings_poll();require(reduction==60&&ds_values[6]->selected&&!ds_values[7]->selected&&!(ds_error_row->flags&1),"failure preserves actual value and exposes error");
 ds_pending=DS_BACK;display_settings_poll();require(ds_view==0,"whitepoint back to display folder");bar->flags&=~1u;
 ds_pending=DS_BACK;display_settings_poll();require(!ds_page&&active==page&&page->scroll==387&&(bar->flags&1),"system position/status restored");
 ds_pending=DS_OPEN;display_settings_poll();ds_pending=DS_BRIGHTNESS;display_settings_poll();LvObj *retained=native_bright;
 LvObj *external=lv_create(0);screen_load(external);display_settings_poll();require(!ds_page&&active==external,"unrelated page is not replaced");
 screen_load(page);ds_pending=DS_OPEN;display_settings_poll();folder=ds_page;ds_pending=DS_BRIGHTNESS;display_settings_poll();
 require(active==retained&&native_bright==retained,"owned native singleton reused after external exit");native_return();screen_delete(&native_bright);require(active==folder,"reused native child returns to current folder");
 ds_pending=DS_BACK;display_settings_poll();
 screen_load(page);ds_pending=DS_OPEN;display_settings_poll();set_mode(1);display_settings_poll();require(!ds_page&&mode==1,"mode change abandons old folder");
 require(layer_clear_count>0&&focus_count>0,"native lifecycle/focus path exercised");
 puts("PASS: factory-action binding, native child callbacks, retained parent/scroll, value readback/error, status restore, external-page/mode exit");fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
