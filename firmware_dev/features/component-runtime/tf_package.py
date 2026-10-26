"""Export reviewed .disc.zip packages as TF directory packages; no APK conversion."""
from pathlib import Path
import argparse,hashlib,tempfile
import manager

def export(archive,output,storage='nand',startup=False):
    if storage not in ('nand','tf'):raise ValueError('storage must be nand or tf')
    # Reuse host installer's complete zip/ABI/version/file validation.
    with tempfile.TemporaryDirectory() as tmp:
        stage=Path(tmp)
        manifest=manager.unpack(archive,stage)
        data={name:(stage/name).read_bytes() for name in manifest['files']}
    if manifest['id'] in ('usb-settings','disc-runtime'):raise ValueError('reserved system component')
    if manifest['kind'] not in ('tool','service'):raise ValueError('UI plugins need display integration first')
    if startup and manifest['kind']!='service':raise ValueError('only services can autostart')
    if 257 not in manifest['base']:raise ValueError('requires V2.57 compatibility')
    dest=output/'DISC_MODULES'/manifest['id']/manifest['version']
    if dest.exists():raise ValueError('destination already exists')
    metadata={'MODULE_ID':manifest['id'],'MODULE_VERSION':manifest['version'],
              'ENTRY':manifest['entry'],'KIND':manifest['kind'],'ABI':manifest['abi'],
              'BASE':'257','STORAGE':storage,'STARTUP':str(int(startup))}
    if set(data)&(set(metadata)|{'SHA256SUMS'}):raise ValueError('reserved TF metadata filename')
    dest.mkdir(parents=True)
    for name,b in data.items():
        p=dest/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b)
        p.chmod(manifest['files'][name]['mode'])
    for name,value in metadata.items():(dest/name).write_text(value+'\n',encoding='ascii',newline='\n')
    rows=[hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.relative_to(dest).as_posix()
          for p in sorted(dest.rglob('*')) if p.is_file()]
    if len(rows)>512 or sum(p.stat().st_size for p in dest.rglob('*') if p.is_file())>64*1024*1024:
        raise ValueError('TF package exceeds loader limits')
    (dest/'SHA256SUMS').write_text('\n'.join(rows)+'\n',encoding='ascii',newline='\n')
    return dest

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('archive',type=Path);p.add_argument('output',type=Path)
    p.add_argument('--storage',choices=['nand','tf'],default='nand');p.add_argument('--startup',action='store_true')
    a=p.parse_args();print(export(a.archive,a.output,a.storage,a.startup))
