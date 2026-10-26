from pathlib import Path
import subprocess
P=Path(__file__).resolve().parent
F=P.parent
for rel in ('bluetooth-meter/build.py','bluetooth-meter/meter.c'):
    f=F/rel
    s=f.read_text().replace('1.3.124','1.3.125').replace('1.10.124','1.10.125')
    f.write_text(s)
f=F/'settings-debug/build.py'
s=f.read_text().replace('addresses += [0x459470,','addresses += [0x533960,0x4eb4a4,0x4f6c20,0x459470,')
f.write_text(s)
s=(P/'install_switch_update.py').read_text()
s=s.replace("O=P/'reports/host124'","O=P/'reports/host125'")
s=s.replace("target=base+'/releases/1.3.121'","target=base+'/releases/1.3.122'")
s=s.replace('releases/1.3.123 ]','releases/1.3.124 ]').replace('releases/1.3.122 ]','releases/1.3.123 ]')
s=s.replace('unused-host121','unused-host122').replace('backup121','backup122').replace('unused121','unused122')
s=s.replace("[('disc-bt-meter','1.3.124')]","[('disc-bt-meter','1.3.125')]")
s=s.replace("replace('1.10.121','1.10.124')","replace('1.10.121','1.10.125')")
s=s.replace('a40fd93a570ae3ad6106567c7426bbf085d2238b6ab2af1cb2cd0056c4c35198','78674c3d9b6d6ff4acd7aeb6cf682a636a0f847ceb2fbc6d0f13c6bc6acffaa0')
s=s.replace('Host124/UI124: BLE control switch appended to native Settings/Bluetooth without changing stock child indexes.','Host125/UI125: BLE control row reproduces the native Bluetooth row constructor, switch dimensions, alignment, colors, padding and label style; stock child indexes retained.')
(P/'install_stock_row_update.py').write_text(s)
subprocess.run(['wsl','-d','LDAC-WSL','--','sh','-lc','cd "/mnt/d/AI workflow/codex/disc/firmware_dev/features/bluetooth-meter" && python3 build.py'],check=True)
