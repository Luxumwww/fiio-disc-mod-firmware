from pathlib import Path
import subprocess,json,hashlib
p=Path(__file__).resolve().parent;s=p/'upstream';lg=s/'src/third_party/libgooglepinyin';b=p/'build';b.mkdir(exist_ok=True)
subprocess.run(['python3',str(s/'tools/gen_dict.py')],check=True)
sources=[lg/'src'/x for x in ['dictio.cpp','dictlist.cpp','dicttrie.cpp','lpicache.cpp','matrixsearch.cpp','mystdlib.cpp','ngram.cpp','pinyinime.cpp','searchutility.cpp','spellingtable.cpp','spellingtrie.cpp','splparser.cpp','sync.cpp','userdict.cpp','utf16char.cpp']]
base=Path('/root/disc-official-component-base-20261004-r1/base')
cmd=['clang++','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','--sysroot=/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc','-O2','-w','-std=c++11','-fno-exceptions','-fno-rtti','-fno-threadsafe-statics','-fno-use-cxa-atexit','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-DPINYINIME_RUNTIME_ONLY','-I'+str(lg/'include'),'-I'+str(s/'include'),'-include',str(lg/'port/pinyinime_compat.h'),*map(str,sources),str(s/'src/core/ime_engine_port.cpp'),str(p/'cpp_runtime.cpp'),'-L'+str(base/'lib'),'-Wl,-l:libpthread.so.0,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(b/'libdisc-ime.so')]
subprocess.run(cmd,check=True)
print(json.dumps({'bytes':(b/'libdisc-ime.so').stat().st_size,'sha256':hashlib.sha256((b/'libdisc-ime.so').read_bytes()).hexdigest()},indent=2))
