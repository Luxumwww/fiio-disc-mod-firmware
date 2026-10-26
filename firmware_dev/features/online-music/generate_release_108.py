"""Derive one-shot108/062 preparation and installer from107 without touching releases."""
from pathlib import Path
import re
p=Path(__file__).resolve().parent
versions={'1.3.104':'1.3.105','1.3.105':'1.3.106','1.3.106':'1.3.107','1.3.107':'1.3.108','1.10.106':'1.10.107','1.10.107':'1.10.108','0.3.0':'0.4.0','0.4.0':'0.5.0','0.5.0':'0.6.1','0.6.1':'0.6.2','0.6.0':'0.6.2'}
pattern=re.compile('|'.join(re.escape(k) for k in sorted(versions,key=len,reverse=True)))
def advance(s):return pattern.sub(lambda m:versions[m[0]],s)
install=advance((p/'install_playback_fixes.py').read_text())
install=install.replace('backend061','backend062').replace('host107','host108').replace('bridge107','bridge108')
(p/'install_transport_fixes.py').write_text(install,encoding='utf-8',newline='\n')
prepare=advance((p/'prepare_playback_fixes.py').read_text())
# Bootstrap in107 accepts106's bridge;108 must accept107's bridge.
prepare=prepare.replace('a61721746740ffde3300cd2c0f52247fb494f8293ceda7fa995e21de6bb1b8e4','f23b9dad3bdd4a185a1d097fb6c8d9bf3faadf68a75937b95b4f2479f755c715')
prepare=prepare.replace("s=s.replace('f23b9dad3bdd4a185a1d097fb6c8d9bf3faadf68a75937b95b4f2479f755c715','f23b9dad3bdd4a185a1d097fb6c8d9bf3faadf68a75937b95b4f2479f755c715')", "s=s.replace('f23b9dad3bdd4a185a1d097fb6c8d9bf3faadf68a75937b95b4f2479f755c715','684bd066970bf2e1ee63f29cd41e1f82f8002f1c0c2aec4ae7ddff2e04c1fee8')")
prepare=prepare.replace("'old_menu_sha256':'f23b9dad3bdd4a185a1d097fb6c8d9bf3faadf68a75937b95b4f2479f755c715'", "'old_menu_sha256':'684bd066970bf2e1ee63f29cd41e1f82f8002f1c0c2aec4ae7ddff2e04c1fee8'")
prepare=prepare.replace("assert report['menu_sha256']=='684bd066970bf2e1ee63f29cd41e1f82f8002f1c0c2aec4ae7ddff2e04c1fee8'", "assert report['menu_sha256']=='cc8e45997db92be2640a806321c08ac04a54a4782ba52ccdd18d33f7425d6854'")
prepare=prepare.replace('21 regression','26 regression')
prepare=prepare.replace('circular title/artist marquee','native measured horizontal-overflow-only title/artist marquee and static playlist counts')
prepare=prepare.replace('shade resume card','floating shade resume card excluded from parent auto-layout')
prepare=prepare.replace('Fixes empty final control parameters,', 'Fixes actual X2000 colon-separated ALSA hw_ptr clock when native RelTime is zero, closed PCM natural-end fallback, bounded receiver readiness retries, WLAN selection, stale renderer endpoint refresh and six-second decoder control timeout. RAM-only recent stage timing, loading errors return to browser. Fixes empty final control parameters,')
(p/'prepare_transport_fixes.py').write_text(prepare,encoding='utf-8',newline='\n')
print('Generated108/062 preparation and guarded installer; not executed.')
