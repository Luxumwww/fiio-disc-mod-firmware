#ifndef DISC_DLNA_AUDIO_FORMAT_H
#define DISC_DLNA_AUDIO_FORMAT_H
/* Native decoder output, never a claim about the remote file's source bits. */
static int disc_dlna_format(const unsigned *h,unsigned mode,unsigned *rate,unsigned *bits,unsigned *channels){
 if(mode!=5||h[6]!=0||h[11]==2||h[3]<8000||h[3]>768000||(h[4]!=1&&h[4]!=2)||(h[5]!=16&&h[5]!=24&&h[5]!=32))return 0;
 *rate=h[3];*channels=h[4];*bits=h[5];return 1;
}
#endif
