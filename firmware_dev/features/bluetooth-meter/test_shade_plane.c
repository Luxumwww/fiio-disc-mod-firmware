#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "display_transform.h"
#include "shade_plane.h"
static unsigned frozen[129600],plane[129600];
int main(void){
 const unsigned heights[]={0,1,47,179,180,359,360,311,120,0};
 for(unsigned rotation=0;rotation<4;rotation++){
  for(unsigned i=0;i<129600;i++)frozen[i]=0xff000000u|i;
  memcpy(plane,frozen,sizeof plane);unsigned old=0;
  for(unsigned j=0;j<sizeof heights/sizeof heights[0];j++){
   unsigned h=heights[j];shade_mask_strip(plane,frozen,rotation,old,h);old=h;
   for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){unsigned i=disc_physical_index(x,y,rotation);assert(plane[i]==(y<h?0:frozen[i]));}
  }
  assert(!memcmp(plane,frozen,sizeof plane));
 }
 puts("PASS: production DPU shade strip mask matches native logical rectangle in all four rotations, both drag directions/endpoints, exact frozen pixels restored");
}
