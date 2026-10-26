"""Official V2.57 + additive debug/component boot entry; run in WSL."""
from pathlib import Path
import hashlib,json,os,shutil,sys
here=Path(__file__).resolve().parent;disc=here.parents[2]
sys.path.insert(0,str(disc/'firmware_dev'));import workbench as wb
source=disc/'firmware_analysis/main_os_ota_v257/rootfs.squashfs'
expected='111e4dd7ee3d7ff91ba7e61181690be7ffd22bd1bbd13ad513f5f015ffb302ae'
assert wb.digest(source)==expected
work=Path('/root/disc-official-component-base-20261004-r1');base=work/'base';root=work/'rootfs';verified=work/'verified'
out=disc/'firmware_dev/builds/v257-official-component-base-20261004-r1'
assert base.exists() and not (out/'rootfs.squashfs').exists() and not verified.exists()
assert wb.digest(base/'usr/bin/adbd')==json.loads((here/'daemon-build-evidence.json').read_text())['stock_sha256']
out.mkdir(parents=True,exist_ok=True);head=wb.header(source);before=wb.inventory(base)
if not root.exists():
 wb.run(['cp','-a',str(base)+'/.',root])
 for name,meta in before.items():
  if meta['type']=='symlink':os.utime(root/name,(meta['mtime'],meta['mtime']),follow_symlinks=False)
 assert before==wb.inventory(root)
seed=root/'usr/project/disc-runtime-v1';shutil.copytree(here/'service',seed,dirs_exist_ok=True)
sums=[]
for p in sorted(seed.rglob('*')):
 if p.is_file() and p.name!='SHA256SUMS':
  p.chmod(0o755 if p.name=='adbd' or p.name=='atomic-rename' or p.suffix=='.sh' else 0o644)
  sums.append(wb.digest(p)+'  '+p.relative_to(seed).as_posix())
(seed/'SHA256SUMS').write_text('\n'.join(sums)+'\n')
boot=root/'usr/bin/disc-runtime-start.sh';shutil.copyfile(here/'boot-start.sh',boot);boot.chmod(0o755)
init=root/'usr/project/fiio_init.sh';original=(base/'usr/project/fiio_init.sh').read_bytes()
assert original.startswith(b'#!/bin/sh\n') and b'disc-runtime' not in original
entry=b'\n# Additive debug/component service; original UI/audio startup below is retained.\n/usr/bin/disc-runtime-start.sh >/tmp/disc-runtime-boot.log 2>&1 &\n'
init.write_bytes(original.replace(b'#!/bin/sh\n',b'#!/bin/sh\n'+entry,1))
for p in [boot,seed,*seed.rglob('*')]:
 os.chown(p,0,0);os.utime(p,(head['mkfs_time'],head['mkfs_time']))
for name,meta in before.items():
 if meta['type']=='directory' or name=='usr/project/fiio_init.sh':os.utime(root/name,(meta['mtime'],meta['mtime']))
after=wb.inventory(root);changes=wb.delta(before,after)
assert all(c['path'] in ('usr/project/fiio_init.sh','usr/bin/disc-runtime-start.sh') or c['path'].startswith('usr/project/disc-runtime-v1') for c in changes)
for f in ['usr/bin/adbd','usr/bin/mq_ui','usr/bin/mq_player','lib/libc.so.6','lib/ld-linux-mipsn8.so.1','etc/product_version/version.in']:
 assert wb.digest(base/f)==wb.digest(root/f)
qemu=['qemu-mipsel','-cpu','P5600','-L',str(base),str(base/'bin/busybox')]
for p in [boot,init,*seed.glob('*.sh')]:wb.run(qemu+['ash','-n',p],out/(p.name+'-syntax.log'))
image=out/'rootfs.squashfs'
wb.run(['mksquashfs',root,image,'-noappend','-comp','lzo','-b','131072','-processors','2','-no-progress','-mkfs-time',str(head['mkfs_time'])],out/'pack.log')
assert image.stat().st_size<=wb.LIMIT
wb.run(['unsquashfs','-no-progress','-processors','2','-d',verified,image],out/'reextract.log')
diff=wb.delta(after,wb.inventory(verified));wb.save(out/'roundtrip-mismatches.json',diff);assert not diff
wb.save(out/'changes.json',changes)
r={'name':out.name,'official_source_sha256':expected,'official_version':257,'image_sha256':wb.digest(image),'image_bytes':image.stat().st_size,'roundtrip_verified':True,'original_ui_player_adbd_libraries_unchanged':True,'original_init_retained_with_one_background_entry':True,'kernel_and_other_partitions_not_included':True,'default_usb':True,'default_wireless':False,'runtime_works_on_current_community_v427':True,'official_image_device_flashed':False,'official_boot_hardware_verified':False}
wb.save(out/'result.json',r)
wb.save(disc/'刷机工具/jobs/official-component-base.json',{'schema':1,'name':'官方V2.57 + ADB/无线调试/组件入口 R1（官方整包尚未真机验收）','status':'ready','base':'纯官方V2.57 rootfs；增加调试启动入口和组件服务；只刷主rootfs，保留原UI/播放器/库；无线默认关闭','image':'../../firmware_dev/builds/'+out.name+'/rootfs.squashfs','sha256':r['image_sha256']})
print(json.dumps(r,ensure_ascii=False,indent=2))
