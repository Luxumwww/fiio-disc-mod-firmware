from pathlib import Path
import subprocess,sys
P=Path(__file__).resolve().parent;B=Path('/root/disc-official-component-base-20261004-r1/base');O=P/'reports/1.0.0';O.mkdir(parents=True,exist_ok=True)
sys.path.insert(0,str(P.parent/'settings-debug'));from elf_bridge import import_map
player=(B/'usr/bin/mq_player').read_bytes();(P/'test_worker_guard.h').write_text(f'#define LS_ACCESS_GOT {import_map(player,"access")["got"]}u\n')
flags=['--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-fno-builtin','-fPIC','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Wno-unused-function']
subprocess.run(['clang',*flags,'-shared','-nostdlib','-fuse-ld=lld','-L'+str(B/'lib'),str(P/'test_native_worker.c'),'-Wl,-l:libc.so.6,-l:libdl.so.2,-l:libpthread.so.0,--no-undefined,-z,noexecstack','-o',str(O/'native-worker-test.so')],check=True)
subprocess.run(['gcc','-g','-O1','-fsanitize=address,undefined','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',str(P/'test_core.c'),'-o','/tmp/disc-scope-core-test'],check=True)
with (O/'core-tests.log').open('w') as log:subprocess.run(['/tmp/disc-scope-core-test'],stdout=log,stderr=subprocess.STDOUT,check=True)
print((O/'core-tests.log').read_text())
