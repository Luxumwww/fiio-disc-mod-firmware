"""Build an isolated performance candidate, leaving production/providers intact."""
from pathlib import Path
import subprocess,json,hashlib
h=Path(__file__).resolve().parent;d=h.parent/'audio-dsp';out=h/'reports/perf-206';out.mkdir(parents=True,exist_ok=True)
base=Path('/root/disc-official-component-base-20261004-r1/base')
old=(d/'dsp.c').read_text();s=old.replace('#include "warmgloss.h"','#include "'+str(h/'reports/perf-204/candidate/warmgloss.h')+'"')
needle=' float chain_target=d->mask&&d->available&&active_count?1:0;'
assert s.count(needle)==1;s=s.replace(needle,needle+'\n Dsp *single_warm=d->is_chain&&active_count==1&&active_types[0]==DISC_WARMGLOSS_TYPE?active[0]:0;')
needle='   if(d->chain_mix>0)for(unsigned j=0;j<active_count;j++)effect_sample(active[j],active_types[j],io);'
new='''   if(d->chain_mix>0){if(single_warm){float mix=ramp(single_warm,16,DISC_WARMGLOSS_MASK);if(single_warm->warmgloss&&((single_warm->mask&DISC_WARMGLOSS_MASK)||mix>0)){float wet[2]={io[0],io[1]};disc_warmgloss_sample(single_warm->warmgloss,wet);for(unsigned c=0;c<2;c++)io[c]+=(wet[c]-io[c])*mix;}}else for(unsigned j=0;j<active_count;j++)effect_sample(active[j],active_types[j],io);}'''
assert s.count(needle)==1;s=s.replace(needle,new);(out/'candidate-dsp.c').write_text(s)
flags=['--target=mipsel-linux-gnu','-march=mips32r5','-mmsa','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O3','-DDISC_HALL_SCALAR','-DDISC_DSP_UNITY','-fvisibility=hidden','-fno-semantic-interposition','-fno-math-errno','-ffp-contract=fast','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-isystem','/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include','-I'+str(d),'-L'+str(base/'lib')]
names=['ffhaas_core','stereofx_core','tube2_core','powersag2_core','dragonfly_adapter','allpass','biquad','comb','delay','delayline','earlyref','efilter','revbase','slot','utils','zrev','zrev2']
subprocess.run(['clang',*flags,str(out/'candidate-dsp.c'),*[str(d/'build'/(n+'.o')) for n in names],'-Wl,-l:libm.so.6,-l:libc.so.6,-l:libpthread.so.0,--no-undefined,-z,noexecstack,-Bsymbolic','-o',str(out/'candidate.so')],check=True)
subprocess.run(['clang',*flags,'-Wno-unused-function',str(h/'bench_single_warm.c'),'-Wl,-l:libm.so.6,-l:libc.so.6,-l:libdl.so.2,--no-undefined,-z,noexecstack','-o',str(out/'compare.so')],check=True)
data={str(p.relative_to(h.parent)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (d/'dsp.c',d/'build/libdisc-audio-dsp.so',out/'candidate-dsp.c',out/'candidate.so',out/'compare.so',h/'bench_single_warm.c')};(out/'build.json').write_text(json.dumps(data,indent=2)+'\n');print('Built isolated full DSP candidate; production unchanged')
