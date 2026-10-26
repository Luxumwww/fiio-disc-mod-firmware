"""Build official V2.57 with immutable debug/menu/TF-loader seed. WSL only."""
from pathlib import Path
import hashlib,json,os,shutil,sys
here=Path(__file__).resolve().parent;disc=here.parents[2]
sys.path.insert(0,str(disc/'firmware_dev'));import workbench as wb
base=Path('/root/disc-official-component-base-20261004-r1/base')
work=Path('/root/disc-system-base-20261005-r16');root=work/'rootfs';verified=work/'verified'
out=disc/'firmware_dev/builds/v257-system-base-20261005-r16'
assert not verified.exists() and not (out/'rootfs.squashfs').exists()
assert wb.digest(wb.SOURCE)==wb.SHA
out.mkdir(parents=True,exist_ok=True);work.mkdir(parents=True,exist_ok=True)
head=wb.header(wb.SOURCE);before=wb.inventory(base)
if not root.exists():wb.run(['cp','-a',str(base)+'/.',root])
for name,meta in before.items():
 if meta['type']=='symlink':os.utime(root/name,(meta['mtime'],meta['mtime']),follow_symlinks=False)
assert before==wb.inventory(root)
seed=root/'usr/project/disc-system-v1';seed.mkdir()
shutil.copytree(here/'service',seed/'service')
addon=here.parent/'settings-debug'
for name,source in [('disc-settings.so',addon/'build/disc-settings.so'),('librt.so.1',addon/'build/librt.so.1'),('guard.sh',addon/'guard.sh'),('uninstall.sh',addon/'uninstall.sh')]:shutil.copyfile(source,seed/name)
(seed/'VERSION').write_text('1.7.1\n')
def sums(folder):
 rows=[]
 for p in sorted(folder.rglob('*')):
  if p.is_file() and p.name!='SHA256SUMS':rows.append(wb.digest(p)+'  '+p.relative_to(folder).as_posix())
 (folder/'SHA256SUMS').write_text('\n'.join(rows)+'\n')
for p in seed.rglob('*'):
 if p.is_file():p.chmod(0o755 if p.suffix=='.sh' or p.name in ('adbd','atomic-rename','ui-mode','wpa_passphrase') else 0o644)
sums(seed/'service')
# Include the nested manifest itself in the outer seed manifest.
sums(seed)
with (seed/'SHA256SUMS').open('a') as f:f.write(wb.digest(seed/'service/SHA256SUMS')+'  service/SHA256SUMS\n')
prepare=root/'usr/bin/disc-system-prepare.sh';shutil.copyfile(here/'system-prepare.sh',prepare);prepare.chmod(0o755)
boot=root/'usr/bin/disc-runtime-start.sh'
boot.write_text('''#!/bin/sh
unset LD_PRELOAD
sleep 20
root=/usr/data/disc-runtime
seed=/usr/project/disc-system-v1/service
if (cd "$root/service" && sha256sum -c SHA256SUMS >/tmp/disc-runtime-integrity.log 2>&1);then
 exec "$root/service/runtime.sh"
fi
# Independent immutable rescue when editable service cannot start.
DISC_ADBD_BIN=/usr/bin/adbd DISC_ADB_POLICY= exec "$seed/usb-manager.sh"
''');boot.chmod(0o755)
init=root/'usr/project/fiio_init.sh';original=(base/'usr/project/fiio_init.sh').read_bytes()
assert original.startswith(b'#!/bin/sh\n') and b'disc-runtime' not in original
entry=b'\n# Restore immutable system extensions before stock UI loads its libraries.\n/usr/bin/disc-system-prepare.sh >/tmp/disc-system-prepare.log 2>&1\n/usr/bin/disc-runtime-start.sh >/tmp/disc-runtime-boot.log 2>&1 &\n'
init.write_bytes(original.replace(b'#!/bin/sh\n',b'#!/bin/sh\n'+entry,1))
for p in [seed,*seed.rglob('*'),prepare,boot]:os.chown(p,0,0);os.utime(p,(head['mkfs_time'],head['mkfs_time']))
for name,meta in before.items():
 if meta['type']=='directory' or name=='usr/project/fiio_init.sh':os.utime(root/name,(meta['mtime'],meta['mtime']))
after=wb.inventory(root);changes=wb.delta(before,after)
assert all(c['path'] in ('usr/project/fiio_init.sh','usr/bin/disc-system-prepare.sh','usr/bin/disc-runtime-start.sh') or c['path'].startswith('usr/project/disc-system-v1') for c in changes)
for f in ['usr/bin/mq_ui','usr/bin/mq_player','usr/bin/adbd','lib/libc.so.6','lib/ld-linux-mipsn8.so.1','etc/product_version/version.in']:assert wb.digest(base/f)==wb.digest(root/f)
qemu=['qemu-mipsel','-cpu','P5600','-L',str(base),str(base/'bin/busybox')]
for p in [prepare,boot,init,*seed.rglob('*.sh')]:wb.run(qemu+['ash','-n',p])
image=out/'rootfs.squashfs'
wb.run(['mksquashfs',root,image,'-noappend','-comp','lzo','-b','131072','-processors','2','-no-progress','-mkfs-time',str(head['mkfs_time'])],out/'pack.log')
assert image.stat().st_size<=wb.LIMIT
wb.run(['unsquashfs','-no-progress','-processors','2','-d',verified,image],out/'reextract.log')
diff=wb.delta(after,wb.inventory(verified));wb.save(out/'roundtrip-mismatches.json',diff);assert not diff
wb.save(out/'changes.json',changes)
result={'name':out.name,'sha256':wb.digest(image),'bytes':image.stat().st_size,'official_source_sha256':wb.SHA,'roundtrip_verified':True,'original_ui_player_unchanged':True,'default_usb':False,'default_wireless':False,'menu_version':'1.7.1','factory_reset_hardware_verified':False,'hardware_flashed':False,'rootfs_only':True,'personal_backup_format':2,'saved_wifi_ui_enabled':True,'saved_wifi_limit':20,'backup_confirm_and_delete':True,'restore_stationary_hold_ms':1000}
wb.save(out/'result.json',result)
wb.save(disc/'刷机工具/jobs/system-base-r2.json',{'schema':1,'name':'官方 V2.57 系统内置调试菜单与 TF 模块加载 R2（待真机刷入验收）','status':'ready','base':'恢复出厂后重建菜单、模式调试管理与TF加载器；开关首次默认关闭；只刷主rootfs','image':'../../firmware_dev/builds/'+out.name+'/rootfs.squashfs','sha256':result['sha256']})
print(json.dumps(result,ensure_ascii=False,indent=2))
