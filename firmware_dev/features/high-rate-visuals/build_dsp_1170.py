"""Lock DSP117 plus its independent SoXR bridge sources and build outputs."""
from pathlib import Path
import hashlib,json,subprocess,sys
H=Path(__file__).resolve().parent;D=H.parent/'audio-dsp';S=H.parent/'dsp-resample';ROOT=H.parents[2]
OUT=S/'reports/1.17.0';LOCK=OUT/'dsp-build-lock.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 paths={p for ext in ('*.c','*.h','*.cpp','*.py') for p in D.glob(ext) if p.name!='provider_guard.h'}
 paths.update(p for p in (D/'third_party/dragonfly-port').rglob('*') if p.suffix in ('.h','.cpp'))
 paths.update(p for p in S.rglob('*') if p.is_file() and not any(n in ('build','reports','__pycache__') for n in p.relative_to(S).parts) and p.suffix in ('.c','.h','.py','.json'))
 paths.add(S/'build/soxr/soxr-config.h');paths.add(Path(__file__).resolve())
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)}
def verify():
 x=json.loads(LOCK.read_text());assert x['inputs']==inputs(),'DSP117 inputs changed'
 for n,v in x['outputs'].items():assert sha(ROOT/n)==v,'DSP117 output changed: '+n
 return x
if __name__=='__main__':
 if '--check' in sys.argv:print('DSP117 lock PASS inputs='+str(len(verify()['inputs'])));sys.exit()
 subprocess.run([sys.executable,str(S/'build_soxr.py'),'--mips-only','--objects-only'],check=True)
 before=inputs();subprocess.run([sys.executable,str(D/'build.py')],check=True);assert before==inputs()
 outputs={p.relative_to(ROOT).as_posix():sha(p) for p in (D/'build/libdisc-audio-dsp.so',D/'provider_guard.h',D/'build/build.json')}
 OUT.mkdir(exist_ok=True,parents=True);LOCK.write_text(json.dumps(dict(version='1.17.0',inputs=before,outputs=outputs,source_rate_unchanged=True,no_new_thread=True,original_libgcc_sha=sha(Path('/root/disc-official-component-base-20261004-r1/base/lib/libgcc_s.so.1'))),indent=2)+'\n');verify();print('DSP117 build/source lock PASS inputs='+str(len(before)))
