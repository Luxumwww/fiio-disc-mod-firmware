from pathlib import Path
import subprocess,hashlib,json
here=Path(__file__).resolve().parent
headers=here.parents[1]/'reviews/deepseek-probe-r6-20261004/suggested-fix/native/src'
out=here/'service/runtime-links'
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32',
 '-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror',
 '-Wno-unused-function','-fno-builtin','-G0','-mno-abicalls','-fno-pic','-fuse-ld=lld',
 '-nostdlib','-static','-I'+str(headers),str(here/'runtime-links.c'),
 '-Wl,-e,_start,-z,noexecstack','-o',str(out)],check=True)
out.chmod(0o755)
print(json.dumps(dict(bytes=out.stat().st_size,sha256=hashlib.sha256(out.read_bytes()).hexdigest())))
