from pathlib import Path
import subprocess
h=Path(__file__).resolve().parent;o=h/'reports/1.3.215/tools';o.mkdir(exist_ok=True,parents=True)
base=Path('/root/disc-official-component-base-20261004-r1/base');headers='/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include'
flags=['--target=mipsel-linux-gnu','-std=gnu11','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-nostdlib','-fuse-ld=lld','-isystem',headers,'-L'+str(base/'lib')]
subprocess.run(['clang',*flags,'-mno-abicalls','-fno-pic','-G0','-no-pie',str(h/'rescue_uac_214.c'),'-Wl,-e,_start,-dynamic-linker,/lib/ld-linux-mipsn8.so.1,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(o/'rescue-uac')],check=True)
subprocess.run(['clang',*flags,'-shared','-fPIC',str(h/'hot_uac_owner_215.c'),'-Wl,-l:libc.so.6,-l:libdl.so.2,-l:libpthread.so.0,--no-undefined,-z,noexecstack','-o',str(o/'hot-owner.so')],check=True)
print('Finite rescue and actual-owner helper built')
