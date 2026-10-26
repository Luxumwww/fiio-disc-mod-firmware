"""Build hash-bound original-UI extension; no firmware/overlay activation."""
import hashlib,json,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
STOCK=Path('/root/disc-firmware-dev-v257/stock');OUT=HERE/'build/components'
mapping=json.loads((HERE/'reports/v257-integration-map.json').read_text())['binaries']['mq_ui']
data=(STOCK/'usr/bin/mq_ui').read_bytes();assert hashlib.sha256(data).hexdigest()==mapping['sha256']
fnv=2166136261
for byte in data:fnv=((fnv^byte)*16777619)&0xffffffff
geometry=(HERE/'build/artwork/spectrum-warp.bin').read_bytes();assert len(geometry)%12==0
header=[f'#define DISC_UI_FNV32 {fnv}u',f'#define DISC_UI_SIZE {len(data)}u',f'#define DISC_WARP_COUNT {len(geometry)//12}u',f'#define DISC_UI_GUARDS {len(mapping["anchors"])}u','static const struct {unsigned address;unsigned char bytes[16];} disc_ui_guards[DISC_UI_GUARDS]={']
for item in mapping['anchors']:header.append(' {'+item['address']+', {'+','.join(str(v) for v in bytes.fromhex(item['first_16_bytes']))+'}},')
header.append('};');(HERE/'ui_guard_generated.h').write_text('\n'.join(header)+'\n')
flags=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(STOCK/'lib')]
sources=['ui_extension.c','audio_worker.c','stream_core.c','spectrum_core.c']
target=OUT/'libdisc-spectrum-ui.so'
subprocess.run(flags+[str(HERE/name) for name in sources]+['-Wl,-l:libpthread.so.0','-Wl,-l:libdl.so.2','-Wl,-l:libc.so.6','-Wl,--no-undefined,-z,execstack,-soname,libdisc-spectrum-ui.so','-o',str(target)],check=True)
abi=subprocess.run(['readelf','-h','-A','-d','-W','-l',str(target)],capture_output=True,text=True,check=True).stdout
assert 'nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi and 'RWE' in abi
(HERE/'reports/ui-extension-abi.txt').write_text(abi)
report={'ui_sha256_required':mapping['sha256'],'ui_runtime_guard':'full-file FNV32 + file length + 22 mapped text signatures (version identity, not adversarial authenticity)','library_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'bytes':target.stat().st_size,'source_sha256':{name:hashlib.sha256((HERE/name).read_bytes()).hexdigest() for name in sources},'stock_binary_changed':False,'overlay_changed':False,'ui_runtime_verified':False,'device_verified':False}
(HERE/'reports/ui-extension-build.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
