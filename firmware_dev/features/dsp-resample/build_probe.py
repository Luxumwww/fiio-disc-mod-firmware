from pathlib import Path
import subprocess
p=Path(__file__).resolve().parent;out=p/'build';out.mkdir(exist_ok=True)
base=Path('/root/disc-official-component-base-20261004-r1/base');headers='/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include'
flags=['--target=mipsel-linux-gnu','-march=mips32r5','-mmsa','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-isystem',headers,'-L'+str(base/'lib')]
subprocess.run(['clang',*flags,str(p/'bench_provider.c'),'-Wl,-l:libm.so.6,-l:libc.so.6,-l:libdl.so.2,--no-undefined,-z,noexecstack','-o',str(out/'bench-provider.so')],check=True);print('Isolated single-effect benchmark compiled')
