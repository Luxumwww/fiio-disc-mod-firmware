/* Mode-independent two-page LEFT-entry / RIGHT-return touch state. */
typedef struct {int x,y,direction;unsigned down,base;int pressed,dragging;} DiscSwipe;
typedef struct {unsigned flags,position,target;} DiscSwipeResult;
#define SWIPE_BEGIN 1u
#define SWIPE_MOVE 2u
#define SWIPE_SETTLE 4u
static DiscSwipeResult swipe_event(DiscSwipe *s,unsigned code,int x,int y,unsigned time,unsigned position,int animating){
 DiscSwipeResult r={0,position,0};
 if(code==1){if(s->pressed||animating)return r;*s=(DiscSwipe){x,y,0,time,position>=180?360u:0u,1,0};return r;}
 if(!s->pressed||animating||!(code==2||code==3||code==8))return r;
 int dx=x-s->x,dy=y-s->y;
 if(!s->dragging&&((dx<-40&&s->base==0)||(dx>40&&s->base==360))&&dx*dx>dy*dy*2){s->dragging=1;s->direction=dx<0?-1:1;r.flags|=SWIPE_BEGIN;}
 if(s->dragging){int p=(int)s->base-dx;if(p<0)p=0;if(p>360)p=360;r.position=(unsigned)p;r.flags|=SWIPE_MOVE;}
 if(code==3||code==8){if(s->dragging){int distance=dx<0?-dx:dx;int commit=distance>=85||(distance>=45&&time-s->down<220);r.target=commit?(s->direction<0?360u:0u):s->base;r.flags|=SWIPE_SETTLE;}s->pressed=s->dragging=0;}
 return r;
}
