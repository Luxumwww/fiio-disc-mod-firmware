"""Isolated native EQ oracle; no production edit or device command."""
from pathlib import Path
import struct
import subprocess,hashlib,json
h=Path(__file__).resolve().parent;o=h/'reports/eq-candidate';o.mkdir(exist_ok=True,parents=True);base=Path('/root/disc-official-component-base-20261004-r1/base')
stock=base/'usr/bin/mq_player';assert hashlib.sha256(stock.read_bytes()).hexdigest()=='a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc'
data=stock.read_bytes();eh=struct.unpack_from('<16sHHIIIIIHHHHHH',data);segments=[struct.unpack_from('<IIIIIIII',data,eh[5]+i*eh[9]) for i in range(eh[10])];seg=next(s for s in segments if s[0]==1 and s[2]<=0x45b000 and 0x45c000<=s[2]+s[4]);offset=seg[1]+0x45b000-seg[2];code=data[offset:offset+4096]
(o/'stock_function.h').write_text('static const unsigned char stock_eq_function[]={'+','.join(map(str,code))+'};\n')
flags=['--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-ffp-contract=off','-fno-math-errno','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Wno-unused-function','-mno-abicalls','-fno-pic','-G0','-no-pie','-nostdlib','-fuse-ld=lld','-isystem','/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include','-L'+str(base/'lib')]
(o/'candidate.c').write_text('#define disc_dsp_native_eq native_eq_candidate\n#include "../../../audio-dsp/native_eq_msa.c"\n')
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r5','-mmsa','-fno-vectorize','-fno-slp-vectorize','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-ffp-contract=off','-fno-math-errno','-fno-pic','-mno-abicalls','-G0','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Wno-unused-function','-isystem','/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include','-c',str(o/'candidate.c'),'-o',str(o/'candidate.o')],check=True)
subprocess.run(['clang',*flags,str(h/'test_native_eq_candidate.c'),str(o/'candidate.o'),'-Wl,--image-base=0x09000000,-e,_start,-dynamic-linker,/lib/ld-linux-mipsn8.so.1,-l:libm.so.6,-l:libc.so.6,-l:libdl.so.2,--no-undefined,-z,noexecstack','-o',str(o/'native-eq-test')],check=True)
r=subprocess.run(['qemu-mipsel','-cpu','P5600','-L',str(base),str(o/'native-eq-test')],capture_output=True,text=True,timeout=60);(o/'qemu.log').write_text(r.stdout+r.stderr);print(r.stdout+r.stderr);assert r.returncode==0
(o/'result.json').write_text(json.dumps(dict(result='PASS',qemu=r.stdout,source_sha256={str(p.relative_to(h)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (h/'native_eq_candidate.h',h/'test_native_eq_candidate.c',Path(__file__).resolve(),o/'stock_function.h',o/'native-eq-test')}),indent=2)+'\n')
