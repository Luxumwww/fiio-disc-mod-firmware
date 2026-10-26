/* V2.57 lv_refr flush rotates enum 1 counterclockwise, enum 3 clockwise. */
static unsigned disc_physical_index(unsigned x,unsigned y,unsigned rotation){
 switch(rotation&3u){case 1:return (359u-x)*360u+y;case 2:return (359u-y)*360u+359u-x;case 3:return x*360u+359u-y;default:return y*360u+x;}
}
/* Full opaque frames: choose the transform once, not once per pixel.
 * Blocked quarter turns keep destination rows in a small working set. */
static void disc_rotate_frame(unsigned *dest,const unsigned *source,unsigned rotation){
 switch(rotation&3u){
 case 0:memcpy(dest,source,518400);return;
 case 2:for(unsigned i=0;i<129600;i++)dest[129599-i]=source[i];return;
 default:
  for(unsigned by=0;by<360;by+=16)for(unsigned bx=0;bx<360;bx+=16){
   unsigned ey=by+16<360?by+16:360,ex=bx+16<360?bx+16:360;
   if((rotation&3u)==1){for(unsigned x=bx;x<ex;x++)for(unsigned y=by;y<ey;y++)dest[(359-x)*360+y]=source[y*360+x];}
   else {for(unsigned x=bx;x<ex;x++)for(unsigned y=by;y<ey;y++)dest[x*360+359-y]=source[y*360+x];}
  }
 }
}
