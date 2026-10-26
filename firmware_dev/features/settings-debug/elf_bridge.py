"""Extend a copy of stock librt's dependency list using unused ELF metadata slots.
No library instructions, existing symbols/versions or original ROM files change.
"""
from pathlib import Path
import hashlib,json,struct
HERE=Path(__file__).resolve().parent
BASE=Path('/root/disc-official-component-base-20261004-r1/base')
def sections(data):
 h=struct.unpack_from('<16sHHIIIIIHHHHHH',data)
 rows=[struct.unpack_from('<IIIIIIIIII',data,h[6]+i*h[11]) for i in range(h[12])]
 t=rows[h[13]];names=data[t[4]:t[4]+t[5]]
 return h,{names[r[0]:].split(b'\0',1)[0].decode(): (i,r) for i,r in enumerate(rows)}

def bridge(stock, dependency='/usr/data/disc-settings.so'):
 data=bytearray(stock);h,sec=sections(data)
 ph=[list(struct.unpack_from('<IIIIIIII',data,h[5]+i*h[9])) for i in range(h[10])]
 assert h[1]==3 and h[2]==8 and h[9]==32
 slot=next(i for i,p in enumerate(ph) if p[0]==0)
 dyn=next(p for p in ph if p[0]==2)
 entries=[struct.unpack_from('<II',data,i) for i in range(dyn[1],dyn[1]+dyn[4],8)]
 tags=dict(entries);null=next(i for i,e in enumerate(entries) if e[0]==0)
 assert entries[null+1]==(0,0)
 _,s=sec['.dynstr'];assert tags[5]==s[3] and tags[10]==s[5]
 original=bytes(data[s[4]:s[4]+s[5]])
 strings=original+dependency.encode('ascii')+b'\0'
 offset=(len(data)+65535)&~65535
 vaddr=(max(p[2]+p[5] for p in ph if p[0]==1)+65535)&~65535
 data.extend(bytes(offset-len(data)));data.extend(strings)
 struct.pack_into('<IIIIIIII',data,h[5]+slot*32,1,offset,vaddr,vaddr,len(strings),len(strings),4,65536)
 for i,(tag,value) in enumerate(entries):
  if tag==5:struct.pack_into('<II',data,dyn[1]+i*8,tag,vaddr)
  if tag==10:struct.pack_into('<II',data,dyn[1]+i*8,tag,len(strings))
 struct.pack_into('<II',data,dyn[1]+null*8,1,len(original))
 sh=s.copy() if isinstance(s,list) else list(s);sh[3]=vaddr;sh[4]=offset;sh[5]=len(strings)
 struct.pack_into('<IIIIIIIIII',data,h[6]+sec['.dynstr'][0]*40,*sh)
 # Verify every original code byte is identical, and all old symbol strings survive.
 for name,(_,r) in sec.items():
  if r[2]&4:assert data[r[4]:r[4]+r[5]]==stock[r[4]:r[4]+r[5]],name
 assert strings[:len(original)]==original
 return bytes(data)

def import_map(ui, symbol):
 h,sec=sections(ui);dyn=sec['.dynamic'][1]
 tags=dict(struct.unpack_from('<II',ui,i) for i in range(dyn[4],dyn[4]+dyn[5],8))
 symbols=sec['.dynsym'][1];strings=sec['.dynstr'][1];table=ui[strings[4]:strings[4]+strings[5]]
 for i in range(symbols[5]//16):
  n=struct.unpack_from('<I',ui,symbols[4]+16*i)[0]
  if table[n:].split(b'\0',1)[0]==symbol.encode('ascii'):
   # This ET_EXEC uses R_MIPS_JUMP_SLOT in .got.plt for imported functions.
   # Symbols below DT_MIPS_GOTSYM are NOT in the general MIPS GOT.
   rel=sec['.rel.plt'][1]
   cells=[a for a,info in (struct.unpack_from('<II',ui,p) for p in range(rel[4],rel[4]+rel[5],8)) if info>>8==i]
   assert len(cells)==1
   return {'symbol_index':i,'got':hex(cells[0])}
 raise ValueError(symbol+' import not found')

def ui_map(ui):
 cell=import_map(ui,'usleep')
 return {'usleep_symbol_index':cell['symbol_index'],'usleep_got':cell['got']}

if __name__=='__main__':
 source=BASE/'lib/librt.so.1';stock=source.read_bytes();output=HERE/'build/librt.so.1';output.parent.mkdir(exist_ok=True);output.write_bytes(bridge(stock))
 ui=(BASE/'usr/bin/mq_ui').read_bytes();report={'original_librt_sha256':hashlib.sha256(stock).hexdigest(),'bridge_sha256':hashlib.sha256(output.read_bytes()).hexdigest(),'original_instruction_bytes_unchanged':True,'original_rom_modified':False,'ui_sha256':hashlib.sha256(ui).hexdigest(),**ui_map(ui)}
 (HERE/'elf-map.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
