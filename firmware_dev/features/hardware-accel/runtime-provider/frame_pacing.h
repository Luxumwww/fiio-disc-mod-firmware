/* Absolute 60Hz submission slots. Rendering jitter must not discard a whole
 * frame just because two independent relative16ms gates disagree. The driver
 * still waits for vblank; there is no spinning or backlog replay. */
typedef struct {unsigned next,last;int valid;} DiscFramePacer;
#define DISC_FRAME_US 16667u
static int disc_frame_due(const DiscFramePacer *p,unsigned now){
 return !p->valid||((unsigned)(now-p->last)>=DISC_FRAME_US/2&&
                     (int)(now-p->next)>=-(int)(DISC_FRAME_US/2));
}
static void disc_frame_commit(DiscFramePacer *p,unsigned now){
 if(!p->valid){p->next=now+DISC_FRAME_US;p->valid=1;}
 else if((int)(now-p->next)>=0)p->next+=((now-p->next)/DISC_FRAME_US+1)*DISC_FRAME_US;
 else p->next+=DISC_FRAME_US; /* A native flush consumes the upcoming slot. */
 if((int)(p->next-now)>(int)(DISC_FRAME_US*2))p->next=now+DISC_FRAME_US;
 p->last=now;
}
