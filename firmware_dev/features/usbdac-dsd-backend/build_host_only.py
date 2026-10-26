from pathlib import Path
import ast, subprocess, json, hashlib
HERE=Path(__file__).resolve().parent.parent/'bluetooth-meter'
BASE=Path('/root/disc-official-component-base-20261004-r1/base');out=HERE/'build'
tree=ast.parse((HERE/'build.py').read_text())
expr=next(x.value for x in tree.body if isinstance(x,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='cmd' for t in x.targets))
cmd=eval(compile(ast.Expression(expr),str(HERE/'build.py'),'eval'))
subprocess.run(cmd,check=True)
expr=next(x.value for x in tree.body if isinstance(x,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='report' for t in x.targets))
report=eval(compile(ast.Expression(expr),str(HERE/'build.py'),'eval'))
(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
