from pathlib import Path
import hashlib,json,struct,subprocess
p=Path(__file__).resolve().parent;base=Path('/root/disc-official-component-base-20261004-r1/base')
u=(base/'usr/bin/mq_ui').read_bytes();assert hashlib.sha256(u).hexdigest()=='f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226'
h=struct.unpack_from('<16sHHIIIIIHHHHHH',u);segments=[struct.unpack_from('<IIIIIIII',u,h[5]+i*h[9]) for i in range(h[10])]
lines=['static const struct {unsigned address;unsigned char bytes[16];} provider_guards[]={']
for a in (0x51a7e0,0x565dcc,0x51ab20,0x51ab0c):
 s=next(s for s in segments if s[0]==1 and s[2]<=a<a+16<=s[2]+s[4]);o=s[1]+a-s[2];lines.append('{0x%x,{%s}},'%(a,','.join(map(str,u[o:o+16]))))
lines.append('};');(p/'provider_guard.h').write_text('\n'.join(lines)+'\n')
out=p/'build';out.mkdir(exist_ok=True)
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-fPIC','-shared','-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),str(p/'provider.c'),'-Wl,-l:libc.so.6,--no-undefined,-z,noexecstack','-o',str(out/'libdisc-hwaccel.so')],check=True)
abi=subprocess.run(['readelf','-h','-A','-W','-l',str(out/'libdisc-hwaccel.so')],capture_output=True,text=True,check=True).stdout
assert 'nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi and 'RWE' not in abi
(out/'abi.txt').write_text(abi)
report={'version':'1.0.7','sha256':hashlib.sha256((out/'libdisc-hwaccel.so').read_bytes()).hexdigest(),'bytes':(out/'libdisc-hwaccel.so').stat().st_size,'capabilities':['DPU'],'extra_daemon':False,'new_kernel_driver':False}
(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
