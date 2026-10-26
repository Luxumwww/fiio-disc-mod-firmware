#include <assert.h>
#include <stdio.h>
#include "image_stamp.h"
int main(void){
 DiscImageStamp cache={0};int object=0,image=0,next_image=0;
 assert(image_stamp_update(&cache,&object,&image,1,360)==DISC_IMAGE_SOURCE);
 /* A stopped backdrop survives thousands of refresh and poll callbacks. */
 for(unsigned i=0;i<6000;i++)assert(!image_stamp_update(&cache,&object,&image,1,360));
 /* The renderer updates pixels in place; pointer equality must not freeze it. */
 for(unsigned i=2;i<602;i++)assert(image_stamp_update(&cache,&object,&image,i,360)==DISC_IMAGE_PIXELS);
 assert(image_stamp_update(&cache,&object,&image,601,120)==DISC_IMAGE_PIXELS);
 assert(image_stamp_update(&cache,&object,&next_image,601,120)==DISC_IMAGE_SOURCE);
 cache=(DiscImageStamp){0};assert(image_stamp_update(&cache,&object,&next_image,601,120)==DISC_IMAGE_SOURCE);
 assert(!image_stamp_update(&cache,0,&image,1,0));
 puts("PASS: frozen backdrop 6000 callbacks emits no damage; mutable image revisions and swipe position still redraw; replacement/reset rebinds native source");
}
