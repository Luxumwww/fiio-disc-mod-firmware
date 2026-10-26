from pathlib import Path
import hashlib,json,struct,subprocess,sys
from elf_bridge import BASE,bridge,ui_map,import_map
HERE=Path(__file__).resolve().parent
MENU_BRIDGE_VERSION='1.10.220'
module=HERE.parent/'bluetooth-meter/build/libdisc-bt-meter.so'
if module.exists():
 data=module.read_bytes();module_fnv=2166136261
 for b in data:module_fnv=((module_fnv^b)*16777619)&0xffffffff
 (HERE/'bt_module_guard.h').write_text(f'#define BT_MODULE_SIZE {len(data)}u\n#define BT_MODULE_FNV {module_fnv}u\n')
else:(HERE/'bt_module_guard.h').write_text('#define BT_MODULE_SIZE 0u\n#define BT_MODULE_FNV 0u\n')
provider=HERE.parent/'hardware-accel/runtime-provider/build/libdisc-hwaccel.so'
data=provider.read_bytes();h=2166136261
for b in data:h=((h^b)*16777619)&0xffffffff
(HERE/'hw_provider_guard.h').write_text(f'#define HW_PROVIDER_SIZE {len(data)}u\n#define HW_PROVIDER_FNV {h}u\n')
xy=HERE.parent/'audio-visuals/xy/build/libdisc-audio-xy.so'
data=xy.read_bytes();h=2166136261
for b in data:h=((h^b)*16777619)&0xffffffff
(HERE/'xy_provider_guard.h').write_text(f'#define XY_PROVIDER_SIZE {len(data)}u\n#define XY_PROVIDER_FNV {h}u\n')
polar=HERE.parent/'audio-visuals/polar/build/libdisc-audio-polar.so'
data=polar.read_bytes();h=2166136261
for b in data:h=((h^b)*16777619)&0xffffffff
(HERE/'polar_provider_guard.h').write_text(f'#define POLAR_PROVIDER_SIZE {len(data)}u\n#define POLAR_PROVIDER_FNV {h}u\n')
spectrum=HERE.parent/'audio-visuals/spectrum/build/libdisc-audio-spectrum.so'
data=spectrum.read_bytes();h=2166136261
for b in data:h=((h^b)*16777619)&0xffffffff
(HERE/'spectrum_provider_guard.h').write_text(f'#define SPECTRUM_PROVIDER_SIZE {len(data)}u\n#define SPECTRUM_PROVIDER_FNV {h}u\n')
mandala=HERE.parent/'audio-visuals/mandala/build/libdisc-audio-mandala.so'
data=mandala.read_bytes();h=2166136261
for b in data:h=((h^b)*16777619)&0xffffffff
(HERE/'mandala_provider_guard.h').write_text(f'#define MANDALA_PROVIDER_SIZE {len(data)}u\n#define MANDALA_PROVIDER_FNV {h}u\n')
ui=(BASE/'usr/bin/mq_ui').read_bytes();rt=(BASE/'lib/librt.so.1').read_bytes()
assert hashlib.sha256(ui).hexdigest()=='f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226'
assert hashlib.sha256(rt).hexdigest()=='4192eaea2420074471cfb2917121892fae607f44fc69c3629bfe6b71040dfb1e'
addresses=[0x403970,0x485ef8,0x457e6c,0x4f8850,0x4f889c,0x4f8304,0x4f6b60,
0x4e9a70,0x4e969c,0x52df64,0x52f2bc,0x4eb680,0x4ebc14,0x4e96d0,0x4e97f8,
0x4f45d0,0x4f6df0,0x4f6b50,0x4f6e60,0x4f6c40,0x4e39ec,0x4e3934,0x4e3924,
0x4f6af0,0x4f6b00,0x4f6b10,0x4f6b20,0x46d88c,0x4e2734,0x4f7e98,0x486620,
0x4e42e8,0x4e45bc,0x51a7e0,0x4e26b4,0x52a8a4,0x488860,0x52f4f0,0x4f0268,0x4f02c8,0x5030a0,0x5030e4,0x470128,0x48b28c,0x481bec,0x4fa364,0x525320,0x4f5ea4,0x4f6e10,0x4f3f10,0x4f6f10,0x7c6bd0,0x7ca120,0x46ea1c,0x4e394c]
fnv=2166136261
addresses.append(0x46ec0c)
addresses += [0x4777e0,0x46f290,0x46fdac,0x474838,0x46b7a0,0x459f20,0x459f14,0x45a940]
addresses += [0x533960,0x4eb4a4,0x4f6c20,0x459470,0x4e9c04,0x4e9e0c,0x4e98f8,0x4e9918]
addresses += [0x483170,0x4775a8,0x48831c,0x4f15ac,0x52cb10,0x52cb44,0x4f6e50,0x51b3e0,0x51b7b0,0x4eb600]
addresses += [0x52607c]
addresses += [0x479090,0x4790b0]
addresses += [0x4ece1c]
addresses += [0x47fdac]
addresses += [0x46d5c0,0x4f6b30,0x4618a8,0x4617e0]
addresses += [0x471108,0x473054,0x46d848,0x4f273c]
addresses += [0x53342c,0x52a754,0x52a4ec,0x52a7c8,0x52a7f0,0x4f6e80,0x4f6cb0]
addresses += [0x503c28,0x503c5c,0x5253fc,0x46b570,0x7c2050,0x7ffd00]
for b in ui:fnv=((fnv^b)*16777619)&0xffffffff
h=struct.unpack_from('<16sHHIIIIIHHHHHH',ui)
ph=[struct.unpack_from('<IIIIIIII',ui,h[5]+i*h[9]) for i in range(h[10])]
mapping=ui_map(ui)
lines=[f'#define UI_MQ_RECEIVE_GOT {import_map(ui,"mq_receive")["got"]}u',f'#define UI_FNV {fnv}u',f'#define UI_SIZE {len(ui)}u',f'#define USLEEP_GOT {mapping["usleep_got"]}u','static const struct {unsigned address;unsigned char bytes[16];} guards[]={']
player=(BASE/'usr/bin/mq_player').read_bytes()
assert hashlib.sha256(player).hexdigest()=='a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc'
player_header=struct.unpack_from('<16sHHIIIIIHHHHHH',player)
player_segments=[struct.unpack_from('<IIIIIIII',player,player_header[5]+i*player_header[9]) for i in range(player_header[10])]
scope_lines=[f'#define LS_OPENDIR_GOT {import_map(player,"opendir")["got"]}u','static const struct {unsigned address;unsigned char bytes[16];} ls_scanner_guards[]={']
scope_got=int(import_map(player,"opendir")["got"],16);segment=next(s for s in player_segments if s[0]==1 and s[2]<=scope_got<scope_got+4<=s[2]+s[4]);scope_original=struct.unpack_from('<I',player,segment[1]+scope_got-segment[2])[0]
scope_lines.insert(1,f'#define LS_OPENDIR_UNRESOLVED {scope_original}u')
for address in (0x46830c,0x468408,0x827bc0):
 segment=next(s for s in player_segments if s[0]==1 and s[2]<=address<address+16<=s[2]+s[4]);offset=segment[1]+address-segment[2]
 scope_lines.append('{0x%x,{%s}},'%(address,','.join(map(str,player[offset:offset+16]))))
