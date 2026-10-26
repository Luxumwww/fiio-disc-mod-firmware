from pathlib import Path
import subprocess,hashlib,json
p=Path(__file__).resolve().parent;out=p/'build';out.mkdir(exist_ok=True)
base=Path('/root/disc-official-component-base-20261004-r1/base')
headers='/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include'
common=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-nostdlib','-fuse-ld=lld','-isystem',headers,'-L'+str(base/'lib')]
subprocess.run([*common,'-fPIC','-shared',str(p/'recorder.c'),'-Wl,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-crash-recorder.so')],check=True)
subprocess.run([*common,'-mno-abicalls','-fno-pic','-G0','-no-pie',str(p/'test_recorder.c'),'-Wl,-e,_start,-dynamic-linker,/lib/ld-linux-mipsn8.so.1,-l:libdl.so.2,-l:libc.so.6,-z,noexecstack','-o',str(out/'test_recorder')],check=True)
b=(out/'libdisc-crash-recorder.so').read_bytes();h=2166136261
for c in b:h=((h^c)*16777619)&0xffffffff
(p.parents[1]/'settings-debug/diag_provider_guard.h').write_text(f'#define DIAG_PROVIDER_SIZE {len(b)}u\n#define DIAG_PROVIDER_FNV {h}u\n')
(out/'build.json').write_text(json.dumps({'version':'0.3','sha256':hashlib.sha256(b).hexdigest(),'bytes':len(b),'MIPS_context_layout_static_asserts':'PASS'},indent=2)+'\n')
print((out/'build.json').read_text())
