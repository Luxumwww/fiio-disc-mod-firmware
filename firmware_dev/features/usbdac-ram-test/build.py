from pathlib import Path
import hashlib,json,subprocess
p=Path(__file__).resolve().parent;out=p/'build';out.mkdir(exist_ok=True)
for name in ('test_packet','test_runtime','test_split_write','test_control_trace','test_format_profile','test_sync_probe'):
 cmd=['clang','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Wno-unused-function',str(p/(name+'.c')),'-pthread','-ldl','-o',str(out/name)]
 subprocess.run(cmd,check=True);subprocess.run([str(out/name)],check=True)
base=Path('/root/disc-official-component-base-20261004-r1/base')
cmd=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),str(p/'replay.c'),'-Wl,-l:libpthread.so.0,-l:libdl.so.2,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-ram-dop.so')]
subprocess.run(cmd,check=True)
b=(out/'libdisc-ram-dop.so').read_bytes();h=2166136261
for v in b:h=((h^v)*16777619)&0xffffffff
(p/'provider_guard.h').write_text(f'#define RAM_DOP_SIZE {len(b)}u\n#define RAM_DOP_FNV {h}u\n')
abi=subprocess.check_output(['readelf','-h','-A','-W','-l',str(out/'libdisc-ram-dop.so')],text=True)
assert 'nan2008' in abi and 'RWE' not in abi
(out/'abi.txt').write_text(abi)
(out/'build.json').write_text(json.dumps(dict(version='1.0.7',bytes=len(b),sha256=hashlib.sha256(b).hexdigest(),DAC_register_writes='No new DAC changes; prior24-bit receiver profile stays unarmed',PCM_reconfigure=False,realtime_split_frames=512,control_observer_seconds=5,control_observer_events=256,sync_only_flags=7,sync_control_fields_get_only=True,hearing='pending'),indent=2))