scope_lines.append('};');(HERE.parent/'library-scope/scanner_guard.h').write_text('\n'.join(scope_lines)+'\n')
cover_lines=[f'#define COVER_PIC_GET_GOT {import_map(player,"AudioCodecGetPicBytesNew")["got"]}u',f'#define COVER_MALLOC_GOT {import_map(player,"malloc")["got"]}u',f'#define COVER_PTHREAD_CREATE_GOT {import_map(player,"pthread_create")["got"]}u','static const struct {unsigned address;unsigned char bytes[16];} cover_guards[]={']
for address in (0x44ea10,0x4ce8c0,0x462440,0x44f5ec,0x47e928,0x480bb8,0x636084):
 segment=next(s for s in player_segments if s[0]==1 and s[2]<=address<address+16<=s[2]+s[4]);offset=segment[1]+address-segment[2]
 cover_lines.append('{0x%x,{%s}},'%(address,','.join(map(str,player[offset:offset+16]))))
cover_lines.append('};');(HERE.parent/'embedded-cover/cover_guard.h').write_text('\n'.join(cover_lines)+'\n')
pu_lines=[f'#define PU_CODEC_{key}_GOT {import_map(player,name)["got"]}u' for key,name in (('OPEN','AudioCodecOpen'),('READ','AudioCodecRead'),('CLOSE','AudioCodecClose'))]
pu_lines.append('static const struct {unsigned address;unsigned char bytes[16];} pu_player_guards[]={')
for address in (0x44b0f0,0x44b2ec,0x44badc,0x44bfd4,0x44cc30,0x44ccd0):
 segment=next(s for s in player_segments if s[0]==1 and s[2]<=address<address+16<=s[2]+s[4]);offset=segment[1]+address-segment[2]
 pu_lines.append('{0x%x,{%s}},'%(address,','.join(map(str,player[offset:offset+16]))))
