from pathlib import Path
import subprocess,hashlib,json,sys
p=Path(__file__).resolve().parent;out=p/'build';out.mkdir(exist_ok=True)
base=Path('/root/disc-official-component-base-20261004-r1/base')
cmd=['clang','--target=mipsel-linux-gnu','-march=mips32r5','-mmsa','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O3','-DDISC_HALL_SCALAR','-DDISC_DSP_UNITY','-fvisibility=hidden','-fno-semantic-interposition','-fno-math-errno','-ffp-contract=fast','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),str(p/'dsp.c'),'-Wl,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-audio-dsp.so')]
port=p/'third_party/dragonfly-port'
sources=['allpass','biquad','comb','delay','delayline','earlyref','efilter','revbase','slot','utils','zrev','zrev2']
cppflags=['clang++',*cmd[1:cmd.index('-O3')],'-O3','-std=c++11','-DLIBFV3_FLOAT','-nostdinc++','-fno-exceptions','-fno-rtti','-fno-threadsafe-statics','-fno-semantic-interposition','-fno-math-errno','-ffp-contract=fast','-fvisibility=hidden','-fPIC','-I'+str(port),'-Wno-deprecated-register','-c']
ffobj=out/'ffhaas_core.o'
subprocess.run(['clang',*cmd[1:cmd.index('-O3')],'-O3','-ffp-contract=off','-fno-semantic-interposition','-fno-math-errno','-fvisibility=hidden','-fPIC','-Wall','-Wextra','-Werror','-c',str(p/'ffhaas_core.c'),'-o',str(ffobj)],check=True)
stobj=out/"stereofx_core.o"
subprocess.run(["clang",*cmd[1:cmd.index("-O3")],"-O2","-fno-vectorize","-fno-slp-vectorize","-ffp-contract=off","-fno-semantic-interposition","-fno-math-errno","-fvisibility=hidden","-fPIC","-Wall","-Wextra","-Werror","-c",str(p/"stereofx_core.c"),"-o",str(stobj)],check=True)
tbobj=out/'tube2_core.o'
subprocess.run(['clang',*cmd[1:cmd.index('-O3')],'-O2','-fno-vectorize','-fno-slp-vectorize','-ffp-contract=off','-fno-semantic-interposition','-fno-math-errno','-fvisibility=hidden','-fPIC','-Wall','-Wextra','-Werror','-c',str(p/'tube2_core.c'),'-o',str(tbobj)],check=True)
psobj=out/'powersag2_core.o'
subprocess.run(['clang',*cmd[1:cmd.index('-O3')],'-O2','-fno-vectorize','-fno-slp-vectorize','-ffp-contract=off','-fno-semantic-interposition','-fno-math-errno','-fvisibility=hidden','-fPIC','-Wall','-Wextra','-Werror','-c',str(p/'powersag2_core.c'),'-o',str(psobj)],check=True)
eqobj=out/'native_eq_msa.o'
subprocess.run(['clang',*cmd[1:cmd.index('-O3')],'-O2','-fno-vectorize','-fno-slp-vectorize','-ffp-contract=off','-fno-semantic-interposition','-fno-math-errno','-fvisibility=hidden','-fPIC','-isystem','/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include','-Wall','-Wextra','-Werror','-c',str(p/'native_eq_msa.c'),'-o',str(eqobj)],check=True)
src=p.parent/'dsp-resample'
subprocess.run([sys.executable,str(src/'build_soxr.py'),'--mips-only','--objects-only'],check=True)
soxr_objects=[src/'build/soxr'/('mips-'+name+'.o') for name in ('soxr','data-io','dbesi0','filter','fft4g64','cr','cr64')]
src_objects=[]
for source in ('transport_soxr.c','soxr_compat.c'):
 obj=out/(source+'.o');subprocess.run(['clang',*cmd[1:cmd.index('-O3')],'-O3','-ffp-contract=off','-fno-math-errno','-fPIC','-fvisibility=hidden','-DSOXR_LIB','-I'+str(src/'build/soxr'),'-isystem','/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include','-c',str(src/source),'-o',str(obj)],check=True);src_objects.append(obj)
objects=[str(ffobj),str(stobj),str(tbobj),str(psobj),str(eqobj),*[str(o) for o in soxr_objects+src_objects]]
for source in [p/'dragonfly_adapter.cpp',*[port/'freeverb'/(name+'.cpp') for name in sources]]:
 obj=out/(source.stem+'.o');subprocess.run([*cppflags,str(source),'-o',str(obj)],check=True);objects.append(str(obj))
cmd[cmd.index('-Wl,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack')]='-Wl,-l:libm.so.6,-l:libc.so.6,-l:libpthread.so.0,-l:libgcc_s.so.1,--no-undefined,-z,noexecstack,-Bsymbolic'
cmd[cmd.index('-o'):cmd.index('-o')]=objects
subprocess.run(cmd,check=True)
abi=subprocess.check_output(['readelf','-h','-A','-l','-W',str(out/'libdisc-audio-dsp.so')],text=True)
assert 'nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi and 'RWE' not in abi
(out/'abi.txt').write_text(abi)
data=(out/'libdisc-audio-dsp.so').read_bytes();h=2166136261
for b in data:h=((h^b)*16777619)&0xffffffff
(p/'provider_guard.h').write_text(f'#define DSP_PROVIDER_SIZE {len(data)}u\n#define DSP_PROVIDER_FNV {h}u\n')
(out/'build.json').write_text(json.dumps({'version':'1.17.0','bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'new_thread':False,'DX8_removed':True,'IR_files':0,'own_audio_stream':False},indent=2)+'\n')
print((out/'build.json').read_text())
