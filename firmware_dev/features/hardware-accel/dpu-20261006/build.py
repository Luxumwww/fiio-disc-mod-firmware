import subprocess
from pathlib import Path
import sys
p=Path(__file__).resolve().parent
name=sys.argv[1] if len(sys.argv)>1 else 'read_config'
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-fno-builtin','-mno-abicalls','-fno-pic','-G0','-no-pie','-nostdlib','-fuse-ld=lld','-Wl,-e,_start,-dynamic-linker,/lib/ld-linux-mipsn8.so.1,-z,noexecstack','-L/root/disc-official-component-base-20261004-r1/base/lib',str(p/(name+'.c')),'-Wl,-l:libc.so.6','-o',str(p/name)],check=True)



