/* Opaque, rotation-neutral frozen pages. Full rows use contiguous copies;
 * horizontal slides copy two runs per row. No FFT/trace/rotation per pixel. */
enum {DISC_SLIDE_RIGHT,DISC_SLIDE_LEFT,DISC_SLIDE_UP,DISC_SLIDE_DOWN};
static void disc_slide_compose(unsigned *out,const unsigned *old,const unsigned *next,unsigned p,unsigned direction){
 if(p>360)p=360;
 if(direction==DISC_SLIDE_UP){memcpy(out,old+p*360,(360-p)*1440);memcpy(out+(360-p)*360,next,p*1440);return;}
 if(direction==DISC_SLIDE_DOWN){memcpy(out,next+(360-p)*360,p*1440);memcpy(out+p*360,old,(360-p)*1440);return;}
 for(unsigned y=0;y<360;y++){
  unsigned *d=out+y*360;const unsigned *a=old+y*360,*b=next+y*360;
  if(direction==DISC_SLIDE_RIGHT){memcpy(d,a+p,(360-p)*4);memcpy(d+360-p,b,p*4);}
  else {memcpy(d,b+360-p,p*4);memcpy(d+p,a,(360-p)*4);}
 }
}
