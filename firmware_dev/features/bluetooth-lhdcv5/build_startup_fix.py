from pathlib import Path
import subprocess,json,hashlib
P=Path(__file__).resolve().parent;out=P/'reports/startup-fix-114';out.mkdir(exist_ok=True)
source=(P/'build_display_fix.py').read_text().replace('display-fix-112','startup-fix-114')
exec(compile(source,str(P/'build_display_fix.py')+' [114]','exec'))
subprocess.run(['python3',str(P.parent/'settings-debug/build.py')],check=True)
for name in ['entry.sh','bluealsa.sh']:subprocess.run(['sh','-n',str(P/'module-source'/name)],check=True)
files=[P.parent/'bluetooth-meter/build/libdisc-bt-meter.so',P.parent/'settings-debug/build/disc-settings.so']
(out/'build.json').write_text(json.dumps({str(f):{'sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'bytes':f.stat().st_size} for f in files},indent=2))
