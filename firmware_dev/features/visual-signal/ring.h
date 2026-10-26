/* Shared PCM observation only. Fixed 32-bit layout on host and MIPS o32.
 * One nonblocking writer; reader retries at most twice. No audio mutation. */
#ifndef DISC_VISUAL_RING_H
#define DISC_VISUAL_RING_H
#define DISC_VS_MAGIC 0x56535031u
#define DISC_VS_FRAMES 65536u
#define DISC_VS_PATH "/tmp/disc-runtime/visual-pcm"
#define DISC_VS_PREF "/usr/data/disc-runtime/module-data/disc-bt-meter/visual-signal-source"
typedef struct {
 unsigned magic,abi,owner,bytes;
 volatile unsigned request,lease,ui_mode; /* UI owns these three words. */
 volatile unsigned lock,sequence;
 unsigned request_seen,mode,route,rate,position,valid,stamp,blocks;
 unsigned total,last_frames,max_frames,reserved[12];
 int pcm[DISC_VS_FRAMES*2];
} DiscVisualRing;
typedef struct {unsigned sequence,rate,mode,request,blocks,total,valid,last_frames,max_frames;} DiscVisualWindow;
typedef char DiscVisualLayout[(sizeof(unsigned)==4&&sizeof(int)==4&&sizeof(DiscVisualRing)==128+DISC_VS_FRAMES*8)?1:-1];
static inline int disc_vs_requested(const DiscVisualRing *r,unsigned t,unsigned mode){
 unsigned lease=r->lease;__sync_synchronize();
 int age=(int)(t-lease);
 return (r->request&1u)&&r->ui_mode==mode&&(mode==2||mode==4)&&age>=-250&&age<1000;
}
static inline void disc_vs_invalidate(DiscVisualRing *r){
 if(__sync_lock_test_and_set(&r->lock,1))return;
 if(r->valid){r->sequence++;__sync_synchronize();r->valid=0;r->request_seen=0;__sync_synchronize();r->sequence++;}
 __sync_lock_release(&r->lock);
}
static inline void disc_vs_write(DiscVisualRing *r,const int *pcm,unsigned frames,unsigned rate,unsigned mode,unsigned route,unsigned t){
 if(!r||!pcm||!frames||rate<8000||rate>384000||!disc_vs_requested(r,t,mode))return;
 if(__sync_lock_test_and_set(&r->lock,1))return; /* Never wait on audio. */
 unsigned request=r->request;
 if(!disc_vs_requested(r,t,mode)){__sync_lock_release(&r->lock);return;}
 r->sequence++;__sync_synchronize();
 if(r->request_seen!=request||r->rate!=rate||r->mode!=mode||r->route!=route){r->position=r->valid=r->total=r->max_frames=0;}
 r->request_seen=request;r->rate=rate;r->mode=mode;r->route=route;
 if(frames>DISC_VS_FRAMES){pcm+=(frames-DISC_VS_FRAMES)*2;frames=DISC_VS_FRAMES;}
 unsigned first=DISC_VS_FRAMES-r->position;if(first>frames)first=frames;
 __builtin_memcpy(r->pcm+r->position*2,pcm,first*8);
 if(first<frames)__builtin_memcpy(r->pcm,pcm+first*2,(frames-first)*8);
 r->position=(r->position+frames)&(DISC_VS_FRAMES-1);
 r->valid=r->valid+frames>DISC_VS_FRAMES?DISC_VS_FRAMES:r->valid+frames;
 r->stamp=t;r->blocks++;r->total+=frames;r->last_frames=frames;if(frames>r->max_frames)r->max_frames=frames;
 /* A simultaneous source/visibility change makes this publication unusable. */
 if(r->request!=request)r->valid=0;
 __sync_synchronize();r->sequence++;__sync_lock_release(&r->lock);
}
static inline int disc_vs_meta(const DiscVisualRing *r,unsigned rate,unsigned mode,unsigned owner,unsigned t,DiscVisualWindow *out){
 if(!r)return 0;
 for(unsigned retry=0;retry<2;retry++){
  unsigned sequence=r->sequence;if(sequence&1u)continue;__sync_synchronize();
  unsigned request=r->request;
  if(r->magic!=DISC_VS_MAGIC||r->abi!=1||r->owner!=owner||r->bytes!=sizeof(*r)||!disc_vs_requested(r,t,mode)||r->request_seen!=request||r->mode!=mode||r->rate!=rate||!r->valid||(unsigned)(t-r->stamp)>300)return 0;
  DiscVisualWindow window={sequence,rate,mode,request,r->blocks,r->total,r->valid,r->last_frames,r->max_frames};
  __sync_synchronize();if(sequence==r->sequence&&request==r->request){*out=window;return 1;}
 }
 return 0;
}
static inline int disc_vs_read_window(const DiscVisualRing *r,int *dest,unsigned frames,unsigned rate,unsigned mode,unsigned owner,unsigned t,unsigned end,int tail,DiscVisualWindow *out){
 if(!r||!dest||!frames||frames>DISC_VS_FRAMES)return 0;
 for(unsigned retry=0;retry<2;retry++){
  unsigned sequence=r->sequence;if(sequence&1u)continue;__sync_synchronize();
  unsigned request=r->request,position=r->position;
  if(r->magic!=DISC_VS_MAGIC||r->abi!=1||r->owner!=owner||r->bytes!=sizeof(*r)||!disc_vs_requested(r,t,mode)||r->request_seen!=request||r->mode!=mode||r->rate!=rate||r->valid<frames||position>=DISC_VS_FRAMES||(unsigned)(t-r->stamp)>300)return 0;
  unsigned offset=tail?0:r->total-end;if(offset>r->valid-frames)return 0;
  DiscVisualWindow window={sequence,rate,mode,request,r->blocks,r->total,r->valid,r->last_frames,r->max_frames};
  unsigned start=(position+DISC_VS_FRAMES-frames-offset)&(DISC_VS_FRAMES-1),first=DISC_VS_FRAMES-start;if(first>frames)first=frames;
  __builtin_memcpy(dest,r->pcm+start*2,first*8);
  if(first<frames)__builtin_memcpy(dest+first*2,r->pcm,(frames-first)*8);
  __sync_synchronize();if(sequence==r->sequence&&request==r->request){*out=window;return 1;}
 }
 return 0;
}
static inline int disc_vs_read(const DiscVisualRing *r,int *dest,unsigned frames,unsigned rate,unsigned mode,unsigned owner,unsigned t,DiscVisualWindow *out){return disc_vs_read_window(r,dest,frames,rate,mode,owner,t,0,1,out);}
/* Play the captured block in chronological display windows, instead of
 * holding only its tail for an entire 40-90ms native audio block. */
