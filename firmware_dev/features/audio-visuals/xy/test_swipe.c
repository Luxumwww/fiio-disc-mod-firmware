#include <assert.h>
#include <stdio.h>
#include "swipe.h"
int main(void){DiscSwipe s={0};DiscSwipeResult r;
 swipe_event(&s,1,280,180,100,0,0);r=swipe_event(&s,2,212,180,400,0,0);assert(r.position==68&&s.dragging);
 swipe_event(&s,1,212,180,401,68,0);assert(s.x==280&&s.base==0);r=swipe_event(&s,8,212,180,500,68,0);assert((r.flags&SWIPE_SETTLE)&&r.target==0&&!s.pressed);
 r=swipe_event(&s,8,212,180,501,68,0);assert(!r.flags);
 swipe_event(&s,1,280,180,600,0,0);r=swipe_event(&s,2,150,180,700,0,0);assert(r.position==130);r=swipe_event(&s,8,150,180,800,130,0);assert(r.target==360);
 swipe_event(&s,1,80,180,900,360,0);r=swipe_event(&s,2,240,180,1000,360,0);assert(r.position==200);r=swipe_event(&s,8,240,180,1100,200,0);assert(r.target==0);
 swipe_event(&s,1,280,180,1200,0,0);r=swipe_event(&s,3,228,180,1250,0,0);assert((r.flags&SWIPE_SETTLE)&&r.target==360&&!s.pressed);
 swipe_event(&s,1,200,180,1300,0,0);r=swipe_event(&s,2,300,180,1400,0,0);assert(!r.flags);swipe_event(&s,8,300,180,1500,0,0);
 swipe_event(&s,1,200,80,1600,0,0);r=swipe_event(&s,2,140,240,1700,0,0);assert(!r.flags);swipe_event(&s,8,140,240,1800,0,0);
 swipe_event(&s,1,200,180,1900,68,0);r=swipe_event(&s,8,180,180,2200,68,0);assert(!r.flags);assert(s.base==0);
 puts("PASS: left entry, right return, duplicate press preserves origin, release loss settles, late release ignored, wrong/vertical gesture ignored");return 0;}
