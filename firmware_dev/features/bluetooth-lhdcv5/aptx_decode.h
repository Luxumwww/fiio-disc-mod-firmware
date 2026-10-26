#include "third_party/freeaptx/freeaptx.h"
struct aptx_receiver {
 struct aptx_context *decoder;
 unsigned hd,have_sequence,ssrc;uint16_t sequence;
 uint64_t packets,frames,malformed,gaps,duplicates,dropped;
};
static uint32_t aptx_be32(const uint8_t *p){return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|p[2]<<8|p[3];}
static int aptx_receive(struct aptx_receiver *r,const uint8_t *p,size_t n,
 int (*output)(void *,const int32_t *,size_t),void *user){
 r->packets++;
 if(r->hd){
  if(n<12||(p[0]>>6)!=2)goto malformed;
  size_t header=12+(p[0]&15)*4,end=n;
  if(header>n)goto malformed;
  if(p[0]&0x10){
   if(n-header<4)goto malformed;
   size_t extension=4+(((unsigned)p[header+2]<<8)|p[header+3])*4;
   if(extension>n-header)goto malformed;header+=extension;
  }
  if(p[0]&0x20){unsigned pad=p[n-1];if(!pad||pad>end-header)goto malformed;end-=pad;}
  if(end==header||(end-header)%6)goto malformed;
  unsigned ssrc=aptx_be32(p+8);uint16_t seq=(uint16_t)((unsigned)p[2]<<8|p[3]);
  if(r->have_sequence&&ssrc==r->ssrc){
   unsigned delta=(uint16_t)(seq-r->sequence);
   if(!delta||delta>=0x8000){r->duplicates++;return 0;}
   if(delta>1)r->gaps+=delta-1;
  }else if(r->have_sequence)aptx_reset(r->decoder);
  r->ssrc=ssrc;r->sequence=seq;r->have_sequence=1;p+=header;n=end-header;
 }else if(!n||n%4)goto malformed;
 while(n){
  size_t chunk=n,block=r->hd?6:4;if(chunk>128*block)chunk=128*block;
  uint8_t packed[128*24+24];int32_t pcm[(128+1)*8];
  size_t written=0,dropped=0;int synced=0;
  size_t used=aptx_decode_sync(r->decoder,p,chunk,packed,sizeof(packed),&written,&synced,&dropped);
  (void)synced;r->dropped+=dropped;
  if(!used||used>chunk||written>sizeof(packed)||written%6)goto malformed;
  for(size_t i=0;i<written/3;i++){
   /* Unsigned assembly aligns signed 24-bit output to the S32 container,
    * avoiding undefined left shift of a negative signed sample. */
   const uint8_t *b=packed+i*3;
   pcm[i]=(int32_t)(((uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16)<<8);
  }
  if(written&&output(user,pcm,written/6))return -1;
  r->frames+=written/6;p+=used;n-=used;
 }
 return 0;
malformed:r->malformed++;errno=EINVAL;return -1;
}
static void aptx_close_receiver(void *arg){
 struct aptx_receiver *r=arg;aptx_finish(r->decoder);r->decoder=NULL;
}
static void *aptx_decode_thread(void *th){
 pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
 pthread_cleanup_push(FN(0x40d13c,void (*)(void *)),th);
 void *t=*(void **)th;struct aptx_receiver r={.hd=U16(t,0x1e)==APTX_HD_ID};
 uint8_t io[56] __attribute__((aligned(8)))={0};U32(io,48)=UINT32_MAX;
 r.decoder=aptx_init(r.hd);
 if(r.decoder){
  pthread_cleanup_push(aptx_close_receiver,&r);
  uint8_t packet[8192];void *pcm=(uint8_t *)t+0x1c0;
  FN(0x40b660,int (*)(void *,unsigned,int))(th,2,0);
  for(;;){
   ssize_t n=FN(0x414660,ssize_t (*)(void *,void *,void *,size_t))(io,th,packet,sizeof(packet));
   if(n<=0)break;
   if(!FN(0x40c14c,int (*)(void *))(pcm)){aptx_reset(r.decoder);r.have_sequence=0;continue;}
   int status=aptx_receive(&r,packet,(size_t)n,emit,pcm);
   if(status&&errno!=EINVAL)break;
  }
  FN(0x40b660,int (*)(void *,unsigned,int))(th,3,0);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_cleanup_pop(1);
 }
 pthread_cleanup_pop(1);return NULL;
}
static int aptx_start(void *t){
 pthread_t main_thread=(pthread_t)WORD(0x43352c);
 if(!pthread_equal((pthread_t)U32(t,0x88),main_thread)||!pthread_equal((pthread_t)U32(t,0x108),main_thread)){errno=EEXIST;return -1;}
 if(!(U16(t,0x1c)&2)){errno=ENOTSUP;return -1;}
 return FN(0x40cddc,int (*)(void *,void *(*)(void *),const char *,int))((uint8_t *)t+0xd0,aptx_decode_thread,U16(t,0x1e)==APTX_HD_ID?"ba-a2dp-aptx-hd":"ba-a2dp-aptx",1);
}
