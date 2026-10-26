#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "slide_compose.h"
int main(void){
 unsigned *old=malloc(518400),*next=malloc(518400),*out=malloc(518400);assert(old&&next&&out);
 for(unsigned i=0;i<129600;i++){old[i]=0xff000000u+i;next[i]=0xff800000u+i;}
 for(unsigned d=0;d<4;d++)for(unsigned p=0;p<=360;p++){
  disc_slide_compose(out,old,next,p,d);
  for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){
   unsigned v;
   if(d==DISC_SLIDE_RIGHT)v=x<360-p?old[y*360+x+p]:next[y*360+x+p-360];
   else if(d==DISC_SLIDE_LEFT)v=x<p?next[y*360+x+360-p]:old[y*360+x-p];
   else if(d==DISC_SLIDE_UP)v=y<360-p?old[(y+p)*360+x]:next[(y+p-360)*360+x];
   else v=y<p?next[(y+360-p)*360+x]:old[(y-p)*360+x];
   assert(out[y*360+x]==v);
  }
 }
 free(old);free(next);free(out);puts("PASS: four directions, all361 offsets, exact reference pixels, opaque endpoints");return 0;
}
