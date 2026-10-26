/* Own only the selected native display timer's period. Touch polling is a
 * separate stock timer. Home needs30Hz for its25Hz bars; scopes/shade use60Hz. */
static unsigned frame_owned_period;
static void frameperiod(unsigned desired){
 void *disp=lv_disp_default();void *timer=disp?*(void **)((char *)disp+4):0;fast_ui=0;
 if(!timer){fast_timer=0;frame_owned_period=0;return;}
 unsigned period=*(unsigned *)timer;
 if(desired){
  if(timer!=fast_timer){if(!period||period>1000)return;fast_timer=timer;normal_period=period;frame_owned_period=0;}
  /* An unrelated timer owner may change the period: do not override it. */
  if(period==normal_period||period==frame_owned_period){
   if(period!=desired)FN(0x525420,void (*)(void *,unsigned))(timer,desired);
   frame_owned_period=desired;fast_ui=desired==16;
  }
 }else{
  if(timer==fast_timer&&period==frame_owned_period&&period!=normal_period)FN(0x525420,void (*)(void *,unsigned))(timer,normal_period);
  fast_timer=0;frame_owned_period=0;
 }
}
