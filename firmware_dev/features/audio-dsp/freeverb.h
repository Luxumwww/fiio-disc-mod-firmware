/* freeverb.h - FreeVerb (Jezar at Dreampoint, public domain) reference implementation
 * Parameter layout / semantics identical to BASS_FX's BASS_BFX_FREEVERB, which is what
 * Salt Player (com.salt.music) uses for "MAX Audio 音乐厅氛围" (Concert Hall Atmosphere).
 *
 * Extracted from base(1).apk -> classes.dex
 *   Landroidx/media3/re3;->ޡ(Z)V   (setConcertHallAtmosphere)
 * The app calls it with:
 *   BASS_ChannelSetFX(stream, 0x10016 (BASS_FX_BFX_FREEVERB), 0)
 *   fDryMix = 0.85f, fWetMix = 0.75f, fRoomSize = 0.9f, fDamp = 0.4f,
 *   fWidth  = 0.75f, lMode = 0, lChannel = -1 (BASS_BFX_CHANALL)
 */
#ifndef FREEVERB_H
#define FREEVERB_H

#ifdef __cplusplus
extern "C" {
#endif

#define FV_NUMCOMBS   8
#define FV_NUMALLPASS 4
#define FV_STEREO_SPREAD 23

/* 缓冲区长度上限：必须覆盖最高支持采样率下的最长延迟。
 * 最长梳状延迟 1617 样本 @44.1kHz -> 1617*192000/44100 = 7041，取 8192。
 * 最长全通   556 样本 @44.1kHz ->  556*192000/44100 = 2421，取 4096。
 * 低于此值会在高采样率下把延迟截断（听感上混响变短、能量堆积甚至削波）。 */
/* Offset adjacent delay lines by one cache line; power-of-two strides cause
 * all 16 comb heads to compete for the same X2000 L1 cache sets. */
#define FV_MAXCOMB    32784
#define FV_MAXALLPASS 16400

/* BASS_BFX_FREEVERB channel mask (BASS_FX) */
#define BASS_BFX_CHAN1   1
#define BASS_BFX_CHAN2   2
#define BASS_BFX_CHAN3   4
#define BASS_BFX_CHAN4   8
#define BASS_BFX_CHAN5   16
#define BASS_BFX_CHAN6   32
#define BASS_BFX_CHAN7   64
#define BASS_BFX_CHAN8   128
#define BASS_BFX_CHANALL 255
#define BASS_BFX_CHANNONE 0

#define BASS_BFX_FREEVERB_MODE_FREEZE 1

/* Mirror of BASS_FX$BASS_BFX_FREEVERB (field names taken from the DEX) */
typedef struct {
    float fDryMix;   /* 0 .. 2  dry (unaffected) signal mix                     */
    float fWetMix;   /* 0 .. 2  wet (affected)   signal mix                     */
    float fRoomSize; /* 0 .. 1  room size                                       */
    float fDamp;     /* 0 .. 1  damping                                         */
    int   lMode;     /* 0 = normal, BASS_BFX_FREEVERB_MODE_FREEZE = 1           */
    float fWidth;    /* 0 .. 1  stereo width                                    */
    int   lChannel;  /* BASS_BFX_CHANxxx mask                                   */
} BASS_BFX_FREEVERB;

typedef struct {
    float bufL[FV_NUMCOMBS][FV_MAXCOMB];
    float bufR[FV_NUMCOMBS][FV_MAXCOMB];
    int   len[FV_NUMCOMBS];
    int   idxL[FV_NUMCOMBS];
    int   idxR[FV_NUMCOMBS];
    float filtL[FV_NUMCOMBS];
    float filtR[FV_NUMCOMBS];

    float abufL[FV_NUMALLPASS][FV_MAXALLPASS];
    float abufR[FV_NUMALLPASS][FV_MAXALLPASS];
    int   alen[FV_NUMALLPASS];
    int   aidxL[FV_NUMALLPASS];
    int   aidxR[FV_NUMALLPASS];

    /* derived coefficients */
    float gain;       /* internal fixedgain (0.015)             */
    float roomsize1;  /* room   * scaleroom + offsetroom        */
    float damp1;      /* damp   * scaledamp                     */
    float damp2;      /* 1 - damp1                              */
    float wet1, wet2; /* width-processed wet coefficients       */
    float wet;        /* fWetMix * scalewet                     */
    float dry;        /* fDryMix * scaledry                     */
    int   freeze;
    int   sampleRate;
    BASS_BFX_FREEVERB p;
} freeverb_t;

/* 注意：freeverb_t 约 0.8 MB（含缓冲区），**请用 malloc/静态分配，不要放在栈上**
 *（Windows 默认栈只有 1 MB，和其他大对象放一起会溢出）。 */

/* Initialise for a given sample rate (FreeVerb tunings are defined at 44100 Hz and
 * are scaled proportionally for other rates) and apply the parameter block. */
void freeverb_init(freeverb_t *fv, int sampleRate);
void freeverb_set_params(freeverb_t *fv, const BASS_BFX_FREEVERB *p);
void freeverb_mute(freeverb_t *fv);

/* Process one interleaved stereo block in place. channel mask is honoured. */
void freeverb_process(freeverb_t *fv, float *interleavedStereo, int frames);

#ifdef __cplusplus
}
#endif
#endif /* FREEVERB_H */
