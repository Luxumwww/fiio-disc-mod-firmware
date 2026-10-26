#include <assert.h>
#include <stdio.h>
#include "page_navigation.h"
static DiscPageNavigationResult event(DiscPageNavigation *s,unsigned code,int x,int y,unsigned time,unsigned kind,unsigned p){return page_navigation(s,code,x,y,time,kind,p,0,1,1,1,1,1);}
int main(void){
 DiscPageNavigation s={0};DiscPageNavigationResult r;
 event(&s,1,180,120,100,1,360);r=event(&s,2,180,240,150,1,360);assert(s.child&&r.kind==4&&r.motion.position==120&&(r.motion.flags&SWIPE_BEGIN));
 r=event(&s,8,180,250,220,4,360);assert(r.motion.target==360&&s.last_left==4);
 event(&s,1,100,180,300,4,360);r=event(&s,8,260,180,400,4,360);assert(!s.child&&r.kind==4&&r.motion.target==0&&s.last_left==4);
 event(&s,1,280,180,500,0,0);r=event(&s,8,140,180,600,0,0);assert(r.kind==4&&r.motion.target==360);
 event(&s,1,180,260,700,4,360);r=event(&s,8,180,100,800,4,360);assert(s.child&&r.kind==1&&r.motion.target==360&&s.last_left==1);
 event(&s,1,100,180,900,1,360);r=event(&s,8,250,180,1000,1,360);assert(r.motion.target==0);
 event(&s,1,280,180,1100,0,0);r=event(&s,8,140,180,1200,0,0);assert(r.kind==1&&r.motion.target==360);
 event(&s,1,180,120,1300,1,360);r=event(&s,8,180,172,1700,1,360);assert(s.child&&r.motion.target==0&&s.last_left==1);
 event(&s,1,180,20,1800,1,360);r=event(&s,8,180,160,1900,1,360);assert(!r.motion.flags&&!s.child);
 event(&s,1,100,180,2000,4,360);r=event(&s,2,170,180,2100,4,360);assert(!s.child);r=event(&s,8,170,50,2150,4,290);assert(r.kind==4&&!s.child);
 s=(DiscPageNavigation){.last_left=4};page_navigation(&s,1,280,180,2200,0,0,0,1,1,1,0,1);r=page_navigation(&s,8,120,180,2300,0,0,0,1,1,1,0,1);assert(r.kind==1);
 puts("PASS XY down/mandala up, both right-home, last-left re-entry, cancel, native shade, locked axis, unavailable fallback");
}
