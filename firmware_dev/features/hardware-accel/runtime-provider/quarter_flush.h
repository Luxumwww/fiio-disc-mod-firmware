/* The native LVGL rotation has already produced physical row-major pixels.
 * Bounds and clipping retain the original source stride. No second rotation. */
static void quarter_copy(unsigned *dst,const int *r,const unsigned *src){
 int width=r[2]-r[0]+1;
 int x0=r[0]<0?0:r[0],y0=r[1]<0?0:r[1];
 int x1=r[2]>359?359:r[2],y1=r[3]>359?359:r[3];
 if(x0>x1||y0>y1)return;
 for(int y=y0;y<=y1;y++)memcpy(dst+y*360+x0,src+(y-r[1])*width+x0-r[0],(unsigned)(x1-x0+1)*4);
}
