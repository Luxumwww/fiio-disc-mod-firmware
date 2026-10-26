"""Windowed-sinc anti-alias FIRs for integer decimation 1..16. Q15."""
import math
from pathlib import Path
out=['/* Generated integer-decimation low-pass filters, cutoff .42/D, Hann 16D+1 taps. */','static const short disc_fir[17][257]={',' {0},',' {32767},']
for d in range(2,17):
 n=16*d+1;fc=.42/d;values=[]
 for i in range(n):
  x=i-(n-1)/2
  h=2*fc if not x else math.sin(2*math.pi*fc*x)/(math.pi*x)
  values.append(h*(.5-.5*math.cos(2*math.pi*i/(n-1))))
 scale=sum(values);q=[round(v/scale*32767) for v in values];q[n//2]+=32767-sum(q)
 out.append(' {'+','.join(map(str,q))+'},')
out.append('};')
Path(__file__).with_name('fir_tables.h').write_text('\n'.join(out)+'\n')
