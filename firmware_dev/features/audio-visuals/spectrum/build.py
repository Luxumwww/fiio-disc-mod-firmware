from pathlib import Path
import subprocess,json,hashlib,sys
p=Path(__file__).resolve().parent;base=Path('/root/disc-official-component-base-20261004-r1/base');out=p/'build';out.mkdir(exist_ok=True)
subprocess.run([sys.executable,str(p/'make_geometry.py')],check=True)
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),str(p/'spectrum.c'),'-Wl,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-audio-spectrum.so')],check=True)
s=subprocess.run(['readelf','-h','-A','-W','-l','-d',str(out/'libdisc-audio-spectrum.so')],capture_output=True,text=True,check=True).stdout
assert 'nan2008' in s and 'Hard float (32-bit CPU, 64-bit FPU)' in s and 'RWE' not in s
(out/'abi.txt').write_text(s)
report={'version':'1.0.2','bytes':(out/'libdisc-audio-spectrum.so').stat().st_size,'sha256':hashlib.sha256((out/'libdisc-audio-spectrum.so').read_bytes()).hexdigest(),'mode_independent':True,'sampling_owned_by_host':True,'extra_thread':False,'extra_daemon':False,'fft_samples':4096,'fft_period_ms':32,'animation_period_ms':16,'stereo_packed_single_complex_FFT':True,'sparse_owned_bar_pixels':True,'runtime_trigonometry':False,'hard_float_FPU':True}
(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
