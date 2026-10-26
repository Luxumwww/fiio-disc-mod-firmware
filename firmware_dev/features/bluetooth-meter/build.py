"""Build the reviewed module and current menu bridge with the existing WSL toolchain."""
from pathlib import Path
import hashlib
import json
import struct
import subprocess
import sys
import os

HERE = Path(__file__).resolve().parent
BASE = Path('/root/disc-official-component-base-20261004-r1/base')
subprocess.run([sys.executable,str(HERE.parent/'hardware-accel/runtime-provider/build.py')],check=True)
subprocess.run([sys.executable,str(HERE.parent/'audio-visuals/xy/build.py')],check=True)
subprocess.run([sys.executable,str(HERE.parent/'audio-visuals/polar/build.py')],check=True)
subprocess.run([sys.executable,str(HERE.parent/'audio-visuals/spectrum/build.py')],check=True)
subprocess.run([sys.executable,str(HERE.parent/'audio-visuals/mandala/build.py')],check=True)
subprocess.run([sys.executable, str(HERE/'make_assets.py')], check=True)
ui = (BASE/'usr/bin/mq_ui').read_bytes()
assert hashlib.sha256(ui).hexdigest() == 'f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226'
header = struct.unpack_from('<16sHHIIIIIHHHHHH', ui)
ph = [struct.unpack_from('<IIIIIIII',ui,header[5]+i*header[9]) for i in range(header[10])]
addresses = [0x4e9a70,0x4e969c,0x52a8a4,0x52cb10,0x52cb44,0x4f8838,0x4f6b60,0x4f6af0,0x4f6b00,0x4f6b10,0x4f6b20,0x48831c,0x4e42e8,0x4e45bc,0x52df64,0x52f2bc,0x4eb680,0x4ebc14,0x4e96d0,0x4e97f8,0x4f6df0,0x4f6b50,0x4f6e60,0x4f6c40,0x4e39ec,0x4e3934,0x4e3924,0x51a7e0,0x4e26b4,0x525420,0x52dcac,0x4f6e50,0x52f4f0]
addresses += [0x48b28c,0x477e8c,0x461760,0x479090,0x4eb600,0x4670ac,0x69dcf0,0x695540]
addresses += [0x4777e0,0x4763e0]
addresses += [0x565dcc,0x51ab20,0x51ab0c]
addresses += [0x477d50,0x477b00,0x4ece1c,0x4e39ec]
addresses += [0x475840,0x475878,0x48b28c,0x4e9994,0x475f90]
addresses += [0x464708,0x51ab5c,0x484234]
addresses += [0x51b3e0,0x51b7b0,0x51b4b8]
addresses += [0x46d88c,0x485ef8,0x4f8850,0x4f889c,0x486620,0x4e2734,0x4f7e98,0x488860]
addresses += [0x4f8304,0x4f15ac,0x4f02c8]
addresses += [0x47f740,0x47f74c,0x47fa6c,0x4f0268]
addresses += [0x4790b0,0x435bf0,0x47f8f0,0x480228]
addresses += [0x46b9cc,0x4e392c]
addresses += [0x4ef6f4]
addresses += [0x4f3f10,0x52607c]  # Native font/text measurement for USB DAC home.
addresses += [0x46d5c0,0x46d848,0x4e394c,0x5030a0,0x5030e4]
lines = ['static const struct {unsigned address;unsigned char bytes[16];} bt_guards[]={']
for addr in addresses:
    segment = next(p for p in ph if p[0]==1 and p[2]<=addr and addr+16<=p[2]+p[4])
    offset = segment[1]+addr-segment[2]
    lines.append('{0x%x,{%s}},' % (addr,','.join(map(str,ui[offset:offset+16]))))
lines.append('};')
player=(BASE/'usr/bin/mq_player').read_bytes()
phdr=struct.unpack_from('<16sHHIIIIIHHHHHH',player)
segments=[struct.unpack_from('<IIIIIIII',player,phdr[5]+i*phdr[9]) for i in range(phdr[10])]
lines.append('static const struct {unsigned address;unsigned char bytes[16];} player_guards[]={')
for addr in (0x4e0e20,0x4cb3e4):
 segment=next(p for p in segments if p[0]==1 and p[2]<=addr and addr+16<=p[2]+p[4]);offset=segment[1]+addr-segment[2]
 lines.append('{0x%x,{%s}},'%(addr,','.join(map(str,player[offset:offset+16]))))
lines.append('};')
(HERE/'abi_guard.h').write_text('\n'.join(lines)+'\n')
out=HERE/'build';out.mkdir(exist_ok=True)
cmd=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(BASE/'lib')]
if os.environ.get('DISC_STOCK_QUIET')=='1':cmd.append('-DDISC_STOCK_QUIET')
cmd += [str(HERE/'meter.c'),'-Wl,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-bt-meter.so')]
subprocess.run(cmd,check=True)
subprocess.run([sys.executable,str(HERE.parent/'settings-debug/build.py')],check=True)
sys.path.insert(0,str(HERE.parent/'settings-debug'))
from elf_bridge import bridge
(out/'librt-ui-loader.so.1').write_bytes(bridge((BASE/'lib/librt.so.1').read_bytes(),'/usr/data/disc-settings/module-ui.so'))
abi=subprocess.run(['readelf','-h','-A','-W','-l',str(out/'libdisc-bt-meter.so')],capture_output=True,text=True,check=True).stdout
(out/'abi.txt').write_text(abi)
assert 'nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi and 'RWE' not in abi
report={'version':'1.3.181','menu_bridge_version':'1.10.181','library_bytes':(out/'libdisc-bt-meter.so').stat().st_size,'library_sha256':hashlib.sha256((out/'libdisc-bt-meter.so').read_bytes()).hexdigest(),'loader_sha256':hashlib.sha256((out/'librt-ui-loader.so.1').read_bytes()).hexdigest(),'constructor':False,'new_daemon':False,'ui_calls_on_ui_thread':True,'executable_stack':False}
(out/'build.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))

