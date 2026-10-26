"""Generate a circular-safe polar grid and the existing 14px Segoe UI style."""
from pathlib import Path
import math
from PIL import Image,ImageDraw,ImageFont
p=Path(__file__).resolve().parent
font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',14)
grid=[]
for y in range(360):
 for x in range(360):
  dx=x-180;dy=180-y;r=math.hypot(dx,dy);v=0
  if dy>=0 and r<=177:
   if min(abs(r-k) for k in (58,117,175))<.65:v=1
   if abs(dx)<.65 or abs(abs(dx)-dy)<.75:v=2
   if dy==0:v=2
  if v:assert (x-179.5)**2+(y-179.5)**2<179.5**2
  grid.append(v)
lines=['static const unsigned char stereo_grid[129600]={'+','.join(map(str,grid))+'};',
       'typedef struct {unsigned x,y,w,h;const unsigned char *alpha;} StereoLabel;']
labels=[]
for i,(text,cx,y) in enumerate((('CORRELATION',180,213),('M',180,5),('L',61,54),('R',299,54),('-1',99,297),('0',180,297),('+1',261,297),('L',62,274),('R',298,274))):
 box=font.getbbox(text);w=box[2]-box[0]+2;h=box[3]-box[1]+2
 im=Image.new('L',(w,h));ImageDraw.Draw(im).text((1-box[0],1-box[1]),text,font=font,fill=255)
 lx=cx-w//2
 assert all((lx+x-179.5)**2+(y+yy-179.5)**2<179.5**2 for yy in range(h) for x in range(w) if im.getpixel((x,yy)))
 lines.append('static const unsigned char stereo_label%d[]={%s};'%(i,','.join(map(str,im.tobytes()))))
 labels.append('{%d,%d,%d,%d,stereo_label%d}'%(lx,y,w,h,i))
lines.append('static const StereoLabel stereo_labels[]={'+','.join(labels)+'};')
number_font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',26)
chars='-+0123456789.';top=number_font.getbbox(chars)[1];bottom=number_font.getbbox(chars)[3]
lines.append('typedef struct {char ch;unsigned w,h;const unsigned char *alpha;} StereoGlyph;')
glyphs=[]
for i,ch in enumerate(chars):
 w=math.ceil(number_font.getlength(ch))+2;h=bottom-top+2
 im=Image.new('L',(w,h));ImageDraw.Draw(im).text((1,1-top),ch,font=number_font,fill=255)
 lines.append('static const unsigned char stereo_number%d[]={%s};'%(i,','.join(map(str,im.tobytes()))))
 glyphs.append("{'%s',%d,%d,stereo_number%d}"%(ch,w,h,i))
lines.append('static const StereoGlyph stereo_numbers[]={'+','.join(glyphs)+'};')
# Sparse, antialiased arc segments. Trigonometry runs only in this generator.
pixels=[];segments=[]
for channel in range(2):
 sign=1 if channel==0 else -1
 angles=[math.pi/2+sign*(.09+k*(math.pi/2-.18)/96) for k in range(97)]
 poly=[(179.5+170*math.cos(a),179.5+170*math.sin(a)) for a in angles]
 poly += [(179.5+164*math.cos(a),179.5+164*math.sin(a)) for a in reversed(angles)]
 im=Image.new('L',(1440,1440));ImageDraw.Draw(im).polygon([(x*4,y*4) for x,y in poly],fill=255)
 im=im.resize((360,360),Image.Resampling.LANCZOS)
 groups=[[] for _ in range(256)]
 for yy in range(181,355):
  for xx in range(360):
   alpha=im.getpixel((xx,yy))
   if alpha<3:continue
   assert (xx-179.5)**2+(yy-179.5)**2<179.5**2
   theta=math.atan2(yy-179.5,xx-179.5)
   u=((theta-math.pi/2)*sign-.09)/(math.pi/2-.18)
   slot=max(0,min(255,int(u*256)))
   groups[slot].append(((yy*360+xx)<<8)|alpha)
 for group in groups:
  start=len(pixels);pixels.extend(group);segments.append('{%d,%d}'%(start,len(group)))
lines.append('#define STEREO_METER_BINS 256u')
lines.append('typedef struct {unsigned start,count;} StereoSegment;')
lines.append('static const unsigned stereo_meter_pixels[]={'+','.join(map(str,pixels))+'};')
lines.append('static const StereoSegment stereo_meter_segments[]={'+','.join(segments)+'};')
lines.append('static const float stereo_meter_energy[256]={'+','.join('%.8ef'%(32768**2*10**((-54+54*i/255)/10)) for i in range(256))+'};')
(p/'stereo_assets.h').write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
