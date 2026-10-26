"""Keep the existing 14px Segoe UI glyph style for the new audio axes."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
p=Path(__file__).resolve().parent
font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',14)
lines=['typedef struct {unsigned x,y,w,h,rgb;const unsigned char *alpha;} Label;']
descs=[]
for i,(text,cx,y) in enumerate((('S',351,173),('M',180,3),('L',57,51),('R',303,51))):
    box=font.getbbox(text);w=box[2]-box[0]+2;h=box[3]-box[1]+2
    im=Image.new('L',(w,h));ImageDraw.Draw(im).text((1-box[0],1-box[1]),text,font=font,fill=255)
    # Keep every visible glyph pixel inside the circular 360px viewport.
    assert all((cx-w//2+x-179.5)**2+(y+yy-179.5)**2<179.5**2
               for yy in range(h) for x in range(w) if im.getpixel((x,yy)))
    lines.append('static const unsigned char label%d[]={%s};'%(i,','.join(map(str,im.tobytes()))))
    descs.append('{%d,%d,%d,%d,0x91afb3,label%d}'%(cx-w//2,y,w,h,i))
lines.append('static const Label labels[]={'+','.join(descs)+'};')
(p/'labels.h').write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
