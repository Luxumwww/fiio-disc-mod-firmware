from pathlib import Path
import math,colorsys
p=Path(__file__).resolve().parent
n=4096;bands=96;cells=148
def floats(values):
    result=[]
    for v in values:
        s=format(v,'.9g')
        if '.' not in s and 'e' not in s:s+='.0'
        result.append(s+'f')
    return ','.join(result)
lines=['#define SPECTRUM_N 4096u\n#define SPECTRUM_BANDS 96u\n#define SPECTRUM_CELLS 148u']
lines.append('static const float fft_window[4096]={'+floats(0.5-0.5*math.cos(2*math.pi*i/(n-1)) for i in range(n))+'};')
lines.append('static const float fft_cos[2048]={'+floats(math.cos(-2*math.pi*i/n) for i in range(n//2))+'};')
lines.append('static const float fft_sin[2048]={'+floats(math.sin(-2*math.pi*i/n) for i in range(n//2))+'};')
lines.append('static const unsigned short fft_reverse[4096]={'+','.join(str(int(format(i,'012b')[::-1],2)) for i in range(n))+'};')
edges=[20*(1000**(i/bands)) for i in range(bands+1)]
lines.append('static const float spectrum_frequency[97]={'+floats(edges)+'};')
pixels=[[[] for _ in range(cells)] for _ in range(bands)];grid=[]
for y in range(360):
    for x in range(360):
        dx=x-180;dy=y-180;radius=math.hypot(dx,dy)
        if radius>172:continue
        angle=(math.atan2(dy,dx)+math.pi/2)%(2*math.pi)
        if min(abs(radius-r) for r in (24,60,97,134,171))<0.55 or (radius>24 and abs(math.sin(6*angle))*radius/6<0.45):grid.append(y*360+x)
        cell=int(radius)-24
        if not 0<=cell<cells:continue
        position=angle/(2*math.pi)*bands
        band=int(math.floor(position+0.5))%bands;delta=abs(position-math.floor(position+0.5))
        # Distinct angular sectors never share pixels, so sparse erasure is safe.
        coverage=max(0,min(1,(0.37-delta)*2*math.pi*radius/bands+0.5))
        if coverage>0:pixels[band][cell].append((y*360+x,round(coverage*255)))
indices=[];alphas=[];offsets=[]
for band in pixels:
    starts=[]
    for cell in band:
        starts.append(len(indices));indices.extend(i for i,a in cell);alphas.extend(a for i,a in cell)
    starts.append(len(indices));offsets.append(starts)
lines.append('static const unsigned spectrum_offsets[96][149]={'+','.join('{'+','.join(map(str,row))+'}' for row in offsets)+'};')
lines.append('static const unsigned spectrum_pixels[%d]={%s};'%(len(indices),','.join(map(str,indices))))
lines.append('static const unsigned char spectrum_alpha[%d]={%s};'%(len(alphas),','.join(map(str,alphas))))
lines.append('static const unsigned spectrum_grid[%d]={%s};'%(len(grid),','.join(map(str,grid))))
colors=[]
for phase in range(256):
    r,g,b=colorsys.hsv_to_rgb(phase/256,0.76,0.96)
    colors.append((round(r*255)<<16)|(round(g*255)<<8)|round(b*255))
lines.append('static const unsigned spectrum_palette[256]={'+','.join('0x%06xu'%v for v in colors)+'};')
center=[];radii=[]
for y in range(157,204):
    for x in range(157,204):
        radius=math.hypot(x-180,y-180)
        if radius<23.5:center.append(y*360+x);radii.append(round(radius*256))
lines.append('static const unsigned center_pixels[%d]={%s};'%(len(center),','.join(map(str,center))))
lines.append('static const unsigned short center_radii[%d]={%s};'%(len(radii),','.join(map(str,radii))))
(p/'geometry.h').write_text('\n'.join(lines)+'\n',encoding='ascii',newline='\n')
assert len(set(indices))==len(indices)
print('FFT4096,96 bands,%d exclusive bar pixels; Hann/twiddles/bit reversal/grid precomputed'%len(indices))
