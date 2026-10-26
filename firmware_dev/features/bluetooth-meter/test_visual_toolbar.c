/* Real display_ui.h exercised against a bounded MIPS32 factory-UI model.
 * This checks ownership/return invariants, not actual LVGL font rendering. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
typedef struct Obj LvObj;
typedef struct {LvObj *target;unsigned code,data;LvObj *current;} Event;
struct Obj {
 unsigned klass;LvObj *parent;unsigned *spec;unsigned spare[6];unsigned flags,unused;
 const char *text;int valid,scroll,selected,n,events;LvObj *children[48];
 struct {void (*fn)(void *);unsigned code;} cb[16];unsigned attr[9],event_data[96];char owned_text[300];
};
static LvObj pool[3000],*active,*native_bright,*native_off,*bar;
static unsigned used,focus_count,layer_clear_count;
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
static void lv_add_cb(LvObj *o,void (*fn)(void *),unsigned code,void *data){require(o->events<16,"callback bound");unsigned n=o->events++;o->cb[n].fn=fn;o->cb[n].code=code;o->event_data[n*3]=(unsigned)fn;o->event_data[n*3+1]=(unsigned)data;o->event_data[n*3+2]=code;o->attr[3]=(unsigned)o->event_data;o->attr[8]=o->events<<16;}
static void delete_one(LvObj *o){
 if(!lv_valid(o))return;for(int i=0;i<o->n;i++)delete_one(o->children[i]);
 Event event={.target=o,.code=DISC_LV_DELETE};for(int i=0;i<o->events;i++)if(o->cb[i].code==DISC_LV_DELETE)((void (*)(void *))o->event_data[i*3])(&event);o->valid=0;
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
static int point[2],native_calls;
static void *current_target(void *e){Event *v=e;return v->current?v->current:v->target;}
static void *input(void){return point;}
static void getpoint(void *in,int *out){int *p=in;out[0]=p[0];out[1]=p[1];}
static int animdel(void *a,void *b){(void)a;(void)b;return 0;}
static void native_event(void *e){(void)e;native_calls++;}
static void *fake_fn(unsigned a){switch(a){case 0x4e392c:return current_target;case 0x4e42e8:return input;case 0x4e45bc:return getpoint;case 0x51b4b8:return animdel;case 0x47f74c:return native_event;default:return no_style;}}
static LvObj **fake_global(unsigned a){
 switch(a){case 0x8de408:return &bar;case 0x8de04c:return &native_bright;case 0x8de144:return &native_off;default:require(0,"known global");return 0;}
}
#define FN(a,t) ((t)fake_fn((unsigned)(a)))
#define LV_GLOBAL(a) (*fake_global(a))

static void model_size(LvObj *o,int w,int h){require(lv_valid(o),"size live");int *b=(int *)((char *)o+20);b[2]=b[0]+w-1;b[3]=b[1]+h-1;}
static void model_align(LvObj *o,int align,int x,int y){(void)align;require(lv_valid(o),"align live");int *b=(int *)((char *)o+20),w=b[2]-b[0],h=b[3]-b[1];int *p=o->parent?(int *)((char *)o->parent+20):0;b[0]=(p?p[0]:0)+x;b[1]=(p?p[1]:0)+y;b[2]=b[0]+w;b[3]=b[1]+h;}
#define lv_size model_size
#define lv_align model_align
#define lv_border(o,x,y) ((void)(o),(void)(x),(void)(y))
#define lv_radius lv_border
#define lv_bg lv_border
#define lv_color lv_border
static void opa(LvObj *o,unsigned v,unsigned selector){(void)o;(void)v;(void)selector;}
static unsigned model_ms=10000;
static unsigned now(void){return model_ms;}
static void animate(LvObj *o,void (*cb)(LvObj *,int),int from,int to){(void)from;cb(o,to);}
static void ensurefonts(void){}
static void *font16;
static void smallfont(LvObj *o,void *style){(void)o;(void)style;}
static void stylezero(LvObj *o){(void)o;}
static void textalign(LvObj *o,unsigned a,unsigned b){(void)o;(void)a;(void)b;}
static void show(LvObj *o,int yes){if(lv_valid(o)){if(yes)o->flags&=~1u;else o->flags|=1;}}
#define lv_label lv_create
static int vt_editing,active_enabled=1,menu_open=1,folded=1,dac_folded=1,fail_write;

static LvObj *control_target;static int control_moved;
static unsigned shade_visible_height(LvObj *o){if(!lv_valid(o)||(o->flags&1))return 0;int h=*(int *)((char *)o+24)+360;return h<0?0:h>360?360:(unsigned)h;}
static const char *localpath(const char *path){static char b[4][256];static unsigned i;char *p=b[i++%4];snprintf(p,256,"/tmp/disc-vt-test/%s",strrchr(path,'/')+1);return p;}
static int localopen(const char *path,int flags,...){return open(localpath(path),flags,0600);}
static int localrename(const char *a,const char *b){return fail_write?-1:rename(localpath(a),localpath(b));}
static int textread(const char *path,char *b,unsigned n){int f=open(localpath(path),0);if(f<0)return 0;int z=read(f,b,n-1);close(f);if(z<=0)return 0;b[z]=0;return z;}
#define open localopen
#define rename localrename
#define active active_enabled
#include "visual_toolbar_edit.h"
#undef active

#undef open
#undef rename
extern void *mmap(void *,unsigned,int,int,int,long);
static void setmode(unsigned mode){*(unsigned *)0x8e16d4=mode;}
static void event(LvObj *o,unsigned code,int x,int y){Event e={.target=o,.code=code};point[0]=x;point[1]=y;model_ms+=20;vt_event(&e);}
static void begin(LvObj *o){event(o,1,20,250);model_ms+=500;event(o,5,20,250);require(vt_editing,"stationary long-press edit begins");event(o,8,20,250);require(vt_editing,"initial long-press release does not finish");}
static void drag(unsigned id,unsigned slot){LvObj *o=vt_buttons[id];int *b=(int *)((char *)o+20),*c=(int *)((char *)vt_clip+20);event(o,1,b[0]+32,b[1]+32);event(o,2,c[0]+vt_slot_x(slot)+32,c[1]+vt_slot_y(slot)+32);event(o,8,point[0],point[1]);}
static void controls(LvObj *clip,LvObj **buttons,LvObj **labels){for(unsigned i=0;i<6;i++){buttons[i]=lv_create(clip);labels[i]=lv_create(clip);model_size(buttons[i],64,64);model_size(labels[i],120,30);model_align(buttons[i],1,vt_slot_x(i),vt_slot_y(i));model_align(labels[i],1,vt_slot_x(i)-28,vt_slot_y(i)+66);}}
int main(void){
 require(sizeof(void *)==4,"MIPS32 pointers");require(mmap((void *)0x8e1000,4096,3,0x812,-1,0)==(void *)0x8e1000,"mode map");
 active=lv_create(0);LvObj *shade=lv_create(0),*clip=lv_create(shade),*buttons[6],*labels[6];model_size(shade,360,360);model_size(clip,360,194);model_align(clip,1,0,68);lv_add_cb(shade,(void (*)(void *))0x47f74c,1,(void *)42);controls(clip,buttons,labels);setmode(3);vt_prepare(3,shade,clip,buttons,labels);
 require(vt_orders[0][0]==0&&vt_orders[0][5]==5,"BT defaults");int descriptors=shade->events;vt_prepare(3,shade,clip,buttons,labels);require(shade->events==descriptors,"bindings not duplicated");
 event(clip,5,20,250);require(!vt_editing,"orphan long-press cannot begin");
 event(clip,1,148,250);event(clip,2,148,180);model_align(shade,1,0,-107);event(clip,5,148,143);event(clip,8,148,19);require(!vt_editing&&shade->event_data[0]==0x47f74c,"slow upward native shade drag never gates original callback");
 event(clip,1,148,250);model_align(shade,1,0,0);event(clip,5,148,250);event(clip,8,148,250);require(!vt_editing,"press started half-open cannot enter after shade opens");
 event(clip,1,148,250);event(clip,2,148,180);event(clip,2,148,250);event(clip,5,148,250);event(clip,8,148,250);require(!vt_editing,"move-away then return is not stationary long-press");
 event(clip,1,148,250);event(clip,3,148,250);event(clip,5,148,250);require(!vt_editing,"press-lost cancels pending hold");
 begin(clip);model_align(shade,1,0,-107);vt_guard(3);require(!vt_editing&&folded&&shade->event_data[0]==0x47f74c&&shade->event_data[1]==42,"unexpected shade movement aborts edit and restores native callback");model_align(shade,1,0,0);
 begin(clip);require(!folded&&shade->event_data[0]==(unsigned)vt_native_gate&&shade->event_data[1]==42,"edit expands and gates stock callback without userdata change");drag(5,0);require(vt_orders[0][0]==5&&vt_orders[0][1]==0,"drag stable ID reorders");int *p=(int *)((char *)labels[5]+20);int *iconbox=(int *)((char *)buttons[5]+20);require(p[0]==iconbox[0]-28&&p[1]==iconbox[1]+66,"caption follows icon");vt_finish(1);require(!vt_editing&&folded&&shade->event_data[0]==0x47f74c&&shade->event_data[1]==42,"done restores folding and actual native callback");char text[64];require(textread(vt_order_path(),text,sizeof text)&&!strcmp(text,"5 0 1 2 3 4\n"),"BT order saved");
 LvObj *dacclip=lv_create(shade),*db[6],*dl[6];model_size(dacclip,360,194);model_align(dacclip,1,0,68);controls(dacclip,db,dl);setmode(1);vt_guard(1);vt_prepare(1,shade,dacclip,db,dl);require(vt_orders[1][0]==0&&vt_orders[0][0]==5,"DAC and BT separate");
 begin(dacclip);drag(1,5);fail_write=1;vt_finish(1);require(vt_orders[1][1]==1&&dac_folded&&vt_hint&&!(vt_hint->flags&1),"save failure reverts and reports");fail_write=0;
 begin(dacclip);drag(4,0);setmode(0);vt_guard(0);require(!vt_editing&&vt_orders[1][0]==0&&dac_folded&&shade->event_data[0]==0x47f74c,"mode exit cancels and ungates");
 setmode(1);vt_prepare(1,shade,dacclip,db,dl);begin(dacclip);drag(5,0);active=lv_create(0);vt_guard(1);require(!vt_editing&&vt_orders[1][0]==0,"external page cancels");
 vt_prepare(1,shade,dacclip,db,dl);begin(dacclip);delete_one(db[2]);require(!vt_editing&&vt_orders[1][0]==0&&shade->event_data[0]==0x47f74c,"object deletion aborts without dangling moves");
 puts("PASS actual visual editor MIPS32: slow up-swipe/moving/partial/lost/orphan press rejected, moved shade restores native callbacks, stationary long-press retained, per-mode save/drag/caption/cleanup unchanged");fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");



