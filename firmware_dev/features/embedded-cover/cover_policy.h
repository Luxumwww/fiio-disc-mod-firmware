#ifndef DISC_COVER_POLICY_H
#define DISC_COVER_POLICY_H
#define DISC_COVER_STOCK_LIMIT 1500000u
#define DISC_COVER_MAX_BYTES 8388608u
#define DISC_COVER_MAX_PIXELS 4194304u
#define DISC_COVER_JPEG_MAX_PIXELS 64000000u
#define DISC_COVER_JPEG_HEAP_LIMIT 33554432u
static unsigned cover_be32(const unsigned char *p){return (unsigned)p[0]<<24|(unsigned)p[1]<<16|(unsigned)p[2]<<8|p[3];}
static int cover_dimensions_ok(unsigned w,unsigned h){return w&&h&&w<=4096&&h<=4096&&w<=DISC_COVER_MAX_PIXELS/h;}
static int cover_jpeg_dimensions_ok(unsigned w,unsigned h){return w&&h&&w<=8000&&h<=8000&&w<=DISC_COVER_JPEG_MAX_PIXELS/h;}
static unsigned cover_jpeg_scale(unsigned w,unsigned h){unsigned d=1;while(d<8&&((w+d-1)/d>728||(h+d-1)/d>728))d*=2;return d;}
/* Inspect actual encoded bytes, never MIME/tag width/height. Native decoders
 * still validate the complete PNG/JPEG; this is a pre-allocation size guard. */
static int cover_large_image_ok(const unsigned char *p,unsigned n,unsigned *width,unsigned *height){
 if(!p||n<24||n>DISC_COVER_MAX_BYTES)return 0;
 if(p[0]==137&&p[1]==80&&p[2]==78&&p[3]==71&&p[4]==13&&p[5]==10&&p[6]==26&&p[7]==10){
  if(cover_be32(p+8)!=13||p[12]!='I'||p[13]!='H'||p[14]!='D'||p[15]!='R')return 0;
  unsigned w=cover_be32(p+16),h=cover_be32(p+20);if(!cover_dimensions_ok(w,h))return 0;*width=w;*height=h;return 1;
 }
 if(p[0]!=255||p[1]!=216)return 0;
 unsigned at=2;
 while(at<n){
  if(p[at++]!=255)return 0;while(at<n&&p[at]==255)at++;if(at>=n)return 0;unsigned marker=p[at++];
  if(marker==217||marker==218)return 0;
  if(marker==1||(marker>=208&&marker<=215))continue;
  if(n-at<2)return 0;unsigned size=(unsigned)p[at]<<8|p[at+1];if(size<2||size>n-at)return 0;
  if(marker>=192&&marker<=207&&marker!=196&&marker!=200&&marker!=204){
   if(size<8)return 0;unsigned h=(unsigned)p[at+3]<<8|p[at+4],w=(unsigned)p[at+5]<<8|p[at+6];
   if(!cover_jpeg_dimensions_ok(w,h))return 0;*width=w;*height=h;return 1;
  }
  at+=size;
 }
 return 0;
}
static int cover_large_hint(unsigned bytes){return bytes>=DISC_COVER_STOCK_LIMIT&&bytes<=DISC_COVER_MAX_BYTES;}
#endif
