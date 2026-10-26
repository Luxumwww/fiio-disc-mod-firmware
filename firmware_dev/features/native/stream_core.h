#ifndef DISC_STREAM_CORE_H
#define DISC_STREAM_CORE_H
#include "spectrum_core.h"
#include "pcm_packet.h"
#define DISC_FIR_MAX 257
typedef struct {
 short history[2][DISC_FIR_MAX],window[2][DISC_FFT_SIZE];
 unsigned head,phase,decimation,fill,seen,writepos,rate,format,channels,epoch,next_position;
 unsigned marker_run,bad_markers,last_marker,dop,accepted;
 int db[2];
 DiscSpectrumCore spectrum[2];
 unsigned char levels[DISC_BANDS];
} DiscStream;
void disc_stream_reset(DiscStream *);
int disc_stream_packet(DiscStream *,const DiscPcmPacket *,unsigned bytes);
int disc_stream_analyze(DiscStream *);
#endif
