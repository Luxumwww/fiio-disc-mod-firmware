from pathlib import Path
import subprocess,json,hashlib
p=Path(__file__).resolve().parent;base=Path('/root/disc-official-component-base-20261004-r1/base');out=p/'build';out.mkdir(exist_ok=True)
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),str(p/'xy.c'),'-Wl,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-audio-xy.so')],check=True)
s=subprocess.run(['readelf','-h','-A','-W','-l',str(out/'libdisc-audio-xy.so')],capture_output=True,text=True,check=True).stdout
assert 'nan2008' in s and 'Hard float (32-bit CPU, 64-bit FPU)' in s and 'RWE' not in s
(out/'abi.txt').write_text(s);report={'version':'1.0.9','bytes':(out/'libdisc-audio-xy.so').stat().st_size,'sha256':hashlib.sha256((out/'libdisc-audio-xy.so').read_bytes()).hexdigest(),'mode_independent':True,'sampling_owned_by_host':True,'extra_thread':False,'extra_daemon':False};(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
