"""Reproducible FFmpeg n4.2 DSD FIR lookup and a PCM decimation FIR."""
from pathlib import Path
import math,re,hashlib,json
p=Path(__file__).resolve().parent
text=(p/'vendor/dsd_tablegen.h').read_text()
half=[float(x) for x in re.search(r'htaps\[HTAPS\] = \{(.*?)\}',text,re.S).group(1).split(',') if x.strip()]
assert len(half)==48
def i0(x):
    total=term=1.0
    for k in range(1,80):
        term*=x*x/(4*k*k);total+=term
        if term<total*1e-16:break
    return total
def fir(rate,taps):
    fc=26000/rate;c=(taps-1)/2
    a=[]
    for n in range(taps):
        x=n-c;s=2*fc if x==0 else math.sin(2*math.pi*fc*x)/(math.pi*x)
        w=i0(9*math.sqrt(max(0,1-(x/c)**2)))/i0(9)
        a.append(s*w)
    z=sum(a);return [x/z for x in a]
def arr(name,values):
    return 'static const float '+name+'[]={\n'+',\n'.join(','.join(f'{x:.10e}f' for x in values[i:i+8]) for i in range(0,len(values),8))+'\n};\n'
tables=[]
for t in range(6):
    tables.append([sum((1 if (e>>(7-m))&1 else -1)*half[(5-t)*8+m] for m in range(8)) for e in range(256)])
out='/* DSD FIR coefficients: FFmpeg n4.2 / Sebastian Gesemann; see vendor license. */\n'
out+='static const float d2p_dsd_lut[6][256]={\n'+',\n'.join('{'+','.join(f'{v:.10e}f' for v in row)+'}' for row in tables)+'\n};\n'
for carrier,taps in [(176400,127),(352800,255)]:out+=arr(f'd2p_fir_{carrier}',fir(carrier*2,taps))
(p/'tables.h').write_text(out,encoding='ascii',newline='\n')
print(json.dumps({'tables_sha256':hashlib.sha256(out.encode()).hexdigest(),'output_rate':88200,'output_format':'S32_LE stereo','DSD64_fir_taps':127,'DSD128_fir_taps':255}))
