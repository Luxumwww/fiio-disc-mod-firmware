from pathlib import Path
import subprocess
F=Path(__file__).resolve().parent.parent
for rel,a,b in [('bluetooth-meter/build.py','1.3.123','1.3.124'),('bluetooth-meter/build.py','1.10.123','1.10.124'),('bluetooth-meter/meter.c','1.3.123','1.3.124')]:
 f=F/rel;f.write_text(f.read_text().replace(a,b))
f=F/'settings-debug/build.py';s=f.read_text().replace('addresses += [0x459470]','addresses += [0x459470,0x4e9c04,0x4e9e0c,0x4e98f8,0x4e9918]');f.write_text(s)
s=(F/'mobile-remote/install_getter_update.py').read_text().replace("O=P/'reports/host123'","O=P/'reports/host124'").replace("target=base+'/releases/1.3.120'","target=base+'/releases/1.3.121'").replace('releases/1.3.122 ]','releases/1.3.123 ]').replace('releases/1.3.121 ]','releases/1.3.122 ]').replace('unused-host120','unused-host121').replace('backup120','backup121').replace('unused120','unused121').replace("[('disc-bt-meter','1.3.123')]","[('disc-bt-meter','1.3.124')]").replace("replace('1.10.121','1.10.123')","replace('1.10.121','1.10.124')").replace('3f3aba2dc79950dc2a4aa160cc6814c3472abdd62b7a4cebbb008b868c249582','a40fd93a570ae3ad6106567c7426bbf085d2238b6ab2af1cb2cd0056c4c35198').replace('Host123/UI123','Host124/UI124')
(F/'mobile-remote/install_switch_update.py').write_text(s)
subprocess.run(['wsl','-d','LDAC-WSL','--','sh','-lc','cd "/mnt/d/AI workflow/codex/disc/firmware_dev/features/bluetooth-meter" && python3 build.py'],check=True)
