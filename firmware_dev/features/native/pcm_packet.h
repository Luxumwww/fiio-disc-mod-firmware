#ifndef DISC_PCM_PACKET_H
#define DISC_PCM_PACKET_H
#define DISC_PCM_MAGIC 0x4450434dU
#define DISC_PCM_FRAMES 256U
#ifndef DISC_PCM_SOCKET
#define DISC_PCM_SOCKET "/tmp/disc-spectrum.pcm.sock"
#endif
typedef struct {
 unsigned magic,version,format,rate,channels,frames,position,epoch;
 unsigned char pcm[DISC_PCM_FRAMES*8];
} DiscPcmPacket;
/* Little-endian u32 fields; actual datagram size is 32+frames*channels*sample_bytes. */
#endif
