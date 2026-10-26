"""Generate small native LVGL RGBA tiles for an antialiased rim meter."""
from pathlib import Path
import math

HERE = Path(__file__).parent
COUNT = 64
result = ['/* Continuous rim; four antialiased endpoint positions within each tile. */', '#define SEGMENTS 64', '#define SUBSTEPS 4', 'typedef struct {unsigned header,size;const void *data;} ImageDesc;', 'typedef struct {int x,y,w,h;ImageDesc image[5];} Tile;']
geometries = []
for side in range(2):
    for index in range(COUNT):
        first, last = index * 117 / COUNT, (index + 1) * 117 / COUNT
        pixels = {}
        for y in range(360):
            for x in range(360):
                dx, dy = x + .5 - 180, y + .5 - 180
                radius = math.hypot(dx, dy)
                if radius < 175.5 or radius > 182.5:
                    continue
                angle = math.degrees(math.atan2(dy, dx))
                if side == 0:
                    travel = (angle - 90) % 360
                else:
                    travel = (90 - angle) % 360
                if not first <= travel < last:
                    continue
                alpha = max(0, min(1, 3.5 - abs(radius - 179)))
                pixels[(x,y)] = (round(alpha * 255), travel)
        xs, ys = [p[0] for p in pixels], [p[1] for p in pixels]
        x0,y0,x1,y1 = min(xs),min(ys),max(xs),max(ys)
        w,h = x1-x0+1,y1-y0+1
        fraction = index / (COUNT - 1)
        # Cyan -> green -> warm yellow near the loud end.
        color = (round(45 + 210 * max(0, (fraction-.6)/.4)), round(200 + 40 * min(fraction/.6,1)), round(230*(1-fraction)))
        for active in range(5):
            name = f'pixels_{side}_{index}_{active}'
            data = []
            for y in range(y0,y1+1):
                for x in range(x0,x1+1):
                    a, travel = pixels.get((x,y),(0,0))
                    # Feather the moving endpoint by one pixel, with no gaps
                    # or segment outlines. All states share an identical box.
                    weight = 0 if active == 0 else 1 if active == 4 else max(0,min(1,(first+(last-first)*active/4-travel)*math.pi*179/180+.5))
                    r,g,b = (round(off+(on-off)*weight) for off,on in zip((23,44,50),color))
                    data.extend((b,g,r,a))
            result.append('static const unsigned char '+name+'[]={'+','.join(map(str,data))+'};')
        geometries.append((side,index,x0,y0,w,h))
result.append('static const Tile tiles[128]={')
for side,index,x,y,w,h in geometries:
    head = 5 | (w << 10) | (h << 21)
    images = ','.join('{%du,%d,pixels_%d_%d_%d}' % (head,w*h*4,side,index,state) for state in range(5))
    result.append('{%d,%d,%d,%d,{%s}},' % (x,y,w,h,images))
result.append('};')
result.append('static const unsigned long long energy_threshold[101]={'+','.join(str(round(960 * (32768 * 10 ** (((v-100)*.6)/20)) ** 2))+'ull' for v in range(101))+'};')
(HERE/'assets.h').write_text('\n'.join(result)+'\n',encoding='ascii')
print('tile byte payload:', sum(w*h*20 for _,_,_,_,w,h in geometries))

# Match the stock sysui 64px circular pictograms; no runtime rasterizer.
def line_distance(x,y,a,b):
    dx,dy=b[0]-a[0],b[1]-a[1]
    t=max(0,min(1,((x-a[0])*dx+(y-a[1])*dy)/(dx*dx+dy*dy)))
    return math.hypot(x-a[0]-t*dx,y-a[1]-t*dy)

paths={
 'dsp': [((20,23),(44,23)),((20,32),(44,32)),((20,41),(44,41)),((26,19),(26,27)),((38,28),(38,36)),((29,37),(29,45))],
 'info': [((20,21),(44,21)),((44,21),(44,43)),((44,43),(20,43)),((20,43),(20,21)),((32,30),(32,38)),((31.8,26),(32.2,26))],
 'normalize': [((19,32),(25,32)),((25,32),(29,22)),((29,22),(35,42)),((35,42),(39,32)),((39,32),(45,32))],
 'level': [((20,29),(20,39)),((28,21),(28,43)),((36,25),(36,43)),((44,30),(44,38))],
 'rate': [((18,25),(44,25)),((39,20),(44,25)),((39,30),(44,25)),
          ((20,39),(46,39)),((20,39),(25,34)),((20,39),(25,44))],
 'usb': [((32,46),(32,18)),((32,18),(28,24)),((32,18),(36,24)),
         ((32,37),(23,32)),((23,32),(23,26)),((32,33),(42,28)),((42,28),(42,23))],
 'fold': [((20,27),(32,39)),((32,39),(44,27))],
}
icons=['/* Stock-style, supersampled pictograms. */']
for name,strokes in paths.items():
 for on in range(2):
  pixels=[]
  for y in range(64):
   for x in range(64):
    acc=[0,0,0,0]
    for sy in range(4):
     for sx in range(4):
      px,py=x+(sx+.5)/4,y+(sy+.5)/4
      if math.hypot(px-32,py-32)>32:continue
      ink=any(line_distance(px,64-py if name=='fold' and on else py,a,b)<=1.6 for a,b in strokes)
      if name=='fold' and not ink:continue
      if name=='usb':ink=ink or math.hypot(px-32,py-46)<3 or math.hypot(px-23,py-25)<2.5 or (abs(px-42)<2.3 and abs(py-22)<2.3)
      rgb=(205,205,205) if name=='fold' else (((244,0,65) if on else (205,205,205)) if ink else ((103,19,41) if on else (42,42,42)))
      for k,v in enumerate((*reversed(rgb),255)):acc[k]+=v
    pixels.extend(round(v/16) for v in acc)
  key=f'{name}_icon_{on}'
  icons.append('static const unsigned char '+key+'_data[]={'+','.join(map(str,pixels))+'};')
  icons.append('static const ImageDesc '+key+'={%du,16384,%s_data};'%(5|(64<<10)|(64<<21),key))
(HERE/'control_assets.h').write_text('\n'.join(icons)+'\n',encoding='ascii')
