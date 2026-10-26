from pathlib import Path
import subprocess,json,hashlib
P=Path(__file__).resolve().parent;out=P/'reports/nominal-fix-113';out.mkdir(exist_ok=True)
subprocess.run(['clang','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',str(P/'test_nominal.c'),'-o',str(P/'build/test-nominal')],check=True)
results=[]
for rate in (48000,96000,192000):
 results.append(subprocess.check_output([str(P/'build/test-nominal'),str(P/f'testdata/capture-{rate}.packets')],text=True));print(results[-1],end='')
(out/'tests.txt').write_text(''.join(results))
source=(P/'build_display_fix.py').read_text().replace('display-fix-112','nominal-fix-113')
exec(compile(source,str(P/'build_display_fix.py')+' [113]','exec'))
