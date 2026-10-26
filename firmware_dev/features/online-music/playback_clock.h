/* Millisecond PCM samples with bounded, display-only interpolation.
 * Repeated IPC reads never restart the anchor; pause/track changes do.
 * Stop extrapolating after 1.25s if the receiver/status worker stalls. */
typedef struct { unsigned revision, sample, at, base; int valid, playing; } OnlineClock;
static void online_clock_accept(OnlineClock *c,unsigned revision,unsigned sample,int playing,unsigned at){
 if(!c->valid||c->revision!=revision||c->sample!=sample||c->playing!=playing){
  unsigned base=sample,elapsed=at-c->at;if(elapsed>1250)elapsed=1250;
  unsigned previous=c->base+(c->playing?elapsed:0);
  /* Small delivery jitter must not briefly scroll the lyrics backwards.
   * Larger jumps, pauses and new tracks always accept the actual position. */
  if(c->valid&&c->revision==revision&&c->playing&&playing&&sample>=c->sample&&previous>sample&&previous-sample<500)base=previous;
  c->revision=revision;c->sample=sample;c->at=at;c->base=base;c->playing=playing;c->valid=1;
 }
}
static unsigned online_clock_position(const OnlineClock *c,unsigned at,unsigned duration){
 unsigned pos=c->base,elapsed=at-c->at;
 if(c->valid&&c->playing){if(elapsed>1250)elapsed=1250;pos+=elapsed;}
 if(duration&&pos>duration*1000u)pos=duration*1000u;
 return pos;
}
