"""Build a guarded device-side bridge without changing stock/profile ELF files."""
from pathlib import Path
import hashlib,json,struct,subprocess
here=Path(__file__).resolve().parent
stock=(here/'adbd-stock.bin').read_bytes()
both=(here/'service/profiles/both/adbd').read_bytes()
assert hashlib.sha256(stock).hexdigest()=='a895779fe77ef8c76752bee0e7b733ffd783019582bbfb6e762861093aace6d5'
assert hashlib.sha256(both).hexdigest()=='8fed392edc0040f9cafe3ffad97466941807368a805394b6daa6760fa1ac0f88'
def fnv(data):
 h=2166136261
 for b in data:h=((h^b)*16777619)&0xffffffff
 return h
h=struct.unpack_from('<16sHHIIIIIHHHHHH',stock)
rows=[struct.unpack_from('<IIIIIIIIII',stock,h[6]+i*h[11]) for i in range(h[12])]
names_row=rows[h[13]];names=stock[names_row[4]:names_row[4]+names_row[5]]
sec={names[r[0]:].split(b'\0')[0].decode():r for r in rows}
sym=sec['.dynsym'];strings=sec['.dynstr'];table=stock[strings[4]:strings[4]+strings[5]]
index=next(i for i in range(sym[5]//16) if table[struct.unpack_from('<I',stock,sym[4]+i*16)[0]:].split(b'\0')[0]==b'snprintf')
rel=sec['.rel.plt'];cells=[a for a,info in (struct.unpack_from('<II',stock,p) for p in range(rel[4],rel[4]+rel[5],8)) if info>>8==index]
assert len(cells)==1
strncmp_index=next(i for i in range(sym[5]//16) if table[struct.unpack_from('<I',stock,sym[4]+i*16)[0]:].split(b'\0')[0]==b'strncmp')
strncmp_cells=[a for a,info in (struct.unpack_from('<II',stock,p) for p in range(rel[4],rel[4]+rel[5],8)) if info>>8==strncmp_index]
assert len(strncmp_cells)==1
address=0x40e138;guard=stock[address-0x400000:address-0x400000+32]
assert guard.hex()=='4200063c1c8bc624640005241800a4274c69100c25380002cc68100c1800a427'
lines=[f'#define ADBD_SIZE {len(stock)}u',f'#define ADBD_STOCK_FNV {fnv(stock)}u',f'#define ADBD_BOTH_FNV {fnv(both)}u',f'#define SNPRINTF_GOT {cells[0]}u','#define TCPIP_FORMAT 0x418b1cu','#define TCPIP_RETURN 0x40e150u',f'#define TCPIP_GUARD_ADDRESS {address}u','static const unsigned char tcpip_guard[]={'+','.join(map(str,guard))+'};']
exit_guard=stock[0xcda0:0xcdc0]
assert exit_guard.hex()=='060006245c84a5243069100c25200002d5ff4010d4a9628ed9ff00104c00bf8f'
lines += [f'#define STRNCMP_GOT {strncmp_cells[0]}u','#define TCPIP_NAME 0x41845cu','#define TCPIP_EXIT_RETURN 0x40cdb0u','#define TCPIP_EXIT_GUARD_ADDRESS 0x40cda0u','static const unsigned char tcpip_exit_guard[]={'+','.join(map(str,exit_guard))+'};']
(here/'tcpip-guard.h').write_text('\n'.join(lines)+'\n')
header=here.parents[1]/'reviews/deepseek-probe-r6-20261004/suggested-fix/native/src'
out=here/'tcpip-module-source/libdisc-adb-tcpip.so'
out.parent.mkdir(exist_ok=True)
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-I'+str(header),'-L/root/disc-adb-test-20261004-r1/base/lib','-Wl,-l:libc.so.6,--no-undefined,-z,noexecstack',str(here/'tcpip.c'),'-o',str(out)],check=True)
report={'version':'1.1.0','snprintf_got':hex(cells[0]),'strncmp_got':hex(strncmp_cells[0]),'exit_guard_hex':exit_guard.hex(),'guard_address':hex(address),'guard_hex':guard.hex(),'supported_port':5555,'stock_profile_binaries_unchanged':True,'bytes':out.stat().st_size,'sha256':hashlib.sha256(out.read_bytes()).hexdigest()}
(here/'tcpip-build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
