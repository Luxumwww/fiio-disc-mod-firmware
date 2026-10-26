from pathlib import Path
import subprocess
here=Path(__file__).resolve().parent
base=Path('/root/disc-official-component-base-20261004-r1/base')
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),'-L'+str(base/'usr/lib'),'-Wl,-l:libcrypto.so.1.1,-l:libdl.so.2,-l:libc.so.6,--no-undefined,-z,noexecstack',str(here/'wifi-config.c'),'-o',str(here/'service/libdisc-wifi-config.so')],check=True)
