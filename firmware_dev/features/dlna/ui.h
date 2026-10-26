/* V2.57 contains its complete UPnP renderer. Use native input 12 (DLNA),
 * not input 9 (Roon). Share its existing network playback screen and back
 * animation; native mode switching owns decoder/output/service teardown. */
static LvObj *dlna_menu_row,*dlna_mode_label,*dlna_label_parent;
static int dlna_pending;
static int dlna_online_caption=-1;static unsigned dlna_caption_at;
static void dlna_enter_native(void){
 unsigned output=*(unsigned char *)0x8e1732?4u:6u;
 if(*(unsigned char *)0x8e1728!=output)
  FN(0x48b28c,void (*)(const char *,unsigned,const char *))((const char *)0x69dcf0,output,0);
 *(unsigned char *)0x8e1728=output;
 if(*(unsigned char *)0x8e1729!=12)
  FN(0x48b28c,void (*)(const char *,unsigned,const char *))((const char *)0x695540,12,0);
 *(unsigned char *)0x8e1729=12;
 /* The shared AirPlay/DLNA play screen is UI mode 5. PLAYER mode is 5 for
  * DLNA, 6 for AirPlay; do not confuse these separate enumerations. */
 *(unsigned *)0x8e16d4=5;
 FN(0x4777e0,void (*)(void))();
 *(unsigned *)0x8de250=1;
}
static void dlna_click(void *event){
 if(!stationary_clicked(event))return;
 unlink("/tmp/disc-online-music/active");
 if(FN(0x46f290,int (*)(void))()!=17){
  FN(0x46fdac,void (*)(void (*)(void)))(FN(0x474838,void (*)(void)));return;
 }
 dlna_pending=1;
}
static void dlna_row_delete(void *event){(void)event;dlna_menu_row=0;}
static void dlna_label_delete(void *event){(void)event;dlna_mode_label=0;dlna_label_parent=0;}
static void dlna_poll(void){
 if(dlna_pending){
  dlna_pending=0;
  if(*(unsigned char *)0x8e171c){
   /* Same native Bluetooth-exit confirmation and continuation as AirPlay. */
   FN(0x46b7a0,void (*)(void))();
   FN(0x459f20,void (*)(unsigned))(1);
   FN(0x459f14,void (*)(void (*)(void)))(dlna_enter_native);
   FN(0x45a940,void (*)(void))();
   FN(0x459f20,void (*)(unsigned))(0);
  }else dlna_enter_native();
 }
 LvObj *media=LV_GLOBAL(0x8de1b0);
 if(media&&lv_valid(media)&&!dlna_menu_row){
  unsigned before=child_count(media);
  original_row(media,"DLNA",0,dlna_click,0);
  if(child_count(media)>before){
   dlna_menu_row=child(media,-1);protect_tap(dlna_menu_row);
   lv_add_cb(dlna_menu_row,dlna_row_delete,DISC_LV_DELETE,0);
  }
 }
 LvObj *play=LV_GLOBAL(0x8de290);
 if(play&&lv_valid(play)&&dlna_label_parent!=play){
  dlna_mode_label=lv_label(play);dlna_label_parent=play;
  dlna_online_caption=-1;dlna_caption_at=0;
  lv_text(dlna_mode_label,"DLNA");
  lv_color(dlna_mode_label,0xffffffff,0);
  lv_add_style(dlna_mode_label,(void *)0x83ac5c,0);
  lv_align(dlna_mode_label,2,0,26);
  lv_clear_flag(dlna_mode_label,DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);
  lv_add_cb(dlna_mode_label,dlna_label_delete,DISC_LV_DELETE,0);
 }
 if(dlna_mode_label){
  unsigned at=milliseconds();if(dlna_online_caption<0||at-dlna_caption_at>=500){dlna_caption_at=at;int online_now=!access("/tmp/disc-online-music/active",0);if(dlna_online_caption!=online_now){lv_text(dlna_mode_label,online_now?"在线音乐":"DLNA");dlna_online_caption=online_now;}}
  if(*(unsigned char *)0x8e1729==12)lv_clear_flag(dlna_mode_label,DISC_LV_HIDDEN);
  else lv_add_flag(dlna_mode_label,DISC_LV_HIDDEN);
 }
}
