/* Real display_ui.h exercised against a bounded MIPS32 factory-UI model.
 * This checks ownership/return invariants, not actual LVGL font rendering. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
typedef struct Obj LvObj;
typedef struct {LvObj *target;unsigned code,data;} Event;
struct Obj {
 unsigned klass;LvObj *parent;unsigned *spec;unsigned spare[6];unsigned flags,unused;
 const char *text;int valid,scroll,selected,n,events;LvObj *children[48];
 struct {void (*fn)(void *);unsigned code;} cb[16];unsigned attr[9],event_data[3];char owned_text[300];
};
static LvObj pool[3000],*active,*page,*stock_rows[16],*native_bright,*native_off,*bar;
static unsigned used,mode=0,focus_count,layer_clear_count;
static int reduction,fail_save;static const char *error="";static void (*native_return)(void);
enum {DISC_LV_HIDDEN=1,DISC_LV_CLICKABLE=2,DISC_LV_SCROLLABLE=16,DISC_LV_CLICKED=7,DISC_LV_DELETE=33};
static void require(int value,const char *why){if(!value){printf("FAIL %s\n",why);fflush(stdout);__builtin_trap();}}
static LvObj *lv_create(LvObj *parent){
 require(used<3000,"object pool bound");LvObj *o=&pool[used++];memset(o,0,sizeof *o);o->valid=1;o->parent=parent;o->spec=o->attr;o->flags=18;
 if(parent){require(parent->n<48,"child bound");parent->children[parent->n++]=o;}return o;
}
static int lv_valid(const LvObj *o){return o&&o->valid;}
static void *lv_disp_default(void){return (void *)1;}
static LvObj *lv_active_screen(void *d){(void)d;return active;}
static LvObj *lv_event_target(void *e){return ((Event *)e)->target;}
static unsigned lv_event_code(void *e){return ((Event *)e)->code;}
static int stationary_clicked(void *e){return ((Event *)e)->code==DISC_LV_CLICKED;}
static unsigned child_count(LvObj *o){return (unsigned)o->n;}
static LvObj *child(LvObj *o,int n){return o&&n>=0&&n<o->n?o->children[n]:0;}
static void lv_add_flag(LvObj *o,unsigned f){o->flags|=f;}
static void lv_clear_flag(LvObj *o,unsigned f){o->flags&=~f;}
static void lv_text(LvObj *o,const char *s){require(lv_valid(o),"text target alive");snprintf(o->owned_text,sizeof o->owned_text,"%s",s);o->text=o->owned_text;}
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
static void slider_range(LvObj *o,int low,int high){o->attr[0]=low;o->attr[1]=high;}
static void slider_set(LvObj *o,int value,int anim){(void)anim;require(value>=(int)o->attr[0]&&value<=(int)o->attr[1],"slider bounds");o->selected=value;}
static int slider_get(LvObj *o){return o->selected;}
static void *fake_fn(unsigned a){
 switch(a){
 case 0x46d5c0:return choice_row;case 0x46d848:return choice_check;
 case 0x4f273c:return focus;case 0x4e394c:return event_user;case 0x488860:return gesture;
 case 0x471108:return native_brightness;case 0x473054:return native_screen_off;
 case 0x479090:return status_hide;case 0x4790b0:return status_show;
 case 0x53342c:case 0x52cb10:return lv_create;case 0x52a754:return slider_range;case 0x52a4ec:return slider_set;case 0x52a7f0:return slider_get;default:return no_style;
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

#define lv_btn lv_create
#define lv_label lv_create
#define lv_align(o,a,x,y) ((void)(o),(void)(a),(void)(x),(void)(y))
#define lv_add_style(o,s,a) ((void)(o),(void)(s),(void)(a))
#define lv_color(o,s,a) ((void)(o),(void)(s),(void)(a))
static unsigned model_ms=10000;
static int model_clock(int which,long *t){(void)which;t[0]=model_ms/1000;t[1]=(model_ms%1000)*1000000;return 0;}
#define clock_gettime model_clock
static const char *local_path(const char *path){static char p[4][256];static unsigned slot;char *out=p[slot++%4];snprintf(out,256,"/tmp/disc-pu-ui-test/%s",strrchr(path,'/')+1);return out;}
static int model_open(const char *path,int flags,...){return open(local_path(path),flags,0600);}
static int model_access(const char *path,int mode){return access(local_path(path),mode);}
static int model_rename(const char *a,const char *b){return rename(local_path(a),local_path(b));}
static int model_unlink(const char *path){return unlink(local_path(path));}
#define open model_open
#define access model_access
#define rename model_rename
#define unlink model_unlink
#include "settings_ui.h"
#undef open
#undef access
#undef rename
#undef unlink
extern void *mmap(void *,unsigned,int,int,int,long);
static void set_mode(unsigned v){*(unsigned *)0x8e16d4=v;mode=v;}
static void act(unsigned action,unsigned arg){pu_action=action;pu_arg=arg;model_ms+=300;pu_poll();}
int main(void){
 require(sizeof(void *)==4&&__builtin_offsetof(LvObj,text)==44&&__builtin_offsetof(LvObj,flags)==36,"verified model layout");
 require(mmap((void *)0x8e1000,4096,3,0x812,-1,0)==(void *)0x8e1000,"mode global mapping");set_mode(0);
 bar=lv_create(0);bar->flags|=1;page=lv_create(0);native_title(page,"系统设置",0);page->scroll=387;
 for(int i=0;i<16;i++){unsigned n=child_count(page);original_row(page,"stock",(unsigned)i,(void (*)(void *))0x4575f0,0);stock_rows[i]=child(page,(int)n);}
 screen_load(page);pu_prepare();require(pu_entry==child(page,2)&&child_count(page)==18,"one factory entry at index2");pu_prepare();require(child_count(page)==18,"no duplicate entry");
 act(PU_OPEN,0);require(pu_stack[0]&&pu_views[0]==PU_MAIN&&active!=page,"audio folder opens");
 act(PU_DSP,0);require(pu_depth==1&&pu_node_count==16,"all actual effects listed");unsigned pick=0;while(pick<pu_node_count&&pu_node_types[pick]!=16)pick++;require(pick<pu_node_count,"warm gloss exists");
 act(PU_NODE,pick);LvObj *parameters=active;parameters->scroll=127;act(PU_PARAM,0);require(pu_depth==3&&pu_slider&&pu_views[3]==PU_EDIT,"native slider editor opens");
 act(PU_PLUS,0);model_ms+=300;pu_poll();DiscChain c;unsigned master;require(pu_chain_read(&c,&master)&&c.count==1&&c.nodes[0].type==16&&!c.nodes[0].enabled&&!master,"new parameter node remains disabled and master preserved");
 require(c.nodes[0].params[0]>.75f,"edit actually saved");act(PU_BACK,0);require(active==parameters&&parameters->scroll==127,"parameter parent position retained");
 act(PU_PARAM,0);for(unsigned i=0;i<18;i++){act(PU_PLUS,0);model_ms+=300;pu_poll();require(!pu_error[0]&&!pu_dirty&&pu_chain_read(&c,&master)&&pu_baseline==c.nodes[0].params[0],"consecutive fractional edits use serialized baseline");}act(PU_BACK,0);
 float custom=c.nodes[0].params[0];act(PU_RESET,0);require(pu_views[3]==PU_RESET_CONFIRM,"reset confirmation opens");act(PU_BACK,0);require(pu_chain_read(&c,&master)&&c.nodes[0].params[0]==custom&&active==parameters&&parameters->scroll==127,"cancel reset preserves parameters and parent position");
 act(PU_RESET,0);act(PU_RESET_APPLY,0);require(pu_depth==2&&active==parameters&&pu_chain_read(&c,&master)&&!master&&!c.nodes[0].enabled&&c.nodes[0].params[0]==.75f,"reset saves schema defaults without enabling");require(strstr(pu_message[2]->text,"已恢复")!=0,"reset success feedback");
 act(PU_RESET,0);pu_reset_snapshot[0]=.1f;act(PU_RESET_APPLY,0);require(pu_depth==3&&strstr(pu_error,"参数已变化")!=0&&pu_chain_read(&c,&master)&&c.nodes[0].params[0]==.75f,"reset rejects changed confirmation snapshot");act(PU_BACK,0);
 act(PU_PARAM,0);pu_baseline=.1f;pu_value=.9f;pu_dirty=1;pu_changed=0;act(PU_BACK,0);require(pu_depth==3&&strstr(pu_error,"参数已被修改"),"stale edit refused with error");act(PU_BACK,0);act(PU_BACK,0);act(PU_BACK,0);require(pu_depth==0,"back to main");
 act(PU_USB_OPEN,0);require(pu_views[1]==PU_USB,"USB details");act(PU_USB_RETRY,0);require(strstr(pu_usb_labels[5]->text,"请先开启")!=0,"unowned reconnect rejected");act(PU_BACK,0);
 act(PU_RG_OPEN,0);require(pu_rg_mode==0&&pu_rg_protect==1,"ReplayGain defaults off/protected");act(PU_RG_MODE,0);act(PU_RG_SELECT,2);require(pu_views[1]==PU_REPLAYGAIN&&pu_rg_mode==2,"mode persisted and returns");act(PU_RG_PREAMP,0);act(PU_MINUS,0);model_ms+=300;pu_poll();pu_rg_load();require(pu_rg_preamp==-.5f,"preamp persisted");act(PU_BACK,0);act(PU_BACK,0);bar->flags&=~1u;act(PU_BACK,0);require(active==page&&page->scroll==387&&(bar->flags&1),"original parent position/status restored");
 act(PU_OPEN,0);act(PU_DSP,0);LvObj *external=lv_create(0);screen_load(external);pu_poll();require(!pu_stack[0]&&active==external,"external navigation never stolen");screen_load(page);act(PU_OPEN,0);set_mode(3);pu_poll();require(!pu_stack[0],"mode change cleans stack");
 require(pu_state_uint("x\n", "longer_key",7)==7&&pu_state_uint("pid=-1\n","pid",7)==7,"short state/negative rejected");
 puts("PASS native UI: entry/stock rows, 4-level lifecycle, parameter autosave/disabled/master/stale, USB unowned, RG mode/preamp, return scroll/status, external navigation/mode cleanup");fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
