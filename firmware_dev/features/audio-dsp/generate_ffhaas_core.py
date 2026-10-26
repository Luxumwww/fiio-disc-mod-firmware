"""Mechanical extraction from locked FFmpeg n8.0; arithmetic copied verbatim."""
from pathlib import Path
import hashlib,json
P=Path(__file__).resolve().parent;D=P/'third_party/ffmpeg-haas/n8.0';s=(D/'af_haas.c').read_text();lock=json.loads((D/'source-lock.json').read_text())
assert hashlib.sha256((D/'af_haas.c').read_bytes()).hexdigest()==lock['files']['libavfilter/af_haas.c']
notice=s[:s.index('#include')]
struct=s[s.index('typedef struct HaasContext'):s.index('#define OFFSET')].replace('HaasContext','DiscFFHaas').replace('    const AVClass *class;\n','')
config=s[s.index('    size_t min_buf_size'):s.index('\n    return 0;\n}',s.index('static int config_input'))]
config=config.replace('inlink->sample_rate','rate').replace('av_freep(&s->buffer);','free(s->buffer);').replace('av_calloc','calloc').replace('if (!s->buffer)\n        return AVERROR(ENOMEM);','if (!s->buffer) {free(s);return 0;}')
loop=s[s.index('    for (n = 0; n < in->nb_samples;'):s.index('\n    if (out != in)',s.index('static int filter_frame'))]
loop=loop.replace('for (n = 0; n < in->nb_samples;','for (unsigned n = 0; n < frames;')
out=notice+'''/* DISC adaptation, 2026-10-18: FFmpeg n8.0, commit '''+lock['commit']+'''.
 * Framework/frame allocation replaced by create/reset/process calls. The
 * coefficient calculations and sample-loop body below retain upstream order
 * and double precision. Compile this translation unit with FP contraction OFF.
 * Original source and LGPL license accompany the component source package. */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ffhaas_params.h"
#include "ffhaas_core.h"
#define MAX_HAAS_DELAY 40
'''+struct+'''
DiscFFHaas *disc_ffhaas_create(unsigned rate,const float *p){
 if(rate<8000||rate>384000||!disc_ffhaas_params_valid(p))return 0;
 DiscFFHaas *s=calloc(1,sizeof(*s));if(!s)return 0;
 s->level_in=p[0];s->level_out=p[1];s->par_side_gain=p[2];
 s->par_m_source=(unsigned)p[3]&3u;s->par_middle_phase=(unsigned)p[3]>>2;
 s->par_delay0=p[4];s->par_balance0=p[5];s->par_gain0=p[6];s->par_phase0=(int)p[7];
 s->par_delay1=p[8];s->par_balance1=p[9];s->par_gain1=p[10];s->par_phase1=(int)p[11];
'''+config+'''
 return s;
}
void disc_ffhaas_destroy(DiscFFHaas *s){if(s){free(s->buffer);free(s);}}
void disc_ffhaas_reset(DiscFFHaas *s){memset(s->buffer,0,s->buffer_size*sizeof(*s->buffer));s->write_ptr=0;}
unsigned disc_ffhaas_memory(const DiscFFHaas *s){return (unsigned)(sizeof(*s)+s->buffer_size*sizeof(*s->buffer));}
void disc_ffhaas_process_double(DiscFFHaas *s,const double *src,double *dst,unsigned frames){
 const double level_in=s->level_in;
 const double level_out=s->level_out;
 const uint32_t mask=s->buffer_size-1;
 double *buffer=s->buffer;
'''+loop+'''
}
void disc_ffhaas_sample(DiscFFHaas *s,float *io){double src[2]={io[0],io[1]},out[2];disc_ffhaas_process_double(s,src,out,1);io[0]=(float)out[0];io[1]=(float)out[1];}
'''
(P/'ffhaas_core.c').write_text(out,newline='\n')
(P/'reports/1.10.0/extraction.json').write_text(json.dumps({'upstream_commit':lock['commit'],'source_sha256':lock['files']['libavfilter/af_haas.c'],'sample_body':'verbatim; only for-loop counter/frame-count glue renamed','core_sha256':hashlib.sha256(out.encode()).hexdigest()},indent=2)+'\n')
print('Generated attributed FFmpeg core: coefficient arithmetic and sample body retained verbatim')
