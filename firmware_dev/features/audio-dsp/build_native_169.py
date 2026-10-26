from pathlib import Path
import subprocess,json,hashlib,re
F=Path(__file__).resolve().parent.parent;P=F/'bluetooth-meter';base=Path('/root/disc-official-component-base-20261004-r1/base')
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),str(P/'meter.c'),'-Wl,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(P/'build/libdisc-bt-meter.so')],check=True)
subprocess.run(['python3',str(F/'settings-debug/build.py')],check=True)
meta=json.loads((P/'build/build.json').read_text());data=(P/'build/libdisc-bt-meter.so').read_bytes();version=re.search(r'releases/(1\.3\.\d+)',(P/'meter.c').read_text()).group(1);meta.update(version=version,menu_bridge_version='1.10.'+version.split('.')[-1],library_bytes=len(data),library_sha256=hashlib.sha256(data).hexdigest());(P/'build/build.json').write_text(json.dumps(meta,indent=2)+'\n');print(json.dumps(meta,indent=2))
