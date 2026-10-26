from pathlib import Path
import subprocess,json
P=Path(__file__).resolve().parent;O=P/'reports/0.3.0';S=P/'server';go='/root/disc-go-1.27.2/go/bin/go'
r=subprocess.run([go,'fmt','./...'],cwd=S,check=True,capture_output=True,text=True)
r=subprocess.run([go,'test','-count=1','-v','./...'],cwd=S,capture_output=True,text=True);(O/'go-tests.log').write_text(r.stdout+r.stderr);print(r.stdout+r.stderr,flush=True);r.check_returncode()
import os,hashlib
env=dict(os.environ,CGO_ENABLED='0',GOOS='linux',GOARCH='mipsle',GOMIPS='softfloat',GOTOOLCHAIN='local',GOPROXY='off')
subprocess.run([go,'build','-trimpath','-ldflags=-s -w','-o',str(S/'disc-remote-control'),'.'],cwd=S,env=env,check=True)
b=(S/'disc-remote-control').read_bytes();meta=dict(version='0.3.0',bytes=len(b),sha256=hashlib.sha256(b).hexdigest());(O/'server-build.json').write_text(json.dumps(meta,indent=2));print(meta)
