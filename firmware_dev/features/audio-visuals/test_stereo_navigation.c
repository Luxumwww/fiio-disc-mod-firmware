#include <assert.h>
#include <stdio.h>
#include "page_navigation.h"
static DiscPageNavigationResult e(DiscPageNavigation *s,unsigned code,int x,int y,unsigned t,unsigned kind,unsigned p){return page_navigation(s,code,x,y,t,kind,p,0,1,1,1,1,1);}
int main(void){
 DiscPageNavigation s={0};DiscPageNavigationResult r;
 e(&s,1,280,180,100,2,360);r=e(&s,2,160,180,180,2,360);assert(s.child&&r.kind==5&&r.motion.position==120&&(r.motion.flags&SWIPE_BEGIN));
 r=e(&s,8,130,180,250,5,360);assert(r.motion.target==360&&s.last_bottom==5&&s.last_left==0);
 e(&s,1,180,110,300,5,360);r=e(&s,8,180,260,500,5,360);assert(!s.child&&r.kind==5&&r.motion.target==0&&s.last_bottom==5);
 e(&s,1,180,280,600,0,0);r=e(&s,8,180,100,800,0,0);assert(r.kind==5&&r.motion.target==360);
 e(&s,1,80,180,900,5,360);r=e(&s,8,250,180,1100,5,360);assert(s.child&&r.kind==2&&r.motion.target==360&&s.last_bottom==2);
 e(&s,1,280,180,1200,2,360);r=e(&s,8,227,180,1600,2,360);assert(s.child&&r.motion.target==0&&s.last_bottom==2);
 e(&s,1,180,15,1700,5,360);r=e(&s,8,180,200,1900,5,360);assert(!s.pressed&&!r.motion.flags);
 e(&s,1,280,180,2000,2,360);r=e(&s,2,200,180,2100,2,360);assert(s.child&&r.kind==5);r=e(&s,8,200,310,2200,5,360);assert(s.child&&r.kind==5); /* axis stays horizontal */
 s=(DiscPageNavigation){.last_bottom=5};page_navigation(&s,1,180,280,2300,0,0,0,1,1,1,1,0);r=page_navigation(&s,8,180,100,2400,0,0,0,1,1,1,1,0);assert(r.kind==2);
 s=(DiscPageNavigation){0};page_navigation(&s,1,280,180,2500,2,360,0,1,1,1,1,0);r=page_navigation(&s,8,100,180,2600,2,360,0,1,1,1,1,0);assert(!s.child&&!r.motion.flags);
 puts("PASS polar/stereo horizontal child transition, down-home, remembered re-entry, canceled drag, native shade, locked axis and disabled-provider fallback");
}
