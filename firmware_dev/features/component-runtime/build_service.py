from pathlib import Path
import hashlib,json,struct,sys,subprocess,os
here=Path(__file__).resolve().parent;sys.path.insert(0,str(here.parents[1]))
import workbench as wb
stock=here/'adbd-stock.bin';original=stock.read_bytes();sha=hashlib.sha256(original).hexdigest()
assert len(original)==117668
assert sha=='a895779fe77ef8c76752bee0e7b733ffd783019582bbfb6e762861093aace6d5'
out=here/'service';out.mkdir(exist_ok=True)
# Pin full input plus exact compiled instructions, never heuristic runtime patches.
patches={0x403e04:(bytes.fromhex('d4a9228e'),struct.pack('<I',0x0c10268e)),
         0x403e08:(bytes.fromhex('01004230'),struct.pack('<I',0x240415b3)),
         0x403e0c:(bytes.fromhex('25004014'),struct.pack('<I',0x10000001))}
binary=bytearray(original);audit=[]
for address,(before,after) in patches.items():
 offset=address-0x400000;assert binary[offset:offset+4]==before
 binary[offset:offset+4]=after;audit.append({'address':hex(address),'file_offset':offset,'before':before.hex(),'after':after.hex()})
for mode,data in [('usb',original),('both',bytes(binary)),('wireless',original)]:
 p=out/'profiles'/mode/'adbd';p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data);p.chmod(0o755)
# wireless profile is stock: after USB cleanup, endpoint absence triggers its TCP path.
header=here.parents[1]/'reviews/deepseek-probe-r6-20261004/suggested-fix/native/src/syscalls.h'
flags=['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-fno-builtin','-fuse-ld=lld','-nostdlib','-shared','-fPIC','-I'+str(header.parent)]
root=Path('/root/disc-adb-test-20261004-r1/base')
lib=out/'libdisc-adb-policy.so'
wb.run(flags+[here/'net_policy.c','-L'+str(root/'lib'),'-Wl,-l:libc.so.6','-Wl,--no-undefined,-soname,libdisc-adb-policy.so','-o',lib],here/'net-policy-build.log')
wb.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-fno-builtin','-mno-abicalls','-fno-pic','-fuse-ld=lld','-nostdlib','-static','-I'+str(header.parent),here/'rename.c','-Wl,-e,_start','-o',out/'atomic-rename'],here/'rename-build.log')
result={'stock_sha256':sha,'both_patch':audit,'stock_adbd_modified_on_device':False,
 'patch_behavior':'USB init followed by local_init(5555); pre-JDWP debug trace branch bypassed; JDWP/event loop retained',
 'wireless_profile':'unchanged stock adbd; requires absence of USB endpoint nodes',
 'files':{p.relative_to(out).as_posix():{'bytes':p.stat().st_size,'sha256':wb.digest(p)} for p in out.rglob('*') if p.is_file()}}
wb.save(here/'daemon-build-evidence.json',result)
print(json.dumps(result,indent=2))
