#ifndef DISC_SPECTRUM_VIEW_H
#define DISC_SPECTRUM_VIEW_H
typedef struct {unsigned rate,format,channels,dop,live;int db[2];unsigned char levels[72];} DiscView;
int disc_audio_start(void);
void disc_audio_snapshot(DiscView *);
void disc_audio_stop(void);
#endif