typedef struct {unsigned request,rate,end,stamp,fraction,ready,delay_frames,total;} DiscVisualClock;
static inline int disc_vs_clock(DiscVisualClock *c,const DiscVisualWindow *w,unsigned frames,unsigned t,unsigned *end){
 if(w->valid<frames)return 0;
 unsigned delay=w->max_frames+w->rate/500; /* One native block plus 2ms. */
 if(frames>=DISC_VS_FRAMES)return 0;if(delay>DISC_VS_FRAMES-frames)delay=DISC_VS_FRAMES-frames;
 if(!c->ready||c->request!=w->request||c->rate!=w->rate||t-c->stamp>250||(int)(w->total-c->total)<0){
  *c=(DiscVisualClock){.request=w->request,.rate=w->rate,.stamp=t,.delay_frames=delay,.total=w->total};
  if(w->valid<frames+delay)return 0;
  c->end=w->total-delay;c->ready=1;
 }else{
  c->total=w->total;
  unsigned elapsed=t-c->stamp,advance=(elapsed*w->rate+c->fraction)/1000;
  unsigned remainder=(elapsed*w->rate+c->fraction)%1000,next=c->end+advance,offset=w->total-next;
  if(offset>w->valid-frames){
   /* A late native block must not accumulate elapsed wall time and then
    * skip that many samples when it arrives. Hold the sample cursor and
    * let its actual backlog absorb publication jitter; no fixed added delay. */
   if((int)offset<0){c->stamp=t;c->fraction=remainder;c->delay_frames=w->total-c->end;return 0;}
   c->ready=0;return 0;
  }
  c->end=next;c->stamp=t;c->fraction=remainder;c->delay_frames=offset;
 }
 *end=c->end;return 1;
}
#endif
