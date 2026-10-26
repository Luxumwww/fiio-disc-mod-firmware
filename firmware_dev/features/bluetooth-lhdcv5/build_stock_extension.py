"""Generate exact stock call-site guards and build a small service extension."""
from pathlib import Path
import hashlib,json,struct,subprocess
P=Path(__file__).resolve().parent
stock=(P.parent/'bluetooth-meter/bluealsa-v4.0.0.bin').read_bytes()
assert hashlib.sha256(stock).hexdigest()=='773320d3c61f35ece83becdada68e7417068e81dbd48b22bedc63a985404dd9d'
phoff=struct.unpack_from('<I',stock,28)[0]
phsize,phnum=struct.unpack_from('<HH',stock,42)
segments=[]
for i in range(phnum):
 t,off,addr,_,size,_,flags,_=struct.unpack_from('<8I',stock,phoff+i*phsize)
 if t==1:segments.append((addr,off,size,flags))
def at(addr,size=4):
 for va,off,n,_ in segments:
  if va<=addr and addr+size<=va+n:return stock[off+addr-va:off+addr-va+size]
 raise ValueError(hex(addr))
targets=[0x4048ec,0x404830,0x4054c4,0x40561c,0x4058e4,0x405cc4,0x416024,0x40bf18,0x4052a0,0x40cef0]
sites=[]
# Executable instructions only; exclude rodata/PLT to avoid treating data as JAL.
for addr in range(0x403dc0,0x419c00,4):
 word=struct.unpack('<I',at(addr))[0]
 if word>>26 not in (2,3):continue
 target=((addr+4)&0xf0000000)|((word&0x3ffffff)<<2)
 if target in targets:sites.append((addr,word,targets.index(target)))
print({hex(t):sum(h==i for _,_,h in sites) for i,t in enumerate(targets)},flush=True)
assert all(any(hook==i for _,_,hook in sites) for i in range(len(targets)))
assert at(0x41ace0,8)==bytes(8)
guards=[]
for addr in range(0x40db34,0x40db80,4):guards.append((addr,struct.unpack('<I',at(addr))[0]))
for addr in targets+[0x40b660,0x40cddc,0x40d13c,0x414660,0x414198,0x4144b4,0x40c14c]:
 for offset in range(0,16,4):guards.append((addr+offset,struct.unpack('<I',at(addr+offset))[0]))
for offset in range(0,32,4):guards.append((0x41acc8+offset,struct.unpack('<I',at(0x41acc8+offset))[0]))
pages=sorted({addr&~4095 for addr,_,_ in sites}|{0x41a000,0x40d000})
header='static const uint8_t stock_header[64]={'+','.join(str(n) for n in stock[:64])+'};\n'
header+='static const struct {uint32_t address,word;} guards[]={\n'+''.join(f'{{0x{a:x},0x{w:08x}}},\n' for a,w in guards)+'};\n'
header+='static const struct {uint32_t address,word,hook;} sites[]={\n'+''.join(f'{{0x{a:x},0x{w:08x},{h}}},\n' for a,w,h in sites)+'};\n'
header+='static const uint32_t pages[]={'+','.join(hex(a) for a in pages)+'};\n'
(P/'build/stock_sites.h').write_text(header)
(P/'reports/stock-call-sites.json').write_text(json.dumps({'targets':targets,'sites':sites,'pages':pages},indent=2))
sdk=Path('/root/disc-accel-research-20261006/toolchain')
base=Path('/root/disc-official-component-base-20261004-r1/base')
subprocess.run([str(sdk/'bin/mips-linux-gnu-gcc'),'-EL','-march=mips32r5','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-std=c11','-fPIC','-fvisibility=hidden','-Wall','-Wextra','-Werror','-c',str(P/'stock_extension.c'),'-o',str(P/'build/stock_extension.o')],check=True)
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r5','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-shared','-nostdlib','-fuse-ld=lld',str(P/'build/stock_extension.o'),'-L'+str(P/'build'),'-l:libdisc-lhdcv5.so','-L'+str(base/'lib'),'-Wl,-l:libpthread.so.0,-l:libc.so.6,-l:libgcc_s.so.1,--no-undefined,-z,noexecstack,-rpath,$ORIGIN','-o',str(P/'build/libdisc-lhdcv5-stock.so')],check=True)
out=P/'build/libdisc-lhdcv5-stock.so'
print(json.dumps({'bytes':out.stat().st_size,'sha256':hashlib.sha256(out.read_bytes()).hexdigest(),'call_sites':len(sites),'pages':len(pages)},indent=2))

