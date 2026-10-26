/* Binding an unchanged native image invalidates its entire object. Separate
 * source identity from mutable pixels so frozen backdrops cause no damage. */
typedef struct {const void *object,*source;unsigned revision,position;} DiscImageStamp;
enum {DISC_IMAGE_SOURCE=1,DISC_IMAGE_PIXELS=2};
static int image_stamp_update(DiscImageStamp *s,const void *object,const void *source,unsigned revision,unsigned position){
 if(!object||!source)return 0;
 int result=s->object!=object||s->source!=source?DISC_IMAGE_SOURCE:s->revision!=revision||s->position!=position?DISC_IMAGE_PIXELS:0;
 *s=(DiscImageStamp){object,source,revision,position};return result;
}
