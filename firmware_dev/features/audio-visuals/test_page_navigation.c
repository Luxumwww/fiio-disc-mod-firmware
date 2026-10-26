#include <assert.h>
#include <stdio.h>
#include "page_navigation.h"
static void press(DiscPageNavigation *s,int x,int y,unsigned page){page_navigation(s,1,x,y,100,page,page?360:0,0,1,1,1,1,1);}
int main(void){
 DiscPageNavigation s={0};DiscPageNavigationResult r;
 press(&s,180,280,0);r=page_navigation(&s,2,180,210,150,0,0,0,1,1,1,1,1);assert(r.kind==2&&r.motion.position==70&&(r.motion.flags&SWIPE_BEGIN));
 page_navigation(&s,1,180,210,151,2,70,0,1,1,1,1,1);assert(s.y==280&&s.swipe.base==0);
 r=page_navigation(&s,8,180,160,200,2,120,0,1,1,1,1,1);assert(r.motion.target==360&&(r.motion.flags&SWIPE_SETTLE));
 r=page_navigation(&s,8,180,160,201,2,120,0,1,1,1,1,1);assert(!r.motion.flags);
 press(&s,180,110,2);r=page_navigation(&s,2,180,230,150,2,360,0,1,1,1,1,1);assert(r.kind==2&&r.motion.position==240);r=page_navigation(&s,8,180,230,200,2,240,0,1,1,1,1,1);assert(r.motion.target==0);
 press(&s,180,20,2);r=page_navigation(&s,2,180,150,150,2,360,0,1,1,1,1,1);assert(!r.motion.flags&&!s.pressed); /* native top shade */
 press(&s,180,280,0);r=page_navigation(&s,8,180,224,500,0,0,0,1,1,1,1,1);assert(r.motion.target==0&&(r.motion.flags&SWIPE_SETTLE)); /* cancel short */
 press(&s,280,180,0);r=page_navigation(&s,2,150,180,150,0,0,0,1,1,1,1,1);assert(r.kind==1&&r.motion.position==130);r=page_navigation(&s,8,150,180,200,1,130,0,1,1,1,1,1);assert(r.motion.target==360);
 press(&s,80,180,1);r=page_navigation(&s,8,240,180,200,1,360,0,1,1,1,1,1);assert(r.motion.target==0); /* old XY return */
 press(&s,180,280,0);r=page_navigation(&s,2,110,210,150,0,0,0,1,1,1,1,1);assert(!r.motion.flags);r=page_navigation(&s,8,110,210,200,0,0,0,1,1,1,1,1);assert(!r.motion.flags); /* diagonal */
 press(&s,180,120,0);r=page_navigation(&s,8,180,250,200,0,0,0,1,1,1,1,1);assert(!r.motion.flags); /* home down stays native */
 press(&s,180,280,0);r=page_navigation(&s,8,180,100,200,0,0,0,1,0,1,1,0);assert(!r.motion.flags); /* disabled bottom providers */
 press(&s,280,180,0);r=page_navigation(&s,2,180,180,150,0,0,0,1,1,1,1,1);assert(r.kind==1);r=page_navigation(&s,8,180,20,200,1,100,0,1,1,1,1,1);assert(r.kind==1); /* cannot switch axis midstroke */
 s=(DiscPageNavigation){0};r=page_navigation(&s,1,180,280,100,0,0,1,1,1,1,1,1);assert(!s.pressed&&!r.motion.flags);
 s=(DiscPageNavigation){0};press(&s,100,180,0);r=page_navigation(&s,2,220,180,150,0,0,0,1,1,1,1,1);assert(r.kind==3&&r.motion.position==120);r=page_navigation(&s,8,220,180,200,3,120,0,1,1,1,1,1);assert(r.motion.target==360);
 press(&s,270,180,3);r=page_navigation(&s,2,130,180,150,3,360,0,1,1,1,1,1);assert(r.kind==3&&r.motion.position==220);r=page_navigation(&s,8,130,180,200,3,220,0,1,1,1,1,1);assert(r.motion.target==0);
 press(&s,30,180,0);r=page_navigation(&s,8,210,180,200,0,0,0,1,1,1,1,1);assert(!r.motion.flags); /* stock left-edge exit priority */
 press(&s,100,180,0);r=page_navigation(&s,8,220,180,200,0,0,0,1,1,0,1,1);assert(!r.motion.flags); /* disabled spectrum */
 puts("PASS: spectrum right/left, native exit priority, polar up/down, XY left/right, locked axis, duplicate/bubbled events, short cancel, native top shade, diagonal rejection, disabled provider and animation gate");
}
