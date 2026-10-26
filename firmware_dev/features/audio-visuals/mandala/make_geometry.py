from pathlib import Path
import math, colorsys
p=Path(__file__).resolve().parent
reverse=[int(f'{i:06b}'[::-1],2) for i in range(64)]
twiddle=[(math.cos(-math.tau*i/64),math.sin(-math.tau*i/64)) for i in range(32)]
mapping=[];radii=[];fractions=[]
for y in range(360):
 for x in range(360):
  dx=x-179.5;dy=y-179.5;r=math.hypot(dx,dy);radii.append(min(255,round(r)))
  if r>174 or r<6:mapping.append(65535);fractions.append(0);continue
  angle=math.atan2(dy,dx)%(math.tau/6)
  angle=min(angle,math.tau/6-angle)
  radius=2+(r-6)*28/168
  px=radius*math.cos(angle);py=radius*math.sin(angle);fx=math.floor(px);fy=math.floor(py)
  mapping.append(fy*64+fx)
  fractions.append(round((px-fx)*255)|round((py-fy)*255)<<8)
palette=[]
for i in range(256):
 rgb=colorsys.hsv_to_rgb(i/256,.78,1)
 palette.append(0xff000000|round(rgb[0]*255)<<16|round(rgb[1]*255)<<8|round(rgb[2]*255))
text='static const unsigned char reverse64[64]={'+','.join(map(str,reverse))+'};\n'
text+='static const float twiddle64[32][2]={'+','.join('{%.9ef,%.9ef}'%v for v in twiddle)+'};\n'
# The six-fold projection is exactly symmetric about both screen axes.
# Verify it before sharing interpolation and smoothing across quadrants.
for values in (mapping,fractions,radii):
 for y in range(180):
  for x in range(180):
   value=values[y*360+x]
   assert value==values[y*360+359-x]==values[(359-y)*360+x]==values[(359-y)*360+359-x]
quarter=lambda values:[values[y*360+x] for y in range(180) for x in range(180)]
text+='static const unsigned short mandala_map[32400]={'+','.join(map(str,quarter(mapping)))+'};\n'
text+='static const unsigned short mandala_fraction[32400]={'+','.join(map(str,quarter(fractions)))+'};\n'
text+='static const unsigned char mandala_radius[32400]={'+','.join(map(str,quarter(radii)))+'};\n'
text+='static const unsigned mandala_palette[256]={'+','.join(hex(v)+'u' for v in palette)+'};\n'
starts=[];ends=[]
for y in range(180):
 valid=[x for x in range(180) if mapping[y*360+x]!=65535]
 starts.append(min(valid) if valid else 0);ends.append(max(valid)+1 if valid else 0)
text+='static const unsigned short mandala_row_start[180]={'+','.join(map(str,starts))+'};\n'
text+='static const unsigned short mandala_row_end[180]={'+','.join(map(str,ends))+'};\n'
(p/'geometry.h').write_text(text,encoding='ascii')
