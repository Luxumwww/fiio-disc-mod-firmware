/* Native shade EQ action hard-codes settings as its return destination.
 * Keep its click/long-press behavior; override only the opened page's return. */
static LvObj *pure_eq_origin;
static void pure_eq_origin_deleted(void *e){if(lv_event_target(e)==pure_eq_origin)pure_eq_origin=0;}
static void pure_eq_watch(LvObj *o){
 unsigned *s=*(unsigned **)((char *)o+8);if(s){unsigned n=s[8]>>16,*events=(unsigned *)s[3];
  if(n<=128)for(unsigned i=0;events&&i<n;i++)if(events[i*3]==(unsigned)pure_eq_origin_deleted&&(events[i*3+2]&255)==33)return;
 }
 lv_add_cb(o,pure_eq_origin_deleted,33,0);
}
static void pure_eq_return(void){
 LvObj *origin=pure_eq_origin;pure_eq_origin=0;
 if(*(unsigned *)0x8e16d4==0){
  if(origin&&lv_valid(origin)){FN(0x4e2734,void (*)(LvObj *))(origin);disc_toolbar_restore(&eq_toolbar_status);}
  else FN(0x435bf0,void (*)(void))();
 }else eq_return();
 eq_toolbar_status.bar=0;
}
static void pure_eq_click(void *e){
 if(pure_editing)return;
 LvObj *origin=lv_active_screen(lv_disp_default());
 DiscToolbarStatus saved=disc_toolbar_capture();
 FN(0x480228,void (*)(void *))(e);
 LvObj *eq=LV_GLOBAL(0x8dd9f0);
 if(*(unsigned *)0x8e16d4==0&&origin&&origin!=eq&&eq&&lv_valid(eq)&&lv_active_screen(lv_disp_default())==eq){
  pure_eq_origin=origin;pure_eq_watch(origin);eq_toolbar_status=saved;
  /* Existing native page: constructor updates the callback without rebuilding
   * EQ controls or changing the selected preset. */
  FN(0x4670ac,void (*)(void (*)(void)))(pure_eq_return);
 }
}
static void pure_eq_bind(LvObj *grid){
 if(pure_editing)return;LvObj *button=LV_GLOBAL(0x8deed0);
 if(!button||!lv_valid(button)||parent(button)!=grid)return;
 unsigned *s=*(unsigned **)((char *)button+8);if(!s)return;unsigned n=s[8]>>16,*events=(unsigned *)s[3];
 if(n<=32)for(unsigned i=0;events&&i<n;i++)if(events[i*3]==0x480228&&(events[i*3+2]&255)==7)events[i*3]=(unsigned)pure_eq_click;
}
