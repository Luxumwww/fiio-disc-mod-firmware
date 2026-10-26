"""Produce existing-manager NAND and TF packages, including r16 bootstrap support."""
from pathlib import Path
import hashlib,json,shutil
import package,tf_package
HERE=Path(__file__).resolve().parent
VERSION='1.1.1';ID='disc-adb-tcpip'
src=HERE/'tcpip-module-source';(src/'framework').mkdir(exist_ok=True)
for name in ('runtime.sh','runtime-modules.sh','module-state.sh'):
 shutil.copyfile(HERE/'service'/name,src/'framework'/name)
assert (src/'libdisc-adb-tcpip.so').is_file(), 'build_tcpip.py must run first'
(src/'INTEGRATION').write_text('runtime\n',encoding='ascii',newline='\n')
(src/'VERSION').write_text(VERSION+'\n',encoding='ascii',newline='\n')
archive=HERE/'packages'/f'{ID}-{VERSION}.disc.zip'
if archive.exists():raise RuntimeError('release exists; increase version instead of overwriting')
manifest=package.build(src,archive,ID,VERSION,'entry.sh','service',bases=(257,))
tf=tf_package.export(archive,HERE/'packages'/f'{ID}-{VERSION}-TF',storage='nand',startup=True)
report={'id':ID,'version':VERSION,'kind':'service','integration':'runtime','archive':str(archive),'archive_sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'tf_directory':str(tf),'payload_bytes':sum(p.stat().st_size for p in src.rglob('*') if p.is_file()),'files':manifest['files']}
(HERE/'tcpip-module-build.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:v for k,v in report.items() if k!='files'},ensure_ascii=False,indent=2))
