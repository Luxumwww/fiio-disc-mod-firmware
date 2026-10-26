#include <assert.h>
#include <stdio.h>
#include "frame_pacing.h"
int main(void){
 DiscFramePacer p={0};unsigned old=0,old_count=0,new_count=0;
 for(unsigned i=0;i<600;i++){
  unsigned t=100000+i*16667+(i%2?2000:8000);
  if(!old_count||t-old>=16000){old=t;old_count++;}
  if(disc_frame_due(&p,t)){disc_frame_commit(&p,t);new_count++;}
  assert(!disc_frame_due(&p,t)&&!disc_frame_due(&p,t+1000));
 }
 assert(new_count>=599&&old_count<400);
 unsigned late=p.last+1000000;assert(disc_frame_due(&p,late));disc_frame_commit(&p,late);assert(!disc_frame_due(&p,late+1000));
 p=(DiscFramePacer){0};disc_frame_commit(&p,0xfffff000u);assert(disc_frame_due(&p,0xfffff000u+16667u));disc_frame_commit(&p,0xfffff000u+16667u);assert(!disc_frame_due(&p,p.last));
 p=(DiscFramePacer){0};disc_frame_commit(&p,100000);disc_frame_commit(&p,110000);assert(!disc_frame_due(&p,111000));
 printf("PASS jitter: relative gate %u/600, phase pacer %u/600; duplicate/native flush/stall/wrap guards\n",old_count,new_count);return 0;
}
