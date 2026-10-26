from pathlib import Path
import subprocess,hashlib,json
H=Path(__file__).resolve().parent
B=Path('/root/disc-official-component-base-20261004-r1/base')
S=Path('/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc')
O=H/'build';O.mkdir(exist_ok=True)
cmd=['clang','-std=gnu99','--target=mipsel-linux-gnu','--sysroot='+str(S),'-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(B/'lib'),str(H/'thermal_worker.c'),'-Wl,-l:libdl.so.2,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(O/'libthermal-worker.so')]
subprocess.run(cmd,check=True)
p=O/'libthermal-worker.so';r={'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()};(O/'build.json').write_text(json.dumps(r,indent=2));print(r)
