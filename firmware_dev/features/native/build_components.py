"""Build inactive native components and check stock glibc loading in MIPS QEMU.
Does not change overlay, startup, firmware binaries or device state.
"""
import hashlib,json,shutil,subprocess,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent
BASE=HERE.parent
STOCK=Path('/root/disc-firmware-dev-v257/stock')
OUT=HERE/'build/components';OUT.mkdir(parents=True,exist_ok=True)
expected=json.loads((BASE/'reports/feature-route-evidence.json').read_text())['stock_hashes_verified']
assert hashlib.sha256((STOCK/'usr/bin/mq_player').read_bytes()).hexdigest()==expected['usr/bin/mq_player']
flags=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-fno-builtin','-fuse-ld=lld','-nostdlib','-shared','-fPIC']
core=OUT/'libdisc-spectrum-core.so';tap=OUT/'libdisc-pcm-tap.so'
subprocess.run(flags+[str(HERE/'spectrum_core.c'),'-Wl,--no-undefined,-soname,libdisc-spectrum-core.so','-o',str(core)],check=True)
subprocess.run(flags+[str(HERE/'pcm_tap.c'),'-L'+str(STOCK/'lib'),'-Wl,-l:libdl.so.2','-Wl,-l:libc.so.6','-Wl,--no-undefined,-soname,libdisc-pcm-tap.so','-o',str(tap)],check=True)
artifacts={}
for p in (core,tap):
 abi=subprocess.run(['readelf','-h','-A','-d',str(p)],capture_output=True,text=True,check=True).stdout
 assert 'ELF32' in abi and 'little endian' in abi and 'MIPS32r2' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi and 'nan2008' in abi
 (HERE/'reports'/(p.name+'.abi.txt')).write_text(abi)
 artifacts[p.name]={'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size,'abi':'MIPS32r2/o32/FP64/NaN2008'}
work=Path(tempfile.mkdtemp(prefix='disc-component-load-'))
try:
 for p in (core,tap):shutil.copy2(p,work/p.name)
 probe=work/'load-probe.so'
 subprocess.run(flags+[str(HERE/'component_load_probe.c'),str(HERE/'selftest.c'),'-L'+str(work),'-L'+str(STOCK/'lib'),'-Wl,-l:libdisc-spectrum-core.so','-Wl,-l:libdl.so.2','-Wl,-l:libc.so.6','-Wl,--no-undefined','-o',str(probe)],check=True)
 command=['qemu-mipsel','-strace','-cpu','P5600','-L',str(STOCK),'-E','LD_LIBRARY_PATH='+str(work)+':'+str(STOCK/'lib')+':'+str(STOCK/'usr/lib'),'-E','LD_PRELOAD='+str(work/tap.name)+':'+str(probe),str(STOCK/'usr/bin/ffmpeg'),'-version']
 run=subprocess.run(command,capture_output=True,text=True,timeout=20)
 (HERE/'reports/component-load-stderr.txt').write_text(run.stderr)
 assert run.returncode==0 and 'DISC_NATIVE_CORE_OK' in run.stdout,(run.returncode,run.stdout,run.stderr)
 assert 'socket(PF_UNIX,SOCK_DGRAM' in run.stderr
 assert not any(token in run.stderr for token in ['/dev/snd/','/dev/usb_dac','/proc/asound/cards'])
 source_hashes={name:hashlib.sha256((HERE/name).read_bytes()).hexdigest() for name in ['spectrum_core.c','spectrum_core.h','spectrum_tables.h','pcm_tap.c','pcm_packet.h']}
 report={'artifacts':artifacts,'source_sha256':source_hashes,'stock_player_sha256':expected['usr/bin/mq_player'],'stock_glibc_loader_probe':{'exit':0,'output':run.stdout.strip(),'datagram_socket':True},'player_binaries_changed':False,'startup_changed':False,'overlay_changed':False,'device_access':False,'stock_ui_runtime_verified':False,'real_usb_capture_verified':False,'flashable_feature_image_built':False,'activation':'none: these components are development artifacts only'}
 (HERE/'reports/components-verification.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps(report,indent=2))
finally:
 shutil.rmtree(work)
