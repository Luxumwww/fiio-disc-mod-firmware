"""Build and run the same fixed-point native core on host and MIPS QEMU."""
import hashlib,json,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
OUT=HERE/'build';OUT.mkdir(exist_ok=True)
subprocess.run(['python3',str(HERE/'generate_spectrum_tables.py')],check=True)
host=OUT/'spectrum-host-selftest'
mips=OUT/'spectrum-mips-selftest'
subprocess.run(['gcc','-O2','-Wall','-Wextra','-Werror','-DDISC_HOST_TEST',str(HERE/'spectrum_core.c'),str(HERE/'selftest.c'),'-o',str(host)],check=True)
flags=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-msoft-float','-mnan=2008','-G0','-O2','-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-pic','-mno-abicalls','-nostdlib','-static','-fuse-ld=lld','-Wl,-e,_start']
subprocess.run(flags+[str(HERE/'spectrum_core.c'),str(HERE/'selftest.c'),str(HERE/'start_selftest.S'),'-o',str(mips)],check=True)
checks={}
for name,command in [('host',[str(host)]),('mips',['qemu-mipsel','-cpu','P5600',str(mips)])]:
 result=subprocess.run(command,capture_output=True,text=True,timeout=20)
 assert result.returncode==0 and 'DISC_NATIVE_CORE_OK' in result.stdout,(name,result.returncode,result.stdout,result.stderr)
 checks[name]={'exit':result.returncode,'output':result.stdout.strip()}
abi=subprocess.run(['readelf','-A',str(mips)],check=True,capture_output=True,text=True).stdout
assert 'Soft float' in abi
(OUT/'core-abi.txt').write_text(abi)
report={'checks':checks,'mips_sha256':hashlib.sha256(mips.read_bytes()).hexdigest(),'mips_float_abi':'soft','device_access':False,'firmware_ui_integrated':False}
(HERE/'reports/core-verification.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
