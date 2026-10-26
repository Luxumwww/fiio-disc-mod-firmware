from pathlib import Path
import subprocess,re,sys
p=Path(__file__).resolve().parent;src=p/'third_party/soxr/src';o=p/'build/soxr';o.mkdir(exist_ok=True,parents=True)
enabled={'HAVE_FENV_H','HAVE_STDBOOL_H','HAVE_STDINT_H','HAVE_LRINT','WITH_CR64','WITH_HI_PREC_CLOCK'}
config=(p/'third_party/soxr/soxr-config.h.in').read_text()
config=re.sub(r'#cmakedefine01 (\w+)',lambda m:'#define '+m[1]+' '+str(int(m[1] in enabled)),config)
(o/'soxr-config.h').write_text(config)
common=['-O3','-fno-math-errno','-ffp-contract=off','-fPIC','-fvisibility=hidden','-DSOXR_LIB','-Datexit=disc_soxr_register_exit','-DSOXR_VISIBILITY','-DSOXR_THIS_VERSION_STR="0.1.3"','-I'+str(o),'-I'+str(src)]
base=Path('/root/disc-official-component-base-20261004-r1/base');headers='/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include'
for native in ((True,) if '--mips-only' in sys.argv else (False,True)):
 name='mips' if native else 'host';target=['--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-isystem',headers] if native else []
 objects=[]
 for f in ('soxr','data-io','dbesi0','filter','fft4g64','cr','cr64'):
  obj=o/(name+'-'+f+'.o');subprocess.run(['clang',*target,*common,'-c',str(src/(f+'.c')),'-o',str(obj)],check=True);objects.append(str(obj))
 if '--objects-only' in sys.argv:continue
 link=['-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),'-Wl,-l:libm.so.6,-l:libc.so.6,-l:libgcc_s.so.1,--no-undefined,-z,noexecstack'] if native else ['-lm']
 subprocess.run(['clang',*target,*common,'-shared',str(p/'soxr_compat.c'),*objects,*link,'-o',str(o/(name+'.so'))],check=True);print(name,'soxr64 VHQ built')
 subprocess.run(['clang',*target,*common,'-shared',str(p/'transport_soxr.c'),str(p/'soxr_compat.c'),*objects,*link,'-o',str(o/(name+'-transport.so'))],check=True)
