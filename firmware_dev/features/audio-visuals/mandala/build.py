from pathlib import Path
import subprocess,json,hashlib,sys
p=Path(__file__).resolve().parent;base=Path('/root/disc-official-component-base-20261004-r1/base');out=p/'build';out.mkdir(exist_ok=True)
subprocess.run([sys.executable,str(p/'make_geometry.py')],check=True)
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),str(p/'mandala.c'),'-Wl,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-audio-mandala.so')],check=True)
s=subprocess.check_output(['readelf','-h','-A','-W','-l',str(out/'libdisc-audio-mandala.so')],text=True)
assert 'nan2008' in s and 'Hard float (32-bit CPU, 64-bit FPU)' in s and 'RWE' not in s
(out/'abi.txt').write_text(s)
data=(out/'libdisc-audio-mandala.so').read_bytes()
report={'version':'1.0.4','bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'fft':'64x64 two-dimensional stereo-density FFT, six-fold projection','fft_period_ms':32,'animation_period_ms':16,'extra_thread':False,'extra_daemon':False,'audio_owned_by_host':True}
(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