pu_lines.append('};');(HERE.parent/'player-utilities/player_guard.h').write_text('\n'.join(pu_lines)+'\n')
pc_lines=[f'#define PLAYER_PCM_READ_GOT {import_map(player,"pcm_read")["got"]}u',f'#define PLAYER_PCM_WRITE_GOT {import_map(player,"pcm_write")["got"]}u',f'#define PLAYER_USLEEP_GOT {ui_map(player)["usleep_got"]}u',f'#define PLAYER_IOCTL_GOT {import_map(player,"ioctl")["got"]}u',f'#define PLAYER_DBUS_POP_GOT {import_map(player,"dbus_connection_pop_message")["got"]}u','static const struct {unsigned address;unsigned char bytes[16];} pc_guards[]={']
for address in (0x455c58,0x47bf24,0x47c2e8,0x44c9cc,0x4536d0,0x4f1294,0x4e5b90,0x4ed0e0,0x4ea5d4,0x44dc80,0x4e83b8,0x4e8888,0x4e8b94,0x4eae58,0x4e84f0,0x43db60,0x40fb30,0x40b330,0x4ed84c,0x4eb284,0x4e9534,0x4e8858,0x4e7df0,0x8264d0,0x478354,0x47859c,0x45c648,0x4658c8,0x465970,0x465af8,0x465d64,0x466420,0x4e94f4,0x465ec0,0x466338,0x466980,0x457ec0,0x466f1c,0x5e9eb8,0x5e9ed0,0x453674,0x5e523c,0x5e8f2c,0x5e14fc,0x5f25b0,0x5f261c,0x5e1554,0x5e1a4c,0x5e1a5c,0x5e3d7c,0x5e3f2c):
 segment=next(s for s in player_segments if s[0]==1 and s[2]<=address<address+16<=s[2]+s[4]);offset=segment[1]+address-segment[2]
 pc_lines.append('{0x%x,{%s}},'%(address,','.join(map(str,player[offset:offset+16]))))
pc_lines.append('};');(HERE.parent/'bluetooth-meter/player_controls_guard.h').write_text('\n'.join(pc_lines)+'\n')
player_fnv=2166136261
for b in player:player_fnv=((player_fnv^b)*16777619)&0xffffffff
lines[:0]=[f'#define PLAYER_SIZE {len(player)}u',f'#define PLAYER_FNV {player_fnv}u']
for a in addresses:
 p=next(p for p in ph if p[0]==1 and p[2]<=a<a+16<=p[2]+p[4]);off=p[1]+a-p[2]
 lines.append('{0x%x,{%s}},'%(a,','.join(str(b) for b in ui[off:off+16])))
lines.append('};');(HERE/'guard.h').write_text('\n'.join(lines)+'\n')
if '--guards-only' in sys.argv:
 print('SHA-locked native headers refreshed; no ELF compiled')
 sys.exit(0)
out=HERE/'build';out.mkdir(exist_ok=True);(out/'librt.so.1').write_bytes(bridge(rt))
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(BASE/'lib'),'-Wl,-l:libpthread.so.0,-l:libdl.so.2,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack',str(HERE/'settings.c'),'-o',str(out/'disc-settings.so')],check=True)
report={n:{'sha256':hashlib.sha256((out/n).read_bytes()).hexdigest(),'bytes':(out/n).stat().st_size} for n in ['disc-settings.so','librt.so.1']}
(HERE/'build-report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
(out/'host-build.json').write_text(json.dumps({'version':MENU_BRIDGE_VERSION,'artifacts':report},indent=2)+'\n')
(HERE/'elf-map.json').write_text(json.dumps(mapping,indent=2)+'\n')
