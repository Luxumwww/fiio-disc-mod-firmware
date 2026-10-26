import subprocess
from pathlib import Path
p=Path(__file__).resolve().parent
r=subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r5','-mmsa','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-mno-abicalls','-fno-pic','-no-pie','-fno-builtin','-nostdlib','-fuse-ld=lld','-Wl,-e,_start,-dynamic-linker,/lib/ld-linux-mipsn8.so.1','-L/root/disc-official-component-base-20261004-r1/base/lib',str(p/'probe.c'),'-Wl,-l:libc.so.6','-o',str(p/'probe')],capture_output=True,text=True)
print(r.stdout,r.stderr);raise SystemExit(r.returncode)


