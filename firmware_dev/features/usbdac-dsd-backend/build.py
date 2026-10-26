from pathlib import Path
import hashlib,struct,subprocess,json,sys
p=Path(__file__).resolve().parent
sys.path.insert(0,str(p.parent/'settings-debug'))
from elf_bridge import BASE,import_map
player=(BASE/'usr/bin/mq_player').read_bytes()
assert hashlib.sha256(player).hexdigest()=='a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc'
h=struct.unpack_from('<16sHHIIIIIHHHHHH',player)
segments=[struct.unpack_from('<IIIIIIII',player,h[5]+i*h[9]) for i in range(h[10])]
lines=['static const struct {unsigned address;unsigned char bytes[16];} dsb_guards[]={']
for address in (0x4591b0,0x4cb6b0,0x4cb534,0x4cb3e4,0x478354,0x47859c,0x4e9534,0x465970,0x465af8,0x465d64,0x465ec0,0x466338,0x466420,0x466980):
 s=next(s for s in segments if s[0]==1 and s[2]<=address and address+16<=s[2]+s[4]);offset=s[1]+address-s[2]
 lines.append('{0x%x,{%s}},'%(address,','.join(map(str,player[offset:offset+16]))))
lines.append('};');(p/'guard.h').write_text('\n'.join(lines)+'\n')
for name,address in [('pthread_create',0x83867c),('pcm_open',0x838740),('ioctl',0x83820c)]:assert int(import_map(player,name)['got'],16)==address
out=p/'build';out.mkdir(exist_ok=True)
cmd=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(BASE/'lib'),str(p/'backend.c'),'-Wl,-l:libpthread.so.0,-l:libdl.so.2,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-usbdac-dsd-backend.so')]
subprocess.run(cmd,check=True)
data=(out/'libdisc-usbdac-dsd-backend.so').read_bytes();v=2166136261
for b in data:v=((v^b)*16777619)&0xffffffff
(p/'provider_guard.h').write_text(f'#define DSB_SIZE {len(data)}u\n#define DSB_FNV {v}u\n')
abi=subprocess.check_output(['readelf','-h','-A','-W','-l',str(out/'libdisc-usbdac-dsd-backend.so')],text=True)
assert 'nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi and 'RWE' not in abi
(out/'abi.txt').write_text(abi)
report={'version':'1.0.5','bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'no_DSD_to_PCM':True,'no_native_DSD_init':True,'own_PCM_handle':True,'raw_worker_replacement':True,'constructor':False,'hearing':'pending'}
(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
