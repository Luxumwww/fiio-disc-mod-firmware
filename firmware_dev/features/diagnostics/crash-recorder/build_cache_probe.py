from pathlib import Path
import subprocess
p=Path(__file__).resolve().parent
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-fno-builtin','-nostdlib','-fuse-ld=lld','-isystem','/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include','-L/root/disc-official-component-base-20261004-r1/base/lib','-mno-abicalls','-fno-pic','-G0','-no-pie',str(p/'cache_probe.c'),'-Wl,-e,_start,-dynamic-linker,/lib/ld-linux-mipsn8.so.1,-l:libc.so.6,-z,noexecstack','-o',str(p/'build/cache_probe')],check=True)
