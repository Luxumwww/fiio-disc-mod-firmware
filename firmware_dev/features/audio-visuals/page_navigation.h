#ifndef DISC_PAGE_NAVIGATION_H
#define DISC_PAGE_NAVIGATION_H
#include "xy/swipe.h"
#define DISC_PAGE_HOME 0u
#define DISC_PAGE_XY 1u
#define DISC_PAGE_POLAR 2u
#define DISC_PAGE_SPECTRUM 3u
#define DISC_PAGE_MANDALA 4u
#define DISC_PAGE_STEREO 5u
typedef struct {DiscSwipe swipe;int x,y,pressed,child;unsigned chosen,origin,last_left,last_bottom;} DiscPageNavigation;
typedef struct {DiscSwipeResult motion;unsigned kind;} DiscPageNavigationResult;
/* Touch points are logical display coordinates, already rotated by LVGL.
 * Top-edge downward touches stay with the native shade. One direction locks
 * one renderer until release; duplicate/bubbled events cannot change origin. */
static int page_bottom(unsigned kind){return kind==DISC_PAGE_POLAR||kind==DISC_PAGE_STEREO;}
static int page_swipe_x(unsigned kind,int x,int y){return page_bottom(kind)?y:kind==DISC_PAGE_SPECTRUM?-x:x;}
static int page_swipe_y(unsigned kind,int x,int y){return page_bottom(kind)?x:y;}
static DiscPageNavigationResult page_navigation(DiscPageNavigation *s,unsigned code,int x,int y,unsigned time,unsigned kind,unsigned position,int animating,int xy_available,int polar_available,int spectrum_available,int mandala_available,int stereo_available){
 DiscPageNavigationResult r={{0,position,0},kind};
 if(code==1){
  if(s->pressed||animating)return r;
  unsigned remembered=s->last_left,bottom=s->last_bottom;
  *s=(DiscPageNavigation){.x=x,.y=y,.pressed=1,.chosen=kind,.origin=kind,.last_left=remembered,.last_bottom=bottom};
  if(page_bottom(kind)&&y<60){s->pressed=0;return r;}
  if(kind&&kind!=DISC_PAGE_XY&&kind!=DISC_PAGE_MANDALA&&!page_bottom(kind))swipe_event(&s->swipe,1,page_swipe_x(kind,x,y),page_swipe_y(kind,x,y),time,position,0);
  else s->swipe.down=time;
  return r;
 }
 if(!s->pressed||animating)return r;
 if(page_bottom(s->origin)&&!s->swipe.pressed){
  int dx=x-s->x,dy=y-s->y;
  if(dy>40&&dy*dy>dx*dx*2)swipe_event(&s->swipe,1,s->y,s->x,s->swipe.down,360,0);
  else if(dx*dx>dy*dy*2&&((s->origin==DISC_PAGE_POLAR&&dx<-40&&stereo_available)||(s->origin==DISC_PAGE_STEREO&&dx>40&&polar_available))){
   s->child=1;s->chosen=s->origin==DISC_PAGE_POLAR?DISC_PAGE_STEREO:DISC_PAGE_POLAR;
   int sign=s->origin==DISC_PAGE_POLAR?1:-1;
   swipe_event(&s->swipe,1,sign*s->x,s->y,s->swipe.down,0,0);
  }
 }
 if((s->origin==DISC_PAGE_XY||s->origin==DISC_PAGE_MANDALA)&&!s->swipe.pressed){
  int dx=x-s->x,dy=y-s->y;
  if(dx>40&&dx*dx>dy*dy*2)swipe_event(&s->swipe,1,s->x,s->y,s->swipe.down,360,0);
  else if(s->y>=60&&dy*dy>dx*dx*2&&((s->origin==DISC_PAGE_XY&&dy>40&&mandala_available)||(s->origin==DISC_PAGE_MANDALA&&dy<-40&&xy_available))){
   s->child=1;s->chosen=s->origin==DISC_PAGE_XY?DISC_PAGE_MANDALA:DISC_PAGE_XY;
   int sign=s->origin==DISC_PAGE_XY?-1:1;
   swipe_event(&s->swipe,1,sign*s->y,s->x,s->swipe.down,0,0);
  }
 }
 if(!s->chosen){
  int dx=x-s->x,dy=y-s->y;
  if((xy_available||mandala_available)&&dx<-40&&dx*dx>dy*dy*2)s->chosen=s->last_left==DISC_PAGE_MANDALA&&mandala_available?DISC_PAGE_MANDALA:xy_available?DISC_PAGE_XY:DISC_PAGE_MANDALA;
  else if((polar_available||stereo_available)&&dy<-40&&dy*dy>dx*dx*2)s->chosen=s->last_bottom==DISC_PAGE_STEREO&&stereo_available?DISC_PAGE_STEREO:polar_available?DISC_PAGE_POLAR:DISC_PAGE_STEREO;
  else if(spectrum_available&&s->x>=60&&dx>40&&dx*dx>dy*dy*2)s->chosen=DISC_PAGE_SPECTRUM;
  if(s->chosen){unsigned down=s->swipe.down;swipe_event(&s->swipe,1,page_swipe_x(s->chosen,s->x,s->y),page_swipe_y(s->chosen,s->x,s->y),down,0,0);}
 }
 r.kind=s->chosen;
 if(s->chosen){
  int sx=s->child?(page_bottom(s->origin)?(s->origin==DISC_PAGE_POLAR?x:-x):(s->origin==DISC_PAGE_XY?-y:y)):page_swipe_x(s->chosen,x,y),sy=s->child?(page_bottom(s->origin)?y:x):page_swipe_y(s->chosen,x,y);
  r.motion=swipe_event(&s->swipe,code,sx,sy,time,s->child?0:position,0);
 }
 if((r.motion.flags&SWIPE_SETTLE)&&((s->child&&r.motion.target==360)||(!s->child&&r.motion.target==0))&&(s->chosen==DISC_PAGE_XY||s->chosen==DISC_PAGE_MANDALA))s->last_left=s->chosen;
 if((r.motion.flags&SWIPE_SETTLE)&&((s->child&&r.motion.target==360)||(!s->child&&r.motion.target==0))&&page_bottom(s->chosen))s->last_bottom=s->chosen;
 if(code==3||code==8)s->pressed=0;
 return r;
}
#endif
