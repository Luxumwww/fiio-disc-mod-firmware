/* DoP transport only: retain both DSD payload bytes and channel order.
 * The output marker clock belongs to this writer, including DSD silence.
 * No PCM conversion, bit reversal, gain, EQ or resampling. */
#ifndef DISC_DSD_PACKET_H
#define DISC_DSD_PACKET_H
typedef struct {unsigned next,input_next,input_known,rebases;} DiscDsdPacket;
static void dsde_packet_reset(DiscDsdPacket *s){s->next=5;s->input_next=s->input_known=s->rebases=0;}
static int dsde_packet_valid(const unsigned char *p,unsigned bytes){
 if(!p||bytes<8||(bytes&7))return 0;
 unsigned marker=p[3];if(marker!=5&&marker!=250)return 0;
 for(unsigned i=0;i<bytes;i+=8){
  if(p[i]||p[i+4]||p[i+3]!=marker||p[i+7]!=marker)return 0;
  marker^=255;
 }
 return 1;
}
/* Caller validates the complete source block before publishing any chunk.
 * Markers may restart at an input/song seam; the DSD payload stays unchanged
 * and the DAC sees a single uninterrupted marker sequence. */
static void dsde_packet_copy(DiscDsdPacket *s,const unsigned char *p,unsigned bytes,unsigned char *out,int silence){
 if(!silence){if(s->input_known&&s->input_next!=p[3])s->rebases++;s->input_known=1;}
 for(unsigned i=0;i<bytes;i+=8){
  out[i]=out[i+4]=0;
  out[i+1]=silence?0x69:p[i+1];out[i+2]=silence?0x69:p[i+2];
  out[i+5]=silence?0x69:p[i+5];out[i+6]=silence?0x69:p[i+6];
  out[i+3]=out[i+7]=(unsigned char)s->next;s->next^=255;
 }
 if(!silence)s->input_next=p[bytes-5]^255;
}
#endif
