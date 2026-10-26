"""Experimental SRC only. Does not change/install the current DSP provider."""
from pathlib import Path
import subprocess,json,hashlib
p=Path(__file__).resolve().parent;o=p/'build';o.mkdir(exist_ok=True)
base=Path('/root/disc-official-component-base-20261004-r1/base');headers='/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include'
inc=['-DOUTSIDE_SPEEX','-DRANDOM_PREFIX=disc_quality','-DFLOATING_POINT','-I'+str(p/'third_party/speexdsp/include/speex'),'-I'+str(p/'third_party/speexdsp/libspeexdsp')]
common=['-O3','-ffp-contract=off','-fno-math-errno','-fvisibility=hidden',*inc]
for native in (False,True):
 for msa in ((False,True) if native else (False,)):
  name='mips-msa' if msa else 'mips-scalar' if native else 'host';obj=o/(name+'-speex.o');lib=o/(name+'.so')
  target=['--target=mipsel-linux-gnu','-march=mips32r5' if msa else '-march=mips32r2',*(['-mmsa'] if msa else []),'-mabi=32','-mhard-float','-mfp64','-mnan=2008','-isystem',headers] if native else []
  extra=['-include',str(p/'speex_msa.h')] if msa else []
  subprocess.run(['clang',*target,*common,*extra,'-fPIC','-c',str(p/'third_party/speexdsp/libspeexdsp/resample.c'),'-o',str(obj)],check=True)
  link=['-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),'-Wl,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack'] if native else ['-lm']
  subprocess.run(['clang',*target,*common,'-fPIC','-shared',str(p/'transport.c'),str(obj),*link,'-o',str(lib)],check=True)
  print(name,hashlib.sha256(lib.read_bytes()).hexdigest())
