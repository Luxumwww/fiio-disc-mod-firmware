#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "ring.h"
/* One targeted reproduction: native 3840-frame blocks at 44.1k, independent
 * producer cadence and 16ms display clock, occasional 24ms publication jitter.
 * No audio/DSP/renderer or hardware interaction. */
static int old_clock(DiscVisualClock *c,const DiscVisualWindow *w,unsigned frames,unsigned t,unsigned *end){
 if(w->valid<frames)return 0;unsigned delay=w->max_frames+w->rate/500;
 if(frames>=DISC_VS_FRAMES)return 0;if(delay>DISC_VS_FRAMES-frames)delay=DISC_VS_FRAMES-frames;
 if(!c->ready||c->request!=w->request||c->rate!=w->rate||t-c->stamp>250){
  *c=(DiscVisualClock){.request=w->request,.rate=w->rate,.stamp=t,.delay_frames=delay};
  if(w->valid<frames+delay)return 0;c->end=w->total-delay;c->ready=1;
 }else{
  unsigned elapsed=t-c->stamp,advance=(elapsed*w->rate+c->fraction)/1000;
  unsigned remainder=(elapsed*w->rate+c->fraction)%1000,next=c->end+advance,offset=w->total-next;
  if(offset>w->valid-frames){if((int)offset<0)return 0;c->ready=0;return 0;}
  c->end=next;c->stamp=t;c->fraction=remainder;c->delay_frames=offset;
 }
 *end=c->end;return 1;
}
int main(void){
 unsigned stable_hash[2]={0};
 for(unsigned adaptive=0;adaptive<2;adaptive++)for(unsigned jitter=0;jitter<=24;jitter+=24){
  DiscVisualClock clock={0};DiscVisualWindow w={.rate=44100,.mode=4,.request=3,.max_frames=3840,.last_frames=3840};
  unsigned block=0,feeds=0,misses=0,consecutive=0,longest=0,previous=0,jumps=0;
  for(unsigned t=0;t<6000;t++){
   unsigned due=(block+1)*3840u*1000u/44100u+((block%5==3)?jitter:0);
   if(t>=due){block++;w.total+=3840;w.valid=w.total>65536?65536:w.total;w.sequence+=2;w.blocks++;}
   if(t%16)continue;unsigned end;
   int fresh=adaptive?disc_vs_clock(&clock,&w,946,t,&end):old_clock(&clock,&w,946,t,&end);
   if(t<500){previous=fresh?end:0;continue;}
   if(!fresh){misses++;if(++consecutive>longest)longest=consecutive;continue;}
   if(previous&&end-previous>706)jumps++;
   consecutive=0;previous=end;feeds++;if(!jitter)stable_hash[adaptive]=stable_hash[adaptive]*16777619u+end;
  }
  printf("adaptive=%u jitter=%ums feeds=%u missed=%u longest_hold=%ums jumps=%u\n",adaptive,jitter,feeds,misses,longest*16,jumps);
  if(!jitter)assert(feeds==343&&!misses&&!jumps);
  else if(adaptive)assert(feeds>=341&&misses<=2&&!jumps);else assert(misses>=10&&jumps>=10);
 }
 assert(stable_hash[0]==stable_hash[1]);
 /* A short capture reset keeps the UI request but restarts its total. The
  * old sample cursor must not wait for the new stream to reach its old count. */
 DiscVisualClock c={0};DiscVisualWindow w={.rate=44100,.request=3,.valid=65536,.max_frames=3840,.total=100000};unsigned end;
 assert(disc_vs_clock(&c,&w,946,1000,&end));w.total=w.valid=3840;assert(!disc_vs_clock(&c,&w,946,1016,&end)&&!c.ready);
 w.total=w.valid=7680;assert(disc_vs_clock(&c,&w,946,1032,&end)&&end==7680-3928);
 /* Both frame totals and monotonic millisecond clocks wrap naturally. */
 c=(DiscVisualClock){0};w.valid=65536;w.total=UINT_MAX-1000;unsigned t=UINT_MAX-10;
 assert(disc_vs_clock(&c,&w,946,t,&end));unsigned previous=end;
 for(unsigned i=0;i<12;i++){t+=16;w.total+=706;assert(disc_vs_clock(&c,&w,946,t,&end));unsigned d=end-previous;assert(d==705||d==706);previous=end;}
 puts("PASS production clock: stable stream unchanged; jitter no longer skips samples; short reset and counter/time wrap recover");
 return 0;
}
