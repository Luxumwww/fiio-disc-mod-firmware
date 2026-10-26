"""Build DSP1.16.0 only; lock production inputs before/after compilation."""
from pathlib import Path
import hashlib,json,subprocess,sys
H=Path(__file__).resolve().parent;D=H.parent/'audio-dsp';ROOT=H.parents[2]
OUT=H/'reports/perf-208';LOCK=OUT/'dsp-build-lock.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 paths={p for ext in ('*.c','*.h','*.cpp','*.py') for p in D.glob(ext) if p.name!='provider_guard.h'}
 paths.update(p for p in (D/'third_party/dragonfly-port').rglob('*') if p.suffix in ('.h','.cpp'))
 paths.add(Path(__file__).resolve())
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)}
def verify():
 x=json.loads(LOCK.read_text());assert x['inputs']==inputs(),'DSP source changed since locked build'
 for name,digest in x['outputs'].items():assert sha(ROOT/name)==digest,'DSP output changed: '+name
 return x
if __name__=='__main__':
 if '--check' in sys.argv:print(json.dumps(verify(),indent=2));sys.exit()
 before=inputs();tool=subprocess.check_output(['clang','--version'],text=True)
 subprocess.run([sys.executable,str(D/'build.py')],check=True)
 assert before==inputs(),'DSP inputs changed during compilation'
 outputs={p.relative_to(ROOT).as_posix():sha(p) for p in (D/'build/libdisc-audio-dsp.so',D/'provider_guard.h',D/'build/build.json')}
 OUT.mkdir(exist_ok=True,parents=True);LOCK.write_text(json.dumps(dict(version='1.16.0',inputs=before,outputs=outputs,compiler=tool,threads_added=False,audio_rate_changed=False),indent=2)+'\n')
 verify();print('DSP1.16.0 input lock PASS count='+str(len(before)))
