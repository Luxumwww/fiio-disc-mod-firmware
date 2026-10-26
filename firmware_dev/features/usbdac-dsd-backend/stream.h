/* Independent USB DoP framing. DSD bytes are opaque; no gain or resampler. */
#ifndef DISC_BACKEND_STREAM_H
#define DISC_BACKEND_STREAM_H
typedef struct {unsigned valid_run,expect,known,out_marker,seams,bad;} DsbStream;
static void dsb_reset(DsbStream *s){s->valid_run=s->known=s->seams=s->bad=0;s->expect=s->out_marker=5;}
static int dsb_scan(DsbStream *s,const unsigned char *p,unsigned bytes){
 if(!bytes||(bytes&7))return -1;
 unsigned first=p[3],marker=first;
 for(unsigned i=0;i<bytes;i+=8){
  if(p[i]||p[i+4]||(marker!=5&&marker!=250)||p[i+3]!=marker||p[i+7]!=marker){s->valid_run=0;s->known=0;s->bad++;return 0;}
  marker^=255;
 }
 if(s->known&&first!=s->expect){s->seams++;s->valid_run=0;}
 s->known=1;s->expect=marker;s->valid_run+=bytes/8;if(s->valid_run>32)s->valid_run=32;
 return s->valid_run>=32?2:1;
}
static void dsb_frame(DsbStream *s,const unsigned char *p,unsigned bytes,unsigned char *out){
 for(unsigned i=0;i<bytes;i+=8){
  out[i]=out[i+4]=0;out[i+1]=p?p[i+1]:0x69;out[i+2]=p?p[i+2]:0x69;
  out[i+5]=p?p[i+5]:0x69;out[i+6]=p?p[i+6]:0x69;
  out[i+3]=out[i+7]=(unsigned char)s->out_marker;s->out_marker^=255;
 }
}
static int dsb_zero(const unsigned char *p,unsigned n){for(unsigned i=0;i<n;i++)if(p[i])return 0;return 1;}
static int dsb_marked(const unsigned char *p,unsigned n){
 unsigned count=0;for(unsigned i=0;i+7<n;i+=8)if((p[i+3]==5||p[i+3]==250)&&p[i+7]==p[i+3])if(++count>=16)return 1;return 0;
}
#endif
