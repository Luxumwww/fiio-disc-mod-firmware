from pathlib import Path
import subprocess,hashlib,json
HERE=Path(__file__).resolve().parent
headers=HERE.parents[1]/'reviews/deepseek-probe-r6-20261004/suggested-fix/native/src'
out=HERE/'service/ui-mode'
subprocess.run(['clang','--target=mipsel-linux-gnu','-march=mips32r2','-mabi=32','-mhard-float','-mfp64','-mnan=2008','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-fno-builtin','-mno-abicalls','-fno-pic','-fuse-ld=lld','-nostdlib','-static','-I'+str(headers),str(HERE/'ui-mode.c'),'-Wl,-e,_start,-z,noexecstack','-o',str(out)],check=True)
print(json.dumps({'bytes':out.stat().st_size,'sha256':hashlib.sha256(out.read_bytes()).hexdigest()}))
