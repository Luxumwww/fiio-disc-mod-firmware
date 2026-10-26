/* freeverb.c - see freeverb.h
 *
 * Algorithm: Jezar's FreeVerb (public domain) - 8 parallel damped lowpass-comb filters
 * per channel followed by 4 series allpass filters, with the classic 23-sample stereo
 * spread.  This is the same topology BASS_FX uses to implement BASS_BFX_FREEVERB.
 *
 * Scaling constants are the ones from the original FreeVerb source and are what make
 * the BASS_FX parameter ranges (fRoomSize/fDamp/fWidth 0..1, fDryMix/fWetMix 0..2) map
 * onto the DSP state.
 */
#include "freeverb.h"
#include <string.h>
#include <math.h>

static const int COMB_TUNING[FV_NUMCOMBS]      = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
static const int ALLPASS_TUNING[FV_NUMALLPASS] = {556, 441, 341, 225};

#define FV_FIXEDGAIN   0.015f
#ifndef FV_SCALEWET
#define FV_SCALEWET    0.441f   /* 实测校准（对比 BASS IR） */
#endif
#ifndef FV_SCALEDRY
#define FV_SCALEDRY    1.0f    /* 实测校准 */
#endif
#define FV_SCALEDAMP   0.4f
#ifndef FV_SCALEROOM
#define FV_SCALEROOM   0.28f   /* 实测校准：保持 FreeVerb 原值，与 BASS IR 的 RT60 一致 */
#endif
#define FV_OFFSETROOM  0.7f
#define FV_INITIALROOM 0.5f
#define FV_INITIALDAMP 0.5f
#define FV_INITIALWET  (1.0f/3.0f)
#define FV_INITIALDRY  0.0f
#define FV_INITIALWIDTH 1.0f

static void update(freeverb_t *fv) {
    const BASS_BFX_FREEVERB *p = &fv->p;
    float room = p->fRoomSize, damp = p->fDamp, wet = p->fWetMix, dry = p->fDryMix, width = p->fWidth;

    if (room < 0.0f) room = 0.0f;
    if (room > 1.0f) room = 1.0f;
    if (damp < 0.0f) damp = 0.0f;
    if (damp > 1.0f) damp = 1.0f;
    if (width < 0.0f) width = 0.0f;
    if (width > 1.0f) width = 1.0f;

    fv->roomsize1 = room * FV_SCALEROOM + FV_OFFSETROOM;
    fv->damp1     = damp * FV_SCALEDAMP;
    fv->damp2     = 1.0f - fv->damp1;
    fv->wet       = wet * FV_SCALEWET;
    fv->dry       = dry * FV_SCALEDRY;
    fv->gain      = FV_FIXEDGAIN;
    fv->freeze    = (p->lMode & BASS_BFX_FREEVERB_MODE_FREEZE) ? 1 : 0;

    fv->wet1 = fv->wet * (width / 2.0f + 0.5f);
    fv->wet2 = fv->wet * ((1.0f - width) / 2.0f);
}

void freeverb_init(freeverb_t *fv, int sampleRate) {
    int i;
    memset(fv, 0, sizeof(*fv));
    fv->sampleRate = sampleRate > 0 ? sampleRate : 44100;
    fv->p.fDryMix   = 0.85f;   /* Salt Player "Concert Hall Atmosphere" */
    fv->p.fWetMix   = 0.75f;
    fv->p.fRoomSize = 0.9f;
    fv->p.fDamp     = 0.4f;
    fv->p.fWidth    = 0.75f;
    fv->p.lMode     = 0;
    fv->p.lChannel  = -1;      /* BASS_BFX_CHANALL */
    for (i = 0; i < FV_NUMCOMBS; i++) {
        fv->len[i] = (COMB_TUNING[i] * fv->sampleRate) / 44100;
        if (fv->len[i] < 1) fv->len[i] = 1;
        if (fv->len[i] > FV_MAXCOMB) fv->len[i] = FV_MAXCOMB;
    }
    for (i = 0; i < FV_NUMALLPASS; i++) {
        fv->alen[i] = (ALLPASS_TUNING[i] * fv->sampleRate) / 44100;
        if (fv->alen[i] < 1) fv->alen[i] = 1;
        if (fv->alen[i] > FV_MAXALLPASS) fv->alen[i] = FV_MAXALLPASS;
    }
    update(fv);
}

void freeverb_set_params(freeverb_t *fv, const BASS_BFX_FREEVERB *p) {
    fv->p = *p;
    /* The original FreeVerb accepts 0..1 for room/damp; BASS clamp-notes say the same. */
    update(fv);
}

