from pathlib import Path
import subprocess,hashlib,json
P=Path(__file__).resolve().parent
host=P.parent/'bluetooth-meter';base=Path('/root/disc-official-component-base-20261004-r1/base')
out=P/'reports/receiver-fix-111';out.mkdir(exist_ok=True)
cmd=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),str(host/'meter.c'),'-Wl,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(host/'build/libdisc-bt-meter.so')]
subprocess.run(cmd,check=True)
subprocess.run(['python3',str(P.parent/'settings-debug/build.py')],check=True)
subprocess.run(['python3',str(P/'build_stock_extension.py')],check=True)
files=[host/'build/libdisc-bt-meter.so',P.parent/'settings-debug/build/disc-settings.so',P/'build/libdisc-lhdcv5-stock.so']
(out/'build.json').write_text(json.dumps({str(f):{'sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'bytes':f.stat().st_size} for f in files},indent=2))
