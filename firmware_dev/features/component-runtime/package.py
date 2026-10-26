"""Make a documented .disc.zip from owned component files."""
from pathlib import Path
import argparse, hashlib, json, zipfile
from manager import valid_name,safe_path

def build(source,out,id,version,entry,kind='tool',bases=(257,427)):
    valid_name(id);valid_name(version);safe_path(entry)
    files={};data={}
    for p in sorted(source.rglob('*')):
        if p.is_symlink():raise ValueError('symlinks are not supported')
        if not p.is_file():continue
        name=p.relative_to(source).as_posix();safe_path(name)
        if name in ('ENTRY','KIND','SHA256SUMS','manifest.json'):raise ValueError('reserved name')
        b=p.read_bytes();data[name]=b;files[name]={'sha256':hashlib.sha256(b).hexdigest(),'mode':0o755 if name==entry or name.endswith('.sh') else 0o644}
    if entry not in files:raise ValueError('entry not found')
    m={'schema':1,'id':id,'version':version,'kind':kind,'base':list(bases),'abi':'mips32r2-o32-fp64-nan2008','entry':entry,'files':files}
    out.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
        z.writestr('manifest.json',json.dumps(m,indent=2))
        for name,b in data.items():z.writestr(name,b)
    return m

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('out',type=Path);p.add_argument('--id',required=True);p.add_argument('--version',required=True);p.add_argument('--entry',required=True);p.add_argument('--kind',default='tool',choices=['tool','service','ui']);a=p.parse_args();print(json.dumps(build(a.source,a.out,a.id,a.version,a.entry,a.kind),indent=2))
