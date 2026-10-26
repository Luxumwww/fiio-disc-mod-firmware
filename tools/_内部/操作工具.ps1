param(
 [ValidateSet('Driver','Backup','Flash','Restore','Check','Verify')][string]$Mode='Check',
 [ValidateSet('Partial','Full','')][string]$Scope='',
 [string]$BackupPath='', [string]$SessionPath='',
 [switch]$PrepareOnly, [switch]$DriverElevated
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version 2.0
$internal=[IO.Path]::GetFullPath($PSScriptRoot)
$package=[IO.Directory]::GetParent($internal).FullName
$backupRoot=Join-Path $package '备份'
$utf8=New-Object Text.UTF8Encoding($false)
$manifestSha='676e89178a9ed87b90ff1a3244d61873a8196676588dc03c0e977fd3f582c991'
$layout=@(
 @{name='uboot';offset=0;size=0x200000},@{name='kernel';offset=0x200000;size=0x800000},
 @{name='rootfs';offset=0xa00000;size=0x8000000},@{name='kernel2';offset=0x8a00000;size=0x800000},
 @{name='rootfs2';offset=0x9200000;size=0x1900000},@{name='ota';offset=0xab00000;size=0x100000},
 @{name='mac';offset=0xac00000;size=0x100000},@{name='userdata';offset=0xad00000;size=0x5300000})
$pair=@('rootfs','userdata')
$fullOrder=@('kernel','rootfs','kernel2','rootfs2','ota','mac','userdata','uboot')
$mutex=$null;$locked=$false;$scratch=New-Object 'Collections.Generic.List[string]'
Add-Type -AssemblyName System.IO.Compression.FileSystem
function Digest([string]$Path){
 $s=[IO.File]::OpenRead($Path);$h=[Security.Cryptography.SHA256]::Create()
 try{return ([BitConverter]::ToString($h.ComputeHash($s))).Replace('-','').ToLowerInvariant()}finally{$h.Dispose();$s.Dispose()}
}
function TextFile([string]$Path,[string]$Text){[IO.File]::WriteAllText($Path,$Text,$utf8)}
function JsonFile([string]$Path,$Object){TextFile $Path ($Object|ConvertTo-Json -Depth 12)}
function ReadJson([string]$Path){return ([IO.File]::ReadAllText($Path,$utf8)|ConvertFrom-Json)}
function Has($Object,[string]$Name){return $null -ne $Object.PSObject.Properties[$Name]}
function SafePath([string]$Base,[string]$Relative){
 if([IO.Path]::IsPathRooted($Relative)){throw '文件路径不正确。'}
 $p=[IO.Path]::GetFullPath((Join-Path $Base $Relative))
 if(!$p.StartsWith($Base.TrimEnd('\')+'\',[StringComparison]::OrdinalIgnoreCase)){throw '文件路径超出文件夹。'}
 return $p
}
function AssertFile($Item,[string]$Path){
 if(!(Test-Path -LiteralPath $Path -PathType Leaf)){throw "缺少文件：$($Item.path)"}
 if((Get-Item -LiteralPath $Path).Length -ne [long]$Item.bytes -or (Digest $Path) -ne $Item.sha256){throw "文件校验失败：$($Item.path)"}
}
function AssertNoCloner{
 if(Get-Process -Name cloner,core -ErrorAction SilentlyContinue){throw '请先停止并关闭已经打开的烧录工具，再运行此操作。不会强行关闭它。'}
}
function NewWorkspace([string]$Tag){
 $roots=@([IO.Path]::GetTempPath(),[Environment]::GetFolderPath('CommonDocuments'))
 foreach($root in $roots){
  if(!$root){continue};$root=[IO.Path]::GetFullPath($root)
  if($root -notmatch '^[A-Za-z]:[\\/][A-Za-z0-9 _\.\-\\/]+$'){continue}
  try{
   $dir=Join-Path $root ('DISI/sessions/'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'-'+$Tag+'-'+[Guid]::NewGuid().ToString('N').Substring(0,8))
   [IO.Directory]::CreateDirectory($dir)|Out-Null
   TextFile (Join-Path $dir 'write-test.txt') 'ok'
   return $dir
  }catch{}
 }
 throw '没有可用的临时运行目录。请检查电脑临时文件夹的权限和空间。'
}
function SelectScope([string]$Title,[bool]$AllowResume=$false,[bool]$ForRestore=$false){
 Write-Host "`n$Title" -ForegroundColor Cyan
 if($ForRestore){
  Write-Host '  1. 只恢复系统和功能分区'
  Write-Host '  2. 恢复全部八分区（含设备信息，只能恢复到原机器）'
 }else{
  Write-Host '  1. 系统和功能分区'
  Write-Host '  2. 全量八分区'
 }
 if($AllowResume){Write-Host '  3. 校验并保存上次读取的备份'}
 Write-Host '  0. 退出'
 while($true){
  switch(Read-Host '输入数字'){
   '1'{return 'Partial'} '2'{return 'Full'} '0'{return 'Cancel'}
   '3'{if($AllowResume){return 'Resume'}}
  }
  Write-Host '请输入菜单中的数字。'
 }
}
function ScopeTitle([string]$Value){if($Value -eq 'Full'){return '全量八分区'};return '系统和功能分区'}
function AssertBoot([string]$Base){
 $b=[IO.File]::ReadAllBytes((Join-Path $Base 'uboot.bin'))
 if($b.Length -lt 0x5988 -or [Text.Encoding]::ASCII.GetString($b,0x5800,4) -ne 'nand' -or [BitConverter]::ToUInt32($b,0x5804) -ne 8){throw '启动备份的分区表不符合本机布局，不能全量恢复。'}
 for($i=0;$i -lt 8;$i++){
  $at=0x5808+48*$i;$p=$layout[$i];$stored=0;if($p.name -eq 'userdata'){$stored=1}
  $name=[Text.Encoding]::ASCII.GetString($b,$at,32).Split([char]0)[0]
  if($name -ne $p.name -or [BitConverter]::ToUInt32($b,$at+32) -ne $p.size -or [BitConverter]::ToUInt32($b,$at+36) -ne $p.offset -or [BitConverter]::ToUInt32($b,$at+40) -ne 0 -or [BitConverter]::ToUInt32($b,$at+44) -ne $stored){throw "启动备份的分区参数不匹配：$($p.name)"}
 }
}
function AssertHeaders([string]$Base,[bool]$Full){
 $h=New-Object byte[] 96;$s=[IO.File]::OpenRead((Join-Path $Base 'rootfs.bin'))
 try{$n=$s.Read($h,0,96);$length=$s.Length}finally{$s.Dispose()}
 if($n -ne 96 -or [Text.Encoding]::ASCII.GetString($h,0,4) -ne 'hsqs' -or [BitConverter]::ToUInt16($h,28) -ne 4 -or [BitConverter]::ToUInt64($h,40) -lt 96 -or [BitConverter]::ToUInt64($h,40) -gt $length){throw '主系统备份格式不正确，不能用于恢复。'}
 $s=[IO.File]::OpenRead((Join-Path $Base 'userdata.bin'));$valid=$false
 try{for($i=0;$i -lt 8 -and ($i*131072+4) -le $s.Length;$i++){
  $s.Position=$i*131072;$n=$s.Read($h,0,4)
  if($n -eq 4 -and [Text.Encoding]::ASCII.GetString($h,0,4) -eq 'UBI#'){$valid=$true;break}
 }}finally{$s.Dispose()}
 if(!$valid){throw '功能分区备份格式不正确，不能用于恢复。'}
 if($Full){AssertBoot $Base}
}
function ExpandBackup([string]$Zip){
 $dir=NewWorkspace 'Unpack';$scratch.Add($dir)
 $archive=[IO.Compression.ZipFile]::OpenRead($Zip)
 try{
  $seen=@{};$total=0L
  foreach($entry in $archive.Entries){
   $n=$entry.FullName
   if($n -notmatch '^(backup\.json|uboot\.bin|kernel\.bin|rootfs\.bin|kernel2\.bin|rootfs2\.bin|ota\.bin|mac\.bin|userdata\.bin)$' -or $seen.ContainsKey($n)){throw '这不是本工具的备份ZIP，请选择备份文件夹中的备份包。'}
   $seen[$n]=$true;$limit=262144L
   if($n -ne 'backup.json'){$limit=(@($layout|Where-Object {($_.name+'.bin') -eq $n}))[0].size}
   $total+=$entry.Length
   if($entry.Length -le 0 -or $entry.Length -gt $limit -or $total -gt 268697600L){throw '备份包的文件大小不正确。'}
   $zipStream=$entry.Open();$output=[IO.File]::Create((Join-Path $dir $n));$buffer=New-Object byte[] 1048576;$written=0L
   try{while(($got=$zipStream.Read($buffer,0,$buffer.Length)) -gt 0){$written+=$got;if($written -gt $entry.Length){throw '备份解压长度异常。'};$output.Write($buffer,0,$got)}}finally{$output.Dispose();$zipStream.Dispose()}
   if($written -ne $entry.Length){throw '备份包未解压完整。'}
  }
  if(!$seen.ContainsKey('backup.json')){throw '备份包缺少校验记录。'}
 }finally{$archive.Dispose()}
 return $dir
}
function OpenBackup([string]$Path){
 $path=[IO.Path]::GetFullPath($Path.Trim().Trim('"'))
 if(!(Test-Path -LiteralPath $path)){throw '找不到选中的备份。'}
 $base=$path
 if(Test-Path -LiteralPath $path -PathType Leaf){
  if([IO.Path]::GetExtension($path) -ieq '.zip' -or $path -like '*.zip.*.partial'){$base=ExpandBackup $path}
  else{$base=[IO.Path]::GetDirectoryName($path)}
 }
 if(Test-Path -LiteralPath (Join-Path $base 'backup.json')){
  $record=ReadJson (Join-Path $base 'backup.json')
  if(!(Has $record 'schema') -or $record.schema -notin @(1,2,3) -or !(Has $record 'two_reads_match') -or $record.two_reads_match -isnot [bool] -or !$record.two_reads_match -or !(Has $record 'raw_with_oob') -or $record.raw_with_oob -ne $false){throw '备份记录没有通过双遍读取校验，不能恢复。'}
 }elseif(Test-Path -LiteralPath (Join-Path $base '备份清单.json')){
  $old=ReadJson (Join-Path $base '备份清单.json')
  if($old.raw_with_oob -ne $false -or $old.partitions.Count -ne 8){throw '旧备份记录格式不符。'}
  $rows=@();foreach($p in $layout){
   $matches=@($old.partitions|Where-Object {$_.partition -eq $p.name})
   if($matches.Count -ne 1){throw '旧备份记录缺少分区。'};$r=$matches[0]
   if($r.two_reads_match -ne $true -or $r.offset -ne $p.offset -or $r.nominal_bytes -ne $p.size){throw '旧备份的地址或双读记录不符。'}
   $rows+=@{partition=$p.name;path=$p.name+'.bin';bytes=$r.dump_bytes;sha256=$r.sha256}
  }
  $record=[pscustomobject]@{schema=3;scope='full-eight';raw_with_oob=$false;two_reads_match=$true;parts=$rows}
 }else{throw '选中的文件夹没有备份记录。请选择备份ZIP或backup.json。'}
 $names=@($record.parts|ForEach-Object {$_.partition})
 if($names.Count -notin @(2,8) -or @($names|Select-Object -Unique).Count -ne $names.Count -or @($names|Where-Object {$_ -notin @($layout|ForEach-Object {$_.name})}).Count -ne 0 -or 'rootfs' -notin $names -or 'userdata' -notin $names){throw '备份的分区列表不正确。'}
 return @{directory=$base;record=$record;full=($names.Count -eq 8);source=$path}
}
function ValidateBackup($Own,[string]$Selected){
 $names=$pair;if($Selected -eq 'Full'){
  if(!$Own.full){throw '这份备份只有系统和功能分区，不能全量恢复。'}
  $names=@($layout|ForEach-Object {$_.name})
 }
 foreach($name in $names){
  $p=@($layout|Where-Object {$_.name -eq $name})[0];$matches=@($Own.record.parts|Where-Object {$_.partition -eq $name})
  if($matches.Count -ne 1){throw "备份缺少分区：$name"};$r=$matches[0]
  if($r.path -cne ($name+'.bin') -or [long]$r.bytes -le 0 -or [long]$r.bytes -gt $p.size -or ([long]$r.bytes%131072) -ne 0 -or $r.sha256 -notmatch '^[a-fA-F0-9]{64}$'){throw "分区备份记录不正确：$name"}
  AssertFile $r (SafePath $Own.directory $r.path)
 }
 AssertHeaders $Own.directory ($Selected -eq 'Full')
}
function ChooseBackup{
 $items=@()
 if(Test-Path -LiteralPath $backupRoot){$items+=@(Get-ChildItem -LiteralPath $backupRoot -File -Filter '*.zip'|Sort-Object LastWriteTime -Descending)}
 if(Test-Path -LiteralPath $backupRoot){$items+=@(Get-ChildItem -LiteralPath $backupRoot -Directory|Where-Object {(Test-Path -LiteralPath (Join-Path $_.FullName 'backup.json')) -or (Test-Path -LiteralPath (Join-Path $_.FullName '备份清单.json'))})}
 Write-Host "`n选择要恢复的备份" -ForegroundColor Cyan
 for($i=0;$i -lt $items.Count;$i++){Write-Host ("  {0}. {1}" -f ($i+1),$items[$i].Name)}
 Write-Host '  B. 浏览其他位置的备份ZIP或备份记录'
 Write-Host '  0. 退出'
 while($true){
  $answer=Read-Host '输入数字或B'
  if($answer -eq '0'){return ''}
  if($answer -ieq 'B'){
   Add-Type -AssemblyName System.Windows.Forms
   $dialog=New-Object Windows.Forms.OpenFileDialog
   $dialog.Title='选择自己的备份ZIP，或旧备份中的backup.json / 备份清单.json'
   $dialog.Filter='备份包或备份记录 (*.zip;*.json)|*.zip;*.json'
   try{if($dialog.ShowDialog() -eq [Windows.Forms.DialogResult]::OK){return $dialog.FileName}}finally{$dialog.Dispose()}
   continue
  }
  $number=0;if([int]::TryParse($answer,[ref]$number) -and $number -ge 1 -and $number -le $items.Count){return $items[$number-1].FullName}
  Write-Host '请选择列表中的序号，或输入B浏览。'
 }
}
function ArchiveSession([string]$Dir){
 AssertNoCloner
 $dir=[IO.Path]::GetFullPath($Dir);$rec=ReadJson (Join-Path $dir 'session.json')
 if($rec.mode -ne 'Backup' -or !(Has $rec 'scope') -or $rec.scope -notin @('Partial','Full')){throw '这不是本工具的备份读取记录。'}
 $parts=@($layout|Where-Object {$rec.scope -eq 'Full' -or $_.name -in $pair});$expected=$parts.Count*2
 if($rec.policies.Count -ne $expected){throw '备份读取任务不完整。'}
 Write-Host "`n正在核对两遍读取，请等待……" -ForegroundColor Cyan
 $rows=@();foreach($p in $parts){
  $first=Join-Path $dir ('read1/'+$p.name+'.bin');$second=Join-Path $dir ('read2/'+$p.name+'.bin')
  if(!(Test-Path -LiteralPath $first) -or !(Test-Path -LiteralPath $second)){throw '还没有读完，请先完成烧录工具中的全部读取任务。'}
  $a=(Get-Item -LiteralPath $first).Length;$b=(Get-Item -LiteralPath $second).Length
  if($a -le 0 -or $a -ne $b -or $a -gt $p.size -or ($a%131072) -ne 0){throw "读取长度不正确：$($p.name)"}
  $sha=Digest $first;if($sha -ne (Digest $second)){throw "两次读取不一致：$($p.name)。不要刷写，请重新备份。"}
  $rows+=@{partition=$p.name;path=$p.name+'.bin';bytes=$a;sha256=$sha}
 }
 AssertHeaders (Join-Path $dir 'read1') ($rec.scope -eq 'Full')
 [IO.Directory]::CreateDirectory($backupRoot)|Out-Null
 $label='系统和功能分区';$scopeRecord='rootfs-userdata';if($rec.scope -eq 'Full'){$label='全量八分区';$scopeRecord='full-eight'}
 $name=[IO.Path]::GetFileName($dir)+'-'+$label+'.zip';$dest=Join-Path $backupRoot $name
 if(Test-Path -LiteralPath $dest){
  $existing=OpenBackup $dest;ValidateBackup $existing $rec.scope
  foreach($r in $rows){$match=@($existing.record.parts|Where-Object {$_.partition -eq $r.partition});if($match.Count -ne 1 -or $match[0].sha256 -ne $r.sha256){throw '已有同名备份，内容不同，未覆盖。'}}
 }else{
  $stage=NewWorkspace 'Archive';$scratch.Add($stage)
  foreach($r in $rows){$to=Join-Path $stage $r.path;[IO.File]::Copy((Join-Path $dir ('read1/'+$r.path)),$to,$false);AssertFile $r $to}
  # NewWorkspace's write probe is not part of a backup package.
  [IO.File]::Delete((Join-Path $stage 'write-test.txt'))
  JsonFile (Join-Path $stage 'backup.json') @{schema=3;scope=$scopeRecord;created_at=(Get-Date -Format o);two_reads_match=$true;raw_with_oob=$false;parts=$rows;restore_tested=$false;contains_personal_data=$true}
  $pending=Join-Path $backupRoot ($name+'.'+[Guid]::NewGuid().ToString('N')+'.partial')
  try{
   [IO.Compression.ZipFile]::CreateFromDirectory($stage,$pending,[IO.Compression.CompressionLevel]::Optimal,$false)
   $check=OpenBackup $pending;ValidateBackup $check $rec.scope
   [IO.File]::Move($pending,$dest)
  }finally{if(Test-Path -LiteralPath $pending){[IO.File]::Delete($pending)}}
 }
 TextFile (Join-Path $backupRoot '最近备份.txt') $name
 Write-Host "备份成功：$dest" -ForegroundColor Green
 Write-Host '请妥善保存备份。'
 return $dest
}
function PrepareCloner([string]$Session,[string]$Action,[string]$Selected,$Own){
 $tool=Join-Path $Session 'Cloner'
 foreach($item in $script:manifest.files){
  if(!$item.path.StartsWith('_内部/Cloner/')){continue}
  $relative=$item.path.Substring('_内部/'.Length);$to=Join-Path $Session $relative
  [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($to))|Out-Null
  [IO.File]::Copy((SafePath $package $item.path),$to,$false);AssertFile $item $to
 }
 $targets=@();$policies=@();$board='DISI-'+$Action.ToLowerInvariant()+'-'+$Selected.ToLowerInvariant()+'.cfg'
 if($Action -eq 'Backup'){
  $parts=@($layout|Where-Object {$Selected -eq 'Full' -or $_.name -in $pair})
  foreach($round in @(1,2)){
   [IO.Directory]::CreateDirectory((Join-Path $Session "read$round"))|Out-Null
   foreach($p in $parts){$label=$p.name;if($label -eq 'mac'){$label='p6'}
    $policies+=@{label=$label+"_r$round";offset=$p.offset;type=11;attribute=(Join-Path $Session ("read$round/"+$p.name+'.bin'))}
   }
  }
 }else{
  $targets=$pair;if($Action -eq 'Restore' -and $Selected -eq 'Full'){$targets=$fullOrder}
  [IO.Directory]::CreateDirectory((Join-Path $Session 'images'))|Out-Null
  foreach($name in $targets){
   $p=@($layout|Where-Object {$_.name -eq $name})[0]
   if($Action -eq 'Restore'){$row=@($Own.record.parts|Where-Object {$_.partition -eq $name})[0];$source=Join-Path $Own.directory $row.path}
   else{$relative='_内部/images/'+$(if($name -eq 'rootfs'){'rootfs.squashfs'}else{'userdata.ubifs'});$row=$script:files[$relative];$source=SafePath $package $relative}
   $to=Join-Path $Session ('images/'+$name+'.bin');[IO.File]::Copy($source,$to,$false);AssertFile $row $to
   $label=$name;if($name -eq 'mac'){$label='p6'};if($name -eq 'userdata' -and $Action -eq 'Flash'){$label='user_data'}
   $policies+=@{label=$label;offset=$p.offset;type=0;attribute=$to}
  }
 }
 $text=[IO.File]::ReadAllText((Join-Path $internal 'hardware.cfg'),$utf8).TrimEnd()+"`r`n`r`n[spiPartition]`r`n"
 for($i=0;$i -lt 8;$i++){
  $p=$layout[$i];$label=$p.name;if($label -eq 'mac'){$label='p6'};$flag=2;$manager='MTD_MODE'
  if($targets -contains $p.name){$flag=0}
  if($Action -eq 'Flash' -and $p.name -eq 'userdata'){$label='user_data';$manager='UBI_MANAGER'}
  $text+='Partition'+$i+'="'+$label+','+('0x{0:x}' -f $p.offset)+','+('0x{0:x}' -f $p.size)+','+$flag+','+$manager+'"'+"`r`n"
 }
 $text+="count=8`r`n`r`n[policy]`r`npolicy_count=$($policies.Count)`r`n"
 for($i=0;$i -lt $policies.Count;$i++){
  $p=$policies[$i];$text+="`r`n[policy$i]`r`nattribute=$($p.attribute.Replace('\','/'))`r`nenabled=1`r`nlabel=$($p.label)`r`noffset=$('0x{0:x}' -f $p.offset)`r`nops=`"13,6,1`"`r`ntype=$($p.type)`r`n"
 }
 foreach($guard in @('force_erase=0','download_params=0','security_enable=0','security_burnkey=0','force_reset=0','read_back_chk=1','transfer_data_chk=1')){if($text -notmatch ('(?m)^'+[regex]::Escape($guard)+'\s*$')){throw '内部配置校验失败，已停止。'}}
 if($Action -eq 'Backup' -and ($policies.Count -notin @(4,16) -or $text -match '(?m)^type=0\s*$' -or $text -match ',0,(MTD_MODE|UBI_MANAGER)')){throw '备份配置必须是仅读取。'}
 if($Action -eq 'Flash' -and (($targets -join ',') -ne 'rootfs,userdata' -or $policies.Count -ne 2)){throw '改版固件刷写范围错误。'}
 if($Action -eq 'Restore' -and (($Selected -eq 'Full' -and ($targets -join ',') -ne ($fullOrder -join ',')) -or ($Selected -eq 'Partial' -and ($targets -join ',') -ne ($pair -join ',')))){throw '恢复范围错误。'}
 TextFile (Join-Path $tool ('configs/x2000/'+$board)) $text
 $platform=[IO.File]::ReadAllText((Join-Path $tool 'configs/platforms.cfg'),$utf8)
 TextFile (Join-Path $tool 'configs/platforms.cfg') ([regex]::Replace($platform,'(?m)^board=.*$','board='+$board))
 $rec=@{schema=3;mode=$Action;scope=$Selected;board=$board;session=$Session;policies=$policies;automatic_start=$false;cloner_launched=$false;flash_success=$false;hardware_restore_tested=$false}
 JsonFile (Join-Path $Session 'session.json') $rec
 return $rec
}
function LaunchCloner([string]$Session,$Record){
 AssertNoCloner
 $tool=Join-Path $Session 'Cloner'
 $p=Start-Process -FilePath (Join-Path $tool 'cloner.exe') -WorkingDirectory $tool -PassThru
 $Record.cloner_launched=$true;$Record['pid']=$p.Id;JsonFile (Join-Path $Session 'session.json') $Record
 Write-Host "`n已打开烧录工具。配置和路径已填好，无需修改。" -ForegroundColor Green
 Write-Host '核对后点“开始”，按住音量减插入USB；boot超过50%后松开。'
 Write-Host '没有进度时，在点开始后按上述方法重连。全部任务成功后，停止并关闭烧录工具。'
}
try{
 Write-Host '刷机需谨慎' -ForegroundColor Cyan
 Write-Host '正在检查文件，请稍候……'
 $mp=Join-Path $internal 'manifest.json'
 if((Digest $mp) -ne $manifestSha){throw '工具包校验清单损坏，请完整重新解压。'}
 $script:manifest=ReadJson $mp;$script:files=@{}
 foreach($item in $manifest.files){if($files.ContainsKey($item.path)){throw '校验清单重复。'};AssertFile $item (SafePath $package $item.path);$files[$item.path]=$item}
 Write-Host '文件检查通过。' -ForegroundColor Green
 if($Mode -eq 'Check'){exit 0}
 if($Mode -eq 'Driver' -and $PrepareOnly){Write-Host '仅检查驱动安装流程，未安装驱动，也未请求管理员权限。';exit 0}
 if($Mode -eq 'Driver' -and !$DriverElevated){
  $identity=[Security.Principal.WindowsIdentity]::GetCurrent();$principal=New-Object Security.Principal.WindowsPrincipal($identity)
  if(!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)){
   Write-Host '即将请求管理员权限，请在Windows提示中选择“是”。'
   $args='-NoLogo -NoProfile -ExecutionPolicy Bypass -File "'+$PSCommandPath+'" -Mode Driver -DriverElevated'
   $child=Start-Process -FilePath "$PSHOME\powershell.exe" -Verb RunAs -ArgumentList $args -Wait -PassThru
   exit $child.ExitCode
  }
 }
 $mutex=New-Object Threading.Mutex($false,'Local\DISC-Forum-Toolkit');$locked=$mutex.WaitOne(0)
 if(!$locked){throw '另一个工具入口正在运行，请先完成或退出它。'}
 AssertNoCloner
 if($Mode -eq 'Driver'){
  if($env:PROCESSOR_ARCHITECTURE -eq 'ARM64' -or $env:PROCESSOR_ARCHITEW6432 -eq 'ARM64'){throw '此驱动仅适用于x86/x64 Windows，不支持ARM电脑。'}
  Write-Host '正在安装USB Boot驱动……'
  & "$env:SystemRoot\System32\pnputil.exe" /add-driver (Join-Path $internal 'Driver/usb_cloner_device.inf') /install
  $result=$LASTEXITCODE
  if($result -notin @(0,3010)){throw "Windows未能安装驱动（代码$result）。请保留上面的系统提示。"}
  Write-Host '驱动包安装完成。播放器进入USB Boot并连接后才能确认识别。' -ForegroundColor Green
  if($result -eq 3010){Write-Host 'Windows提示需要重启电脑，请自行安排重启。'}
  exit 0
 }
 if($Mode -eq 'Verify'){
  if(!$SessionPath){$SessionPath=(ReadJson (Join-Path $internal '上次备份.json')).session}
  $null=ArchiveSession $SessionPath;exit 0
 }
 if($Mode -eq 'Backup'){
  if(!$Scope){
   $choice=SelectScope '选择备份范围' $true
   if($choice -eq 'Cancel'){exit 0}
   if($choice -eq 'Resume'){$null=ArchiveSession ((ReadJson (Join-Path $internal '上次备份.json')).session);exit 0}
   $Scope=$choice
  }
  Write-Host ('本次备份：'+(ScopeTitle $Scope)+'。各读取两遍，只读不擦除。')
  $session=NewWorkspace 'Backup';$rec=PrepareCloner $session 'Backup' $Scope $null
  if($PrepareOnly){Write-Host "配置准备完成（未连接设备）：$session";exit 0}
  LaunchCloner $session $rec
  JsonFile (Join-Path $internal '上次备份.json') @{session=$session}
  while($true){
   $answer=Read-Host '全部读取成功并关闭烧录工具后，按回车校验保存；输入0稍后再校验'
   if($answer -eq '0'){Write-Host '读取记录已保留；下次运行“备份”，选择3即可继续校验。';exit 0}
   try{$null=ArchiveSession $session;break}catch{Write-Host ('暂未保存：'+$_.Exception.Message) -ForegroundColor Yellow}
  }
  exit 0
 }
 $own=$null
 if($Mode -eq 'Flash'){
  $Scope='Partial'
  if(!$PrepareOnly){
   $last=Join-Path $backupRoot '最近备份.txt'
   if(!(Test-Path -LiteralPath $last)){throw '请先运行“02-备份.bat”完成备份，成功保存后再刷入。'}
   $own=OpenBackup (SafePath $backupRoot ([IO.File]::ReadAllText($last,$utf8).Trim()));ValidateBackup $own 'Partial'
  }
 }else{
  if(!$BackupPath){$BackupPath=ChooseBackup};if(!$BackupPath){exit 0}
  $own=OpenBackup $BackupPath
  if(!$Scope){
   if($own.full){$choice=SelectScope '这是一份全量备份，选择恢复范围' $false $true;if($choice -eq 'Cancel'){exit 0};$Scope=$choice}
   else{$Scope='Partial'}
  }
  Write-Host ('正在校验选中的备份：'+$own.source)
  ValidateBackup $own $Scope
  Write-Host ('本次恢复：'+(ScopeTitle $Scope)) -ForegroundColor Yellow
  if(!$PrepareOnly){
   if($Scope -eq 'Full'){Write-Host '全量恢复会覆盖全部八分区及设备身份信息，只能恢复到备份所属的原机器。'}
   else{Write-Host '只恢复主系统和功能/个人数据，其余分区保持原样。'}
   Write-Host '请核对这是当前机器自己的备份，再在烧录工具点“开始”。'
  }
 }
 $session=NewWorkspace $Mode;$rec=PrepareCloner $session $Mode $Scope $own
 if($PrepareOnly){Write-Host "配置准备完成（未连接设备）：$session";exit 0}
 LaunchCloner $session $rec
 Write-Host "失败时请保留此运行目录：$session"
}catch{
 Write-Host ('未完成：'+$_.Exception.Message) -ForegroundColor Red
 Write-Host '请保留上面的错误信息。未自动重试刷写。'
 exit 1
}finally{
 foreach($dir in $scratch){
  # Only newly created extraction/archive workspaces are removed. Source
  # backups and device read sessions are never removed here.
  try{if(Test-Path -LiteralPath $dir){[IO.Directory]::Delete($dir,$true)}}catch{}
 }
 if($locked -and $mutex){$mutex.ReleaseMutex()};if($mutex){$mutex.Dispose()}
 if($DriverElevated -and !$PrepareOnly){$null=Read-Host '按回车关闭此窗口'}
}
