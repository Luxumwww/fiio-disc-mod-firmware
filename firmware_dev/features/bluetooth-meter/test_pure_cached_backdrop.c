#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct Obj {int valid,hidden,image;struct Obj *parent;void (*deleted)(void *);} LvObj;
static LvObj objects[64],screens[2]={{.valid=1},{.valid=1}};
static unsigned created,deleted,source_calls;
static int fail_image;
static unsigned dpu_main_generation,dpu_main_rotation,dpu_frozen_xy[129600];
#include "display_transform.h"
static LvObj *lv_create(LvObj *parent){LvObj *o=&objects[created++];assert(created<64);*o=(LvObj){.valid=1,.hidden=0,.parent=parent};return o;}
static LvObj *img(LvObj *parent){if(fail_image){fail_image=0;return 0;}LvObj *o=lv_create(parent);o->image=1;return o;}
static int lv_valid(LvObj *o){return o&&o->valid;}
static LvObj *lv_event_target(void *e){return e;}
static void stylezero(LvObj *o){assert(lv_valid(o));}
static void lv_size(LvObj *o,int w,int h){assert(lv_valid(o)&&w==360&&h==360);}
static void lv_align(LvObj *o,int align,int x,int y){assert(lv_valid(o)&&align==1&&!x&&!y);}
static void lv_bg(LvObj *o,unsigned color,unsigned selector){assert(lv_valid(o)&&color==0xff000000&&!selector);}
static void opa(LvObj *o,unsigned value,unsigned selector){assert(lv_valid(o)&&value==255&&!selector);}
static void lv_add_cb(LvObj *o,void (*cb)(void *),unsigned event,void *user){assert(event==33&&!user);o->deleted=cb;}
static void lv_clear_flag(LvObj *o,unsigned flags){assert(lv_valid(o)&&flags==(2|16));}
static void show(LvObj *o,int yes){assert(lv_valid(o));o->hidden=!yes;}
static const void *source_desc;
static void src(LvObj *o,const void *source){assert(lv_valid(o)&&o->image);source_desc=source;source_calls++;}
static void delete_obj(LvObj **ptr){
 LvObj *o=*ptr;assert(lv_valid(o));if(o->deleted)o->deleted(o);
 for(unsigned i=0;i<created;i++)if(objects[i].parent==o)objects[i].valid=0;
 o->valid=0;*ptr=0;deleted++;
}
#define FN(a,t) ((t)delete_obj)
#include "pure_backdrop.h"
static unsigned logical[129600],repainted[129600];
int main(void){
 for(unsigned i=0;i<129600;i++)logical[i]=0xff123400u+i;
 for(unsigned rotation=0;rotation<4;rotation++){
  dpu_main_rotation=rotation;dpu_main_generation++;
  disc_rotate_frame(dpu_frozen_xy,logical,rotation);
  pure_backdrop_prepare(&screens[0],1);
  assert(pure_backdrop&&!pure_backdrop->hidden&&pure_backdrop_parent==screens);
  assert(pure_backdrop_image&&pure_backdrop_image->parent==pure_backdrop&&source_desc==&pure_backdrop_desc);
  assert(pure_backdrop_desc.header==(5u|(360u<<10)|(360u<<21))&&pure_backdrop_desc.size==518400);
  assert(!memcmp(pure_backdrop_pixels,logical,sizeof logical));
  /* Same cache goes through the native transform, exactly reproducing the
   * captured physical background, without a second moving overlay mask. */
  disc_rotate_frame(repainted,pure_backdrop_pixels,rotation);
  assert(!memcmp(repainted,dpu_frozen_xy,sizeof repainted));
  unsigned calls=source_calls;
  for(unsigned frame=0;frame<100;frame++)pure_backdrop_prepare(&screens[0],1);
  assert(source_calls==calls); /* no per-animation-frame background conversion */
  pure_backdrop_prepare(&screens[0],0);assert(pure_backdrop->hidden);
 }
 unsigned d=deleted;
 pure_backdrop_prepare(&screens[1],0);assert(!pure_backdrop&&!pure_backdrop_image&&deleted==d+1);
 fail_image=1;pure_backdrop_prepare(&screens[1],1);assert(pure_backdrop->hidden&&!pure_backdrop_image);
 pure_backdrop_prepare(&screens[1],1);assert(pure_backdrop_image&&!pure_backdrop->hidden);
 LvObj *old=pure_backdrop;delete_obj(&old);assert(!pure_backdrop&&!pure_backdrop_parent&&!pure_backdrop_image&&!pure_backdrop_generation);
 pure_backdrop_prepare(&screens[0],1);assert(!pure_backdrop->hidden&&pure_backdrop_parent==screens);
 puts("PASS: production pure cached background; exact pixels in four native rotations, 400 frame cache reuse, hide/reopen, page ownership/deletion, allocation retry; controls remain on native top layer");
}
