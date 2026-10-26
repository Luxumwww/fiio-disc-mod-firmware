/* Actual production header on the verified 32-bit object/timer ABI. */
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef struct Obj LvObj;
struct Obj{unsigned klass;LvObj *parent;void *spec;unsigned padding[6],flags,unused;const char *text;unsigned extra[9];void *timer;int valid,n;LvObj *children[2];unsigned bg,color,opacity,align,font,callbacks;void (*deleted)(void *);};
typedef struct {LvObj *target;} Event;
enum{DISC_LV_HIDDEN=1,DISC_LV_DELETE=33};
static LvObj *popup;
static unsigned pauses,styles;static unsigned gif_timer[6],wait_timer[6];
static int lv_valid(LvObj *o){return o&&o->valid;}
static unsigned child_count(LvObj *o){return (unsigned)o->n;}
static LvObj *child(LvObj *o,int i){return i>=0&&i<o->n?o->children[i]:0;}
static LvObj *lv_event_target(void *e){return ((Event *)e)->target;}
static void pause_timer(void *p){assert(p==gif_timer);((unsigned char *)p)[20]|=1;pauses++;}
#define FN(a,t) ((t)pause_timer)
#define LV_GLOBAL(a) popup
static void lv_add_flag(LvObj *o,unsigned flags){assert(lv_valid(o));o->flags|=flags;}
static void lv_bg(LvObj *o,unsigned value,unsigned selector){assert(!selector&&lv_valid(o));o->bg=value;styles++;}
static void bg_opa(LvObj *o,unsigned value,unsigned selector){assert(!selector&&lv_valid(o));o->opacity=value;styles++;}
static void lv_color(LvObj *o,unsigned value,unsigned selector){assert(!selector&&lv_valid(o));o->color=value;styles++;}
static void lv_align(LvObj *o,int align,int x,int y){assert(!x&&!y&&lv_valid(o));o->align=align;styles++;}
static void lv_add_cb(LvObj *o,void (*fn)(void *),unsigned code,void *data){assert(code==33&&!data);o->callbacks++;o->deleted=fn;}
#include "transition_ui.h"
static void setup(LvObj *root,LvObj *gif,LvObj *label){memset(root,0,sizeof *root);memset(gif,0,sizeof *gif);memset(label,0,sizeof *label);root->valid=gif->valid=label->valid=1;root->n=2;root->children[0]=gif;root->children[1]=label;gif->klass=0x7c2050;label->klass=0x7ffd00;gif->timer=gif_timer;label->text="正在切换模式";label->font=0x83ac5c;root->bg=0xffffffff;root->opacity=255;popup=root;}
int main(void){
 assert(sizeof(void *)==4&&__builtin_offsetof(LvObj,flags)==36&&__builtin_offsetof(LvObj,text)==44&&__builtin_offsetof(LvObj,timer)==84);
 LvObj root,gif,label,unrelated;setup(&root,&gif,&label);wait_timer[0]=3000;wait_timer[2]=0x46b570;unrelated=root;
 mode_transition_prepare();assert(root.bg==0xff000000&&root.opacity==255&&(gif.flags&1)&&label.color==0xffa8a8a8&&label.align==9);
 assert(label.font==0x83ac5c&&!strcmp(label.text,"正在切换模式")&&wait_timer[0]==3000&&wait_timer[2]==0x46b570&&gif.timer==gif_timer&&pauses==1&&root.callbacks==1&&unrelated.bg==0xffffffff);
 unsigned count=styles;for(unsigned i=0;i<100;i++)mode_transition_prepare();assert(styles==count&&pauses==1&&root.callbacks==1);
 Event e={&root};root.deleted(&e);assert(!mt_wait);setup(&root,&gif,&label);root.children[0]=&label;root.children[1]=&gif;mode_transition_prepare();assert(root.bg==0xff000000&&pauses==2); /* Identify by class, not child index. */
 root.deleted(&e);setup(&root,&gif,&label);gif.klass=42;mode_transition_prepare();assert(root.bg==0xffffffff&&pauses==2&&!mt_wait);gif.klass=0x7c2050;root.n=1;mode_transition_prepare();assert(!mt_wait);root.n=2;mode_transition_prepare();assert(pauses==3&&root.bg==0xff000000);
 root.deleted(&e);popup=0;mode_transition_prepare();assert(!mt_wait);puts("PASS actual MIPS32 pre-draw wait popup: black first render, static gray localized/native-font label, GIF pause not delete, native wait timer unchanged, no repeat work, class-based reordered children, deletion/reuse, unrelated/incomplete rejected");fflush(stdout);return 0;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
