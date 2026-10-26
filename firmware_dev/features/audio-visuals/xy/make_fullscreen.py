from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
p=Path('firmware_dev/features/audio-visuals/xy');lines=['typedef struct {unsigned x,y,w,h,rgb;const unsigned char *alpha;} Label;'];descs=[]
for i,(text,x,y) in enumerate([('X',338,173),('Y',175,11)]):
 font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',14);box=font.getbbox(text);w=box[2]-box[0]+2;h=box[3]-box[1]+2;im=Image.new('L',(w,h));ImageDraw.Draw(im).text((1-box[0],1-box[1]),text,font=font,fill=255);lines.append('static const unsigned char label%d[]={%s};'%(i,','.join(map(str,im.tobytes()))));descs.append('{%d,%d,%d,%d,0x91afb3,label%d}'%(x,y,w,h,i))
lines.append('static const Label labels[]={'+','.join(descs)+'};');(p/'labels.h').write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
f=p/'xy.c';s=f.read_text(encoding='utf-8').replace('#define SIDE 272','#define SIDE 360').replace('#define LEFT 44','#define LEFT 0').replace('#define TOP 44','#define TOP 0').replace('x==136||y==136','x==180||y==180').replace('(x==0||x==271||y==0||y==271||x%34==0||y%34==0)','((x>0&&x<359&&x%45==0)||(y>0&&y<359&&y%45==0))').replace('136*256+x*126/128','180*256+x*168/128').replace('136*256-y*126/128','180*256-y*168/128')
start=s.index(' for(unsigned i=0;i<sizeof(labels)');end=s.index('\n memcpy(canvas,base',start);paint=s[start:end]
s=s[:start]+' paint_labels(base);'+s[end:]
insert=s.index('static void init(void)');helper=paint.replace('base[ix]','target[ix]');s=s[:insert]+'static void paint_labels(unsigned *target){'+helper+'\n}\n'+s[insert:]
s=s.replace(' }generation++;',' }paint_labels(canvas);generation++;');f.write_text(s,encoding='utf-8',newline='\n')
for name in ('build.py','package_module.py'):
 f=p/name;f.write_text(f.read_text(encoding='utf-8').replace('1.0.1','1.0.2'),encoding='utf-8',newline='\n')
f=p/'module-source/README.txt';s=f.read_text(encoding='utf-8').replace('1.3.15','1.3.16')+'Fullscreen round-screen plot: no title/footer or rectangular frame, only X on right axis and Y on top axis.\n';f.write_text(s,encoding='utf-8',newline='\n')
f=Path('firmware_dev/features/bluetooth-meter/package_module.py');s=f.read_text(encoding='utf-8').replace('disc-audio-xy@1.0.1','disc-audio-xy@1.0.2');f.write_text(s,encoding='utf-8',newline='\n')
print('fullscreen plot / axis labels only prepared')
