from pathlib import Path
import math
p=Path(__file__).resolve().parent
directions=[]
for i in range(320):
    angle=-math.pi/2+2*math.pi*i/320
    directions.append((round(32767*math.cos(angle)),round(32767*math.sin(angle))))
grid=[]
for y in range(360):
    for x in range(360):
        dx=x-180;dy=y-180;r=math.hypot(dx,dy)
        value=0
        if r<=172:
            if min(abs(r-circle) for circle in (30,60,90,120,150,171))<0.55:value=1
            # Twelve quiet spokes, four main axes a little brighter.
            angle=math.atan2(dy,dx)
            deviation=abs(math.sin(6*angle))*r/6
            if deviation<0.45 and r>5:value=1
            if abs(dx)<1 or abs(dy)<1:value=2
        grid.append(value)
(p/'geometry.h').write_text('static const short polar_direction[320][2]={'+','.join('{%d,%d}'%v for v in directions)+'};\nstatic const unsigned char polar_grid[129600]={'+','.join(map(str,grid))+'};\n',encoding='ascii',newline='\n')
