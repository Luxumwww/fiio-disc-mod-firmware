"""Project the reviewed App parameter definitions into the native UI."""
from pathlib import Path
import hashlib, json
HERE=Path(__file__).resolve().parent
source=HERE.parent/'mobile-remote/dsp-schema.json'
schema=json.loads(source.read_text(encoding='utf-8'))
def cstr(value): return json.dumps(value,ensure_ascii=False)
def cf(value):
    s=format(float(value),'.9g')
    return s+('f' if '.' in s or 'e' in s else '.0f')
lines=['/* Generated from mobile-remote/dsp-schema.json; do not hand-edit. */',
       '#ifndef DISC_NATIVE_PARAM_SCHEMA_H','#define DISC_NATIVE_PARAM_SCHEMA_H',
       'typedef struct {const char *label,*unit;float low,high,step,initial;} DiscNativeParam;',
       'typedef struct {const char *name;unsigned count;DiscNativeParam params[12];} DiscNativeEffect;',
       'static const DiscNativeEffect disc_native_effects[17]={']
for effect in schema['modules']:
    i=effect['index']
    assert i!=1 and 0<=i<=16 and len(effect['params'])<=12
    lines.append(' [%d]={%s,%du,{'%(i,cstr(effect['name']),len(effect['params'])))
    for p in effect['params']:
        assert p['max']>=p['min'] and p['step']>0
        lines.append('  {%s,%s,%s,%s,%s,%s},'%(cstr(p['label']),cstr(p.get('unit','')),*(cf(p[k]) for k in ('min','max','step','default'))))
    lines.append(' }},')
lines+=['};','#define DISC_PARAM_SCHEMA_SHA "%s"'%hashlib.sha256(source.read_bytes()).hexdigest(),'#endif','']
(HERE/'param_schema.h').write_text('\n'.join(lines),encoding='utf-8')
print('Generated native definitions for',len(schema['modules']),'effects')
