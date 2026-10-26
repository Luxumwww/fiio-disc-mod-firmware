from pathlib import Path
import subprocess,struct,json,re
p=Path(__file__).resolve().parent;out=p/'build/probe_get_api'
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-fno-builtin','-mno-abicalls','-fno-pic','-G0','-no-pie','-nostdlib','-fuse-ld=lld','-Wl,-e,_start,-dynamic-linker,/lib/ld-linux-mipsn8.so.1,-z,noexecstack','-L/root/disc-official-component-base-20261004-r1/base/lib',str(p/'probe_get_api.c'),'-Wl,-l:libdl.so.2,-l:libc.so.6','-o',str(out)],check=True)
b=out.read_bytes();h=struct.unpack_from('<16sHHIIIIIHHHHHH',b)
loads=[struct.unpack_from('<IIIIIIII',b,h[5]+i*h[9]) for i in range(h[10])]
assert all(s[2]+s[5]<0x51a000 for s in loads if s[0]==1),'Private probe code mappings must not overlap its own binary.'
# Inspect the actual provider getter: the new guard must not call libc or any
# hardware operation in this first function; volatile byte loops stay inlined.
objdump='/root/disc-accel-research-20261006/toolchain/bin/mips-linux-gnu-objdump'
all_asm=subprocess.run([objdump,'-d',str(p/'build/libdisc-hwaccel.so')],capture_output=True,text=True,check=True).stdout
match=re.search(r'^([0-9a-f]+ <disc_accel_get_api>:\n.*?)(?=\n[0-9a-f]+ <|\Z)',all_asm,re.M|re.S)
assert match, 'Provider getter symbol missing from disassembly.'
asm=match.group(1)
assert 'jalr' not in asm and '\tjal\t' not in asm
(p/'build/getter.asm').write_text(asm)
print('Probe built; private data mappings bounded outside binary; provider getter contains no external function calls.')