void freeverb_mute(freeverb_t *fv) {
    int i;
    for (i = 0; i < FV_NUMCOMBS; i++) {
        memset(fv->bufL[i], 0, sizeof(float) * fv->len[i]);
        memset(fv->bufR[i], 0, sizeof(float) * fv->len[i]);
        fv->idxL[i] = fv->idxR[i] = 0;
        fv->filtL[i] = fv->filtR[i] = 0.0f;
    }
    for (i = 0; i < FV_NUMALLPASS; i++) {
        memset(fv->abufL[i], 0, sizeof(float) * fv->alen[i]);
        memset(fv->abufR[i], 0, sizeof(float) * fv->alen[i]);
        fv->aidxL[i] = fv->aidxR[i] = 0;
    }
}

static inline float comb(float *buf, int len, int *idx, float *filterstore,
                         float input, float feedback, float damp1, float damp2) {
    float output = buf[*idx];
    *filterstore = output * damp2 + *filterstore * damp1;
    buf[*idx] = input + *filterstore * feedback;
    if (++(*idx) >= len) *idx = 0;
    return output;
}

static inline float allpass(float *buf, int len, int *idx, float input, float feedback) {
    float bufout = buf[*idx];
    float output = -input + bufout;
    buf[*idx] = input + bufout * feedback;
    if (++(*idx) >= len) *idx = 0;
    return output;
}

#ifdef DISC_DSP_UNITY
__attribute__((always_inline))
#endif
void freeverb_process(freeverb_t *fv, float *io, int frames) {
    int n, i;
    const int doL = (fv->p.lChannel == -1) || (fv->p.lChannel & 1);
    const int doR = (fv->p.lChannel == -1) || (fv->p.lChannel & 2);
    const float feedback = fv->freeze ? 1.0f : fv->roomsize1;

    for (n = 0; n < frames; n++) {
        float inL = io[n * 2 + 0];
        float inR = io[n * 2 + 1];
        float input = (inL + inR) * fv->gain;
        float outL = 0.0f, outR = 0.0f;

#if defined(__mips_msa) && !defined(DISC_HALL_SCALAR)
        v4f32 accum={0,0,0,0};
        for(i=0;i<FV_NUMCOMBS;i+=2){
            v4f32 o={fv->bufL[i][fv->idxL[i]],fv->bufR[i][fv->idxR[i]],fv->bufL[i+1][fv->idxL[i+1]],fv->bufR[i+1][fv->idxR[i+1]]};
            v4f32 f={fv->filtL[i],fv->filtR[i],fv->filtL[i+1],fv->filtR[i+1]};
            f=o*fv->damp2+f*fv->damp1;v4f32 next=f*feedback+input;accum+=o;
            fv->filtL[i]=f[0];fv->filtR[i]=f[1];fv->filtL[i+1]=f[2];fv->filtR[i+1]=f[3];
            fv->bufL[i][fv->idxL[i]]=next[0];fv->bufR[i][fv->idxR[i]]=next[1];fv->bufL[i+1][fv->idxL[i+1]]=next[2];fv->bufR[i+1][fv->idxR[i+1]]=next[3];
            for(int k=i;k<i+2;k++){if(++fv->idxL[k]>=fv->len[k])fv->idxL[k]=0;if(++fv->idxR[k]>=fv->len[k])fv->idxR[k]=0;}
        }
        outL=accum[0]+accum[2];outR=accum[1]+accum[3];
#else
        for (i = 0; i < FV_NUMCOMBS; i++) {
            outL += comb(fv->bufL[i], fv->len[i], &fv->idxL[i], &fv->filtL[i], input, feedback, fv->damp1, fv->damp2);
            outR += comb(fv->bufR[i], fv->len[i], &fv->idxR[i], &fv->filtR[i], input, feedback, fv->damp1, fv->damp2);
        }
#endif
        for (i = 0; i < FV_NUMALLPASS; i++) {
            outL = allpass(fv->abufL[i], fv->alen[i], &fv->aidxL[i], outL, 0.5f);
            outR = allpass(fv->abufR[i], fv->alen[i], &fv->aidxR[i], outR, 0.5f);
        }

        float wetL = outL * fv->wet1 + outR * fv->wet2;
        float wetR = outR * fv->wet1 + outL * fv->wet2;

        io[n * 2 + 0] = doL ? (wetL + inL * fv->dry) : inL;
        io[n * 2 + 1] = doR ? (wetR + inR * fv->dry) : inR;
    }
}
