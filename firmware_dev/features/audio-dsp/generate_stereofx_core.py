from pathlib import Path
import re,json,hashlib,shutil
P=Path(__file__).resolve().parent;D=P/'reports/stereo-research';U=P/'third_party/airwindows-stereofx';T=P/'tests_stereofx';O=P/'reports/1.11.0';U.mkdir(exist_ok=True);T.mkdir(exist_ok=True)
commit=(D/'airwindows-commit.txt').read_text().strip();files={}
for name in ('StereoFX.cpp','StereoFX.h','StereoFXProc.cpp','AIRWINDOWS-LICENSE'):
 q=U/name
 if q.exists():assert q.read_bytes()==(D/name).read_bytes()
 else:shutil.copy2(D/name,q)
 files['LICENSE' if name=='AIRWINDOWS-LICENSE' else 'plugins/WinVST/StereoFX/'+name]=hashlib.sha256(q.read_bytes()).hexdigest()
(U/'source-lock.json').write_text(json.dumps({'project':'Airwindows StereoFX','repository':'https://github.com/airwindows/airwindows','commit':commit,'files':files},indent=2)+'\n')
s=(U/'StereoFXProc.cpp').read_text();body=s[s.index('void StereoFX::processDoubleReplacing'):];body=body[body.index('{')+1:body.rfind('}')]
prologue,loop=body.split('    while (--sampleFrames >= 0)',1)
# Original double processing path has no output dither addition. Keep its noise prevention/RNG.
prologue=prologue.replace('getSampleRate()','s->rate')
for name in ('iirSampleA','iirSampleB','fpdL','fpdR','flip'):
 loop=re.sub(r'\b'+name+r'\b','s->'+name,loop)
loop=loop.replace('*in1++;','in1++;').replace('*in2++;','in2++;').replace('*out1++;','out1++;').replace('*out2++;','out2++;')
prologue=prologue.replace('double* in1  =  inputs[0];','const double* in1=inputs;').replace('double* in2  =  inputs[1];','const double* in2=inputs+1;').replace('double* out1 = outputs[0];','double* out1=outputs;').replace('double* out2 = outputs[1];','double* out2=outputs+1;')
# Interleaved adapter replaces only the four VST pointer increments.
loop=loop.replace('in1++;','in1+=2;').replace('in2++;','in2+=2;').replace('out1++;','out1+=2;').replace('out2++;','out2+=2;')
prologue=re.sub(r'\bA\b','s->A',prologue);prologue=re.sub(r'\bB\b','s->B',prologue);prologue=re.sub(r'\bC\b','s->C',prologue)
# Cache the original coefficient calculations at context creation, in their original order.
for var in ('overallscale','stereowide','centersquish','density','sustain','iirAmount'):
 prologue=re.sub(r'\bdouble '+var+r' = [^;]+;', 'double '+var+' = s->'+var+';',prologue)
