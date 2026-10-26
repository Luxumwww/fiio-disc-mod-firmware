"""Offline logger-only V427 derivative. No connected device or flash access."""
import hashlib, json, os, shutil, subprocess, sys
from pathlib import Path

HERE=Path(__file__).resolve().parent
DISC=HERE.parents[2]
sys.path.insert(0,str(DISC/'firmware_dev'))
import workbench as wb
SOURCE=DISC/'1we/FIIO-DISC-V427-Stable-OneClick-R5/FIIO-DISC-V427-Stable-OneClick-R5/firmware/FIIO-V427-Stable-rootfs.squashfs'
EXPECTED='48e0ca9fde71efce215d9104026dcfbbb9ebd44ae328fc21ff4f8a240892a462'
WORK=Path('/root/disc-diagnostics-v427-r2')
OUT=DISC/'firmware_dev/builds/v427-tf-log-test-20261004-r2'

def main():
    if wb.digest(SOURCE)!=EXPECTED:raise RuntimeError('V427 base SHA256 mismatch')
    header=wb.header(SOURCE)
    if header['compression_id']!=3 or header['block_size']!=131072:raise RuntimeError('Unexpected compression')
    if OUT.exists() or WORK.exists():raise RuntimeError('Preserve previous output; this build uses fresh directories only')
    OUT.mkdir(parents=True);WORK.mkdir(parents=True)
    base=WORK/'base';root=WORK/'rootfs';verified=WORK/'verified'
    wb.run(['unsquashfs','-no-progress','-processors','2','-d',base,SOURCE],OUT/'extract.log')
    before=wb.inventory(base)
    wb.run(['cp','-a',str(base)+'/.',root])
    for path,item in before.items():
        if item['type']=='symlink':os.utime(root/path,(item['mtime'],item['mtime']),follow_symlinks=False)
    if wb.inventory(root)!=before:raise RuntimeError('Clone metadata differs before editing')
    launch=root/'usr/project/fiio_init.sh';meta=launch.stat();data=launch.read_bytes()
    if not data.startswith(b'#!/bin/sh\n'):raise RuntimeError('Expected original shell startup')
    if b'disc-spectrum' in data or b'libdisc-spectrum' in data:raise RuntimeError('Frequency experiment present in base')
    entry=b'\n# Independent opt-in TF diagnostic logger; stock programs remain unchanged.\n/usr/project/disc-tf-log.sh >/dev/null 2>&1 &\n'
    launch.write_bytes(data.replace(b'#!/bin/sh\n',b'#!/bin/sh\n'+entry,1));os.utime(launch,(meta.st_atime,meta.st_mtime))
    parent=root/'usr/project';parent_meta=parent.stat()
    logger=parent/'disc-tf-log.sh';shutil.copyfile(HERE/'disc-tf-log.sh',logger);logger.chmod(0o755);os.chown(logger,0,0)
    os.utime(logger,(header['mkfs_time'],header['mkfs_time']));os.utime(parent,(parent_meta.st_atime,parent_meta.st_mtime))
    after=wb.inventory(root);changes=wb.delta(before,after)
    wb.save(OUT/'changes-before-pack.json',changes)
    if {item['path'] for item in changes}!={'usr/project/fiio_init.sh','usr/project/disc-tf-log.sh'}:raise RuntimeError('Unexpected file/attribute changes')
    for program in ['mq_ui','mq_player']:
        if wb.digest(base/'usr/bin'/program)!=wb.digest(root/'usr/bin'/program):raise RuntimeError('Stock binary changed')
    image=OUT/'rootfs.squashfs'
    wb.run(['mksquashfs',root,image,'-noappend','-comp','lzo','-b','131072','-processors','2','-no-progress','-mkfs-time',str(header['mkfs_time'])],OUT/'pack.log')
    if image.stat().st_size>wb.LIMIT:raise RuntimeError('Too large for observed logical capacity')
    wb.run(['unsquashfs','-no-progress','-processors','2','-d',verified,image],OUT/'reextract.log')
    differences=wb.delta(after,wb.inventory(verified));wb.save(OUT/'roundtrip-mismatches.json',differences)
    if differences:raise RuntimeError('Roundtrip mismatch')
    wb.save(OUT/'changes.json',changes)
    result=dict(build=OUT.name,base='V427 Stable from local R5 package',base_sha256=EXPECTED,
        image_sha256=wb.digest(image),image_bytes=image.stat().st_size,roundtrip_verified=True,
        changed_paths=[item['path'] for item in changes],frequency_extension_present=False,
        native_programs_unchanged=True,device_verified=False,hardware_stability='unverified',
        contains_only_independent_tf_logger=True,boot_marker='DISC_LOG_ENABLE',
        original_kernel_not_changed=True,tool_and_device_used=False)
    wb.save(OUT/'result.json',result)
    job=dict(schema=1,name='V427 TF 启动日志诊断实验版（没有频谱注入，真机未验证）',status='ready',
        base='V427 Stable 同一社区基线，仅 rootfs；需要 TF 标记才持久采集',
        image='../../firmware_dev/builds/'+OUT.name+'/rootfs.squashfs',sha256=result['image_sha256'])
    wb.save(DISC/'刷机工具/jobs/tf-log-test.json',job)
    print(json.dumps(result,ensure_ascii=False,indent=2))

if __name__=='__main__':main()
