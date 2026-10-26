from pathlib import Path
import subprocess
P=Path(__file__).resolve().parent;base=Path('/root/disc-official-component-base-20261004-r1/base');sdk=Path('/root/disc-accel-research-20261006/toolchain')
subprocess.run([str(sdk/'bin/mips-linux-gnu-gcc'),'-EL','-march=mips32r5','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-c',str(P/'pcm_probe.c'),'-o',str(P/'build/pcm_probe.o')],check=True)
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r5','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-nostdlib','-no-pie','-fuse-ld=lld','-Wl,--dynamic-linker=/lib/ld-linux-mipsn8.so.1,-z,noexecstack',str(P/'build/crt1.o'),str(P/'build/crti.o'),str(P/'build/pcm_probe.o'),'-L'+str(base/'usr/lib'),'-Wl,-l:libasound.so.2.0.0','-L'+str(base/'lib'),'-Wl,-l:libc.so.6,-l:libgcc_s.so.1',str(P/'build/crtn.o'),'-o',str(P/'build/pcm-probe')],check=True)
