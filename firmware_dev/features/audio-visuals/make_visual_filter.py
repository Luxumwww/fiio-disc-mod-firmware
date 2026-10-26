"""Generate a95-tap symmetric half-band Kaiser FIR for display spectra only."""
from pathlib import Path
import math,json
p=Path(__file__).resolve().parent
def i0(x):
 total=term=1.0
 for k in range(1,80):
  term*=x*x/(4*k*k);total+=term
  if term<total*1e-17:break
 return total
beta=7.0;radius=47
coef=[math.sin(math.pi*k/2)/(math.pi*k)*i0(beta*math.sqrt(1-(k/radius)**2))/i0(beta) for k in range(1,48,2)]
total=.5+2*sum(coef);center=.5/total;coef=[x/total for x in coef]
def value(x):
 s=format(x,'.10g');return s+('.0' if '.' not in s and 'e' not in s else '')+'f'
text='/* Generated display-only95-tap half-band FIR. Odd offsets1..47. */\n'
text+='static const float disc_visual_halfband_center='+value(center)+';\n'
text+='static const float disc_visual_halfband[24]={'+','.join(map(value,coef))+'};\n'
for name,r,b in (('first',7,6.0),('middle',15,6.0)):
 a=[math.sin(math.pi*k/2)/(math.pi*k)*i0(b*math.sqrt(1-(k/r)**2))/i0(b) for k in range(1,r+1,2)];z=.5+2*sum(a)
 text+='static const float disc_visual_halfband_'+name+'_center='+value(.5/z)+';\n'
 text+='static const float disc_visual_halfband_'+name+'['+str(len(a))+']={'+','.join(value(x/z) for x in a)+'};\n'
(p/'visual_filter_coefficients.h').write_text(text,encoding='ascii',newline='\n')
response={}
for output in (44100,48000):
 def db(f):
  a=center+2*sum(c*math.cos(2*math.pi*f*(2*j+1)/(output*2)) for j,c in enumerate(coef))
  return 20*math.log10(max(abs(a),1e-15))
 ripple=max(abs(db(i*20000/500)) for i in range(501))
 stop=max(db(output-20000+i*20000/500) for i in range(501))
 response[str(output)]={'20k_db':db(20000),'alias_to20k_db':db(output-20000),'0_20k_max_abs_db':ripple,'stop_max_db':stop}
 assert ripple<.1 and stop<-55,response
(p/'visual-filter-response.json').write_text(json.dumps(response,indent=2)+'\n');print(json.dumps(response,indent=2))
