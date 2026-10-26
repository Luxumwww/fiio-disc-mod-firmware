"""Extract the pinned MIT WinVST PowerSag2 double path reproducibly.

The exported core retains upstream independent channels and frame-based timing.
Any linked bass detector/control-rate adaptation belongs to the project wrapper.
"""
from pathlib import Path
import hashlib
import json
import re
import shutil

P=Path(__file__).resolve().parent
U=P/'third_party/airwindows-powersag2'
T=P/'tests_powersag2'
lock=json.loads((U/'source-lock.json').read_text())
for name,expected in lock['files'].items():
    source=U/('AIRWINDOWS-LICENSE' if name=='LICENSE' else Path(name).name)
    assert hashlib.sha256(source.read_bytes()).hexdigest()==expected,source
T.mkdir(exist_ok=True)
source=(U/'PowerSag2Proc.cpp').read_text()
body=source[source.index('void PowerSag2::processDoubleReplacing'):]
body=body[body.index('{')+1:body.rfind('}')]
_,loop=body.split('    while (--sampleFrames >= 0)',1)
for name in ('dL','dR','controlL','controlR','gcount','fpdL','fpdR'):
    loop=re.sub(r'\b'+name+r'\b','s->'+name,loop)
for pointer in ('in1','in2','out1','out2'):
    loop=loop.replace('*'+pointer+'++;',pointer+' += 2;')
loop=loop.replace('sqrt(offset)','sqrt_offset')
prefix='''/* Airwindows PowerSag2 by Chris Johnson, MIT.
 * Generated from locked WinVST source by generate_powersag2_core.py.
 * Original double per-sample arithmetic, independent channel envelopes, ring
 * indices and noise prevention retained. VST shell replaced by an interleaved
 * adapter and cached constant coefficients. Compile with -ffp-contract=off.
 * Bass-band/stereo-link/control-rate behavior belongs to the project wrapper;
 * this core makes no claim to simulate a complete physical tube output stage.
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "powersag2_core.h"
struct DiscPowerSag2 {
    double dL[16386],dR[16386],controlL,controlR;
    double wet,sqrt_offset;
    float A,B;
    int gcount,offset;
    uint32_t fpdL,fpdR;
};
static void configure(DiscPowerSag2 *s,double range,double invwet) {
    s->A=(float)range;s->B=(float)invwet;
    s->fpdL=s->fpdR=16386u;
    double depth=pow(s->A,4);
    s->offset=(int)(depth*16383)+1;
    s->wet=(s->B*2.0)-1.0;
    s->sqrt_offset=sqrt(s->offset);
}
DiscPowerSag2 *disc_powersag2_create(double range,double invwet) {
    if(!isfinite(range)||!isfinite(invwet)||range<0||range>1||invwet<0||invwet>1)return NULL;
    DiscPowerSag2 *s=calloc(1,sizeof(*s));
    if(s)configure(s,range,invwet);
    return s;
}
void disc_powersag2_destroy(DiscPowerSag2 *s){free(s);}
void disc_powersag2_reset(DiscPowerSag2 *s){
    if(!s)return;
    double a=s->A,b=s->B;
    memset(s,0,sizeof(*s));configure(s,a,b);
}
unsigned disc_powersag2_memory(const DiscPowerSag2 *s){return s?(unsigned)sizeof(*s):0;}
void disc_powersag2_process_double(DiscPowerSag2 *s,const double *inputs,double *outputs,int sampleFrames){
    if(!s||!inputs||!outputs||sampleFrames<=0)return;
    const double *in1=inputs,*in2=inputs+1;
    double *out1=outputs,*out2=outputs+1;
    int offset=s->offset;
    double wet=s->wet,sqrt_offset=s->sqrt_offset;
'''
suffix='''}
void disc_powersag2_sample_double(DiscPowerSag2 *s,double io[2]){
    disc_powersag2_process_double(s,io,io,1);
}
/* The linked wrapper feeds the same nonnegative detector to both original
 * channels. Repeating the independent R history cannot change L, so expose L's
 * pre-output clamp directly, preserving its arithmetic and ring index order. */
double disc_powersag2_linked_gain(DiscPowerSag2 *s,double detector){
    if(!s)return 1.0;
    double inputSampleL=detector;
    if(fabs(inputSampleL)<1.18e-23)inputSampleL=s->fpdL*1.18e-17;
    int offset=s->offset;
    if(s->gcount<0||s->gcount>16384)s->gcount=16384;
    s->dL[s->gcount]=fabs(inputSampleL);
    s->controlL+=s->dL[s->gcount];
    s->controlL-=s->dL[s->gcount+offset-((s->gcount+offset>16384)?16384:0)];
    s->gcount--;
    if(s->controlL>offset)s->controlL=offset;
    if(s->controlL<0.0)s->controlL=0.0;
    double burst=inputSampleL*(s->controlL/s->sqrt_offset);
    double clamp=inputSampleL/((burst==0.0)?1.0:burst);
    if(clamp>1.0)clamp=1.0;
    if(clamp<0.0)clamp=0.0;
    s->fpdL^=s->fpdL<<13;s->fpdL^=s->fpdL>>17;s->fpdL^=s->fpdL<<5;
    return clamp;
}
'''
(P/'powersag2_core.c').write_text(prefix+'    while (--sampleFrames >= 0)'+loop+suffix,encoding='utf-8',newline='\n')
shutil.copy2(U/'PowerSag2Proc.cpp',T/'PowerSag2Proc.cpp')
print('Generated locked WinVST PowerSag2 double core')