prologue=prologue.replace('overallscale /= 44100.0;','').replace('overallscale *= s->rate;','')
banner='''/* StereoFX by Chris Johnson / Airwindows, Copyright (c) 2016, MIT.
 * Pinned source in third_party/airwindows-stereofx. VST wrapper replaced with
 * create/reset/interleaved callbacks; original double sample arithmetic retained.
 * Coefficients cached, deterministic equal RNG seeds, no added IR or BASS dependency.
 */
'''
prefix=banner+'''#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "stereofx_core.h"
#include "stereofx_params.h"
struct DiscStereoFx {double rate,A,B,C,iirSampleA,iirSampleB,overallscale,stereowide,centersquish,density,sustain,iirAmount;uint32_t fpdL,fpdR;int flip;};
static void configure(DiscStereoFx *s,unsigned rate,const float *p){
 s->rate=rate;s->A=p[0];s->B=p[1];s->C=p[2];s->fpdL=s->fpdR=16386u;
 s->overallscale=1.0;s->overallscale/=44100.0;s->overallscale*=s->rate;
 s->stereowide=s->A;s->centersquish=s->C;s->density=s->stereowide*2.4;
 s->sustain=1.0-(1.0/(1.0+(s->density/7.0)));s->iirAmount=pow(s->B,3)/s->overallscale;
}
DiscStereoFx *disc_stereofx_create(unsigned rate,const float *p){if(rate<8000||rate>384000||!disc_stereofx_params_valid(p))return 0;DiscStereoFx *s=calloc(1,sizeof(*s));if(s)configure(s,rate,p);return s;}
void disc_stereofx_destroy(DiscStereoFx *s){free(s);}
void disc_stereofx_reset(DiscStereoFx *s){unsigned r=(unsigned)s->rate;float p[12]={(float)s->A,(float)s->B,(float)s->C};memset(s,0,sizeof(*s));configure(s,r,p);}
unsigned disc_stereofx_memory(const DiscStereoFx *s){(void)s;return sizeof(*s);}
void disc_stereofx_process_double(DiscStereoFx *s,const double *inputs,double *outputs,int sampleFrames){
'''
(P/'stereofx_core.c').write_text(prefix+prologue+'    while (--sampleFrames >= 0)'+loop+'}\nvoid disc_stereofx_sample(DiscStereoFx *s,float *io){double in[2]={io[0],io[1]},out[2];disc_stereofx_process_double(s,in,out,1);io[0]=(float)out[0];io[1]=(float)out[1];}\n')
# Test-only tiny VST-facing class allows compiling the unmodified upstream Proc.cpp.
(T/'StereoFX.h').write_text('''#ifndef DISC_STEREOFX_REFERENCE_H
#define DISC_STEREOFX_REFERENCE_H
#include <stdint.h>
#include <math.h>
typedef int VstInt32;
class StereoFX {public:float A=0,B=0,C=0;double iirSampleA=0,iirSampleB=0;uint32_t fpdL=16386,fpdR=16386;bool flip=false;float rate=48000;float getSampleRate(){return rate;}void processReplacing(float**,float**,VstInt32);void processDoubleReplacing(double**,double**,VstInt32);};
#endif
''')
shutil.copy2(U/'StereoFXProc.cpp',T/'StereoFXProc.cpp')
(T/'reference.cpp').write_text('''#include "StereoFX.h"
extern "C" {
void *stereofx_reference_create(unsigned rate,const float *p){StereoFX *s=new StereoFX;s->rate=rate;s->A=p[0];s->B=p[1];s->C=p[2];return s;}
void stereofx_reference_sample(void *v,double *io){StereoFX *s=(StereoFX*)v;double l=io[0],r=io[1],*in[2]={&l,&r},*out[2]={io,io+1};s->processDoubleReplacing(in,out,1);}
void stereofx_reference_destroy(void *v){delete (StereoFX*)v;}
}
''')
(O/'extraction.json').write_text(json.dumps({'project':'Airwindows StereoFX','commit':commit,'method':'original double processing loop, interleaved wrapper; cached coefficients','rng':'deterministic identical seeds16386; original noise prevention and RNG retained','parameter_limits':'B<=0.7 ensures IIR amount <2 throughout 8..384k; A/C original0..1','audition_default':[.5,0,0],'upstream_factory_default':[0,0,0],'precision':'double; FP contraction off','core_sha256':hashlib.sha256((P/'stereofx_core.c').read_bytes()).hexdigest()},indent=2)+'\n')
# Earlier JamesDSP exploration is research only and is not built or exposed as an effect.
research=D/'jdsp-exploration';research.mkdir(exist_ok=True)
for name in ('generate_jdsp_core.py','jdsp_params.h','jdsp_core.h'):
 p=P/name
 if p.exists():shutil.move(str(p),str(research/name))
print('Generated actual StereoFX port, locked MIT source and independent unmodified upstream reference')
