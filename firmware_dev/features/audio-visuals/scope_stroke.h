/* Shared high-rate Wu-style line. Define DISC_SCOPE_STAMP/POINT locally.
 * Paired native endpoints remain exact; interior coverage uses two adjacent
 * minor-axis pixels instead of four bilinear stamps at every major pixel. */
static void disc_scope_stroke(unsigned char *context,int x0,int y0,int x1,int y1){
 (void)context;DISC_SCOPE_POINT(context,x0,y0);DISC_SCOPE_POINT(context,x1,y1);
 int dx=x1-x0,dy=y1-y0,ax=dx<0?-dx:dx,ay=dy<0?-dy:dy,steep=ay>ax,tmp;
 if(steep){tmp=x0;x0=y0;y0=tmp;tmp=x1;x1=y1;y1=tmp;}
 if(x0>x1){tmp=x0;x0=x1;x1=tmp;tmp=y0;y0=y1;y1=tmp;}
 dx=x1-x0;dy=y1-y0;if(!dx)return;
 int first=(x0+255)>>8,last=x1>>8,minor=(y0+(first*256-x0)*dy/dx)*256;
 /* Q16 slope without a64-bit divide: the major axis is at least the minor,
  * so both remainder products fit32 bits for this360px viewport. */
 int scaled=dy*256,increment=(scaled/dx)*256+((scaled%dx)*256)/dx;
 for(int major=first;major<=last;major++,minor+=increment){
  int at=minor>>16;unsigned fraction=((unsigned)minor>>8)&255;
  if(steep){DISC_SCOPE_STAMP(context,at,major,256-fraction);if(fraction)DISC_SCOPE_STAMP(context,at+1,major,fraction);}
  else{DISC_SCOPE_STAMP(context,major,at,256-fraction);if(fraction)DISC_SCOPE_STAMP(context,major,at+1,fraction);}
 }
}
#undef DISC_SCOPE_STAMP
#undef DISC_SCOPE_POINT
