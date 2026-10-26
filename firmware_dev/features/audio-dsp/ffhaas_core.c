/*
 * Copyright (c) 2001-2010 Vladimir Sadovnikov
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* DISC adaptation, 2026-10-18: FFmpeg n8.0, commit 140fd653aed8cad774f991ba083e2d01e86420c7.
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
typedef struct DiscFFHaas {

    int par_m_source;
    double par_delay0;
    double par_delay1;
    int par_phase0;
    int par_phase1;
    int par_middle_phase;
    double par_side_gain;
    double par_gain0;
    double par_gain1;
    double par_balance0;
    double par_balance1;
    double level_in;
    double level_out;

    double *buffer;
    size_t buffer_size;
    uint32_t write_ptr;
    uint32_t delay[2];
    double balance_l[2];
    double balance_r[2];
    double phase0;
    double phase1;
} DiscFFHaas;


DiscFFHaas *disc_ffhaas_create(unsigned rate,const float *p){
 if(rate<8000||rate>384000||!disc_ffhaas_params_valid(p))return 0;
 DiscFFHaas *s=calloc(1,sizeof(*s));if(!s)return 0;
 s->level_in=p[0];s->level_out=p[1];s->par_side_gain=p[2];
 s->par_m_source=(unsigned)p[3]&3u;s->par_middle_phase=(unsigned)p[3]>>2;
 s->par_delay0=p[4];s->par_balance0=p[5];s->par_gain0=p[6];s->par_phase0=(int)p[7];
 s->par_delay1=p[8];s->par_balance1=p[9];s->par_gain1=p[10];s->par_phase1=(int)p[11];
    size_t min_buf_size = (size_t)(rate * MAX_HAAS_DELAY * 0.001);
    size_t new_buf_size = 1;

    while (new_buf_size < min_buf_size)
        new_buf_size <<= 1;

    free(s->buffer);
    s->buffer = calloc(new_buf_size, sizeof(*s->buffer));
    if (!s->buffer) {free(s);return 0;}

    s->buffer_size = new_buf_size;
    s->write_ptr = 0;

    s->delay[0] = (uint32_t)(s->par_delay0 * 0.001 * rate);
    s->delay[1] = (uint32_t)(s->par_delay1 * 0.001 * rate);

    s->phase0 = s->par_phase0 ? 1.0 : -1.0;
    s->phase1 = s->par_phase1 ? 1.0 : -1.0;

    s->balance_l[0] = (s->par_balance0 + 1) / 2 * s->par_gain0 * s->phase0;
    s->balance_r[0] = (1.0 - (s->par_balance0 + 1) / 2) * (s->par_gain0) * s->phase0;
    s->balance_l[1] = (s->par_balance1 + 1) / 2 * s->par_gain1 * s->phase1;
    s->balance_r[1] = (1.0 - (s->par_balance1 + 1) / 2) * (s->par_gain1) * s->phase1;

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
    for (unsigned n = 0; n < frames; n++, src += 2, dst += 2) {
        double mid, side[2], side_l, side_r;
        uint32_t s0_ptr, s1_ptr;

        switch (s->par_m_source) {
        case 0: mid = src[0]; break;
        case 1: mid = src[1]; break;
        case 2: mid = (src[0] + src[1]) * 0.5; break;
        case 3: mid = (src[0] - src[1]) * 0.5; break;
        }

        mid *= level_in;

        buffer[s->write_ptr] = mid;

        s0_ptr = (s->write_ptr + s->buffer_size - s->delay[0]) & mask;
        s1_ptr = (s->write_ptr + s->buffer_size - s->delay[1]) & mask;

        if (s->par_middle_phase)
            mid = -mid;

        side[0] = buffer[s0_ptr] * s->par_side_gain;
        side[1] = buffer[s1_ptr] * s->par_side_gain;
        side_l  = side[0] * s->balance_l[0] - side[1] * s->balance_l[1];
        side_r  = side[1] * s->balance_r[1] - side[0] * s->balance_r[0];

        dst[0] = (mid + side_l) * level_out;
        dst[1] = (mid + side_r) * level_out;

        s->write_ptr = (s->write_ptr + 1) & mask;
    }

}
void disc_ffhaas_sample(DiscFFHaas *s,float *io){double src[2]={io[0],io[1]},out[2];disc_ffhaas_process_double(s,src,out,1);io[0]=(float)out[0];io[1]=(float)out[1];}
