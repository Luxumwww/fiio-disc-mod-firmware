#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "display_transform.h"
static unsigned source[129600],dest[129600];
int main(void){
 for(unsigned i=0;i<129600;i++)source[i]=0xff000000u|i;
 for(unsigned r=0;r<4;r++){
  memset(dest,0,sizeof dest);disc_rotate_frame(dest,source,r);
  for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++)assert(dest[disc_physical_index(x,y,r)]==source[y*360+x]);
 }
 puts("PASS: full-frame optimized copy equals V2.57 mapping at every pixel for all four rotations, including partial edge tiles");
}
