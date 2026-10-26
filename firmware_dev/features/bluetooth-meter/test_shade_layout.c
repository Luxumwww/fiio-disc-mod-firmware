/* Exercise production geometry with the native deferred set_y/layout contract,
 * then composite its mask through the actual strip painter. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
typedef struct {unsigned words[10];} LvObj;
static LvObj object;
static int pending_y,layout_calls,valid=1;
static void update_layout(LvObj *o){assert(o==&object);layout_calls++;memcpy((char *)o+24,&pending_y,4);}
static int lv_valid(LvObj *o){return o==&object&&valid;}
#define FN(a,t) ((t)(update_layout))
#include "shade_geometry.h"
#include "display_transform.h"
#include "shade_plane.h"
static unsigned frozen[129600],plane[129600];
int main(void){
 const int path[]={-360,-359,-300,-167,0,0,-29,-58,-87,-108,-137,-166,-188,-216,-245,-281,-310,-339,-360,-360,-340,0,-360};
 for(unsigned i=0;i<129600;i++)frozen[i]=0xff000000u|i;
 for(unsigned rotation=0;rotation<4;rotation++){
  unsigned old=0;memcpy(plane,frozen,sizeof plane);pending_y=-360;update_layout(&object);
  for(unsigned n=0;n<sizeof path/sizeof path[0];n++){
   /* An animation updates only pending style; coords still contain last y. */
   pending_y=path[n];unsigned height=shade_visible_height(&object);
   assert(height==(unsigned)(pending_y+360));
   shade_mask_strip(plane,frozen,rotation,old,height);old=height;
   for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){
    unsigned p=disc_physical_index(x,y,rotation);assert(plane[p]==(y<height?0:frozen[p]));
   }
  }
 }
 int calls=layout_calls;object.words[9]=1;pending_y=0;assert(!shade_visible_height(&object)&&layout_calls==calls);
 object.words[9]=0;valid=0;assert(!shade_visible_height(&object)&&!shade_visible_height(0)&&layout_calls==calls);
 valid=1;pending_y=-500;assert(!shade_visible_height(&object));pending_y=75;assert(shade_visible_height(&object)==360);
 puts("PASS: production native layout-before-mask; delayed style, expanded close/reopen and both endpoints, all rotations, exact composite pixels, hidden/invalid/bounds guards");
}
