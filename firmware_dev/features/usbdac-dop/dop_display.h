/* UI thread only; source classification, never classify a track by rate alone. */
static LvObj *dop_display_protocol,*dop_display_rate;
static unsigned dop_display_time,dop_display_last_rate;
static int dop_display_owned;
#ifndef DISC_DOP_DISPLAY_TEXT
/* Verified stock label_set_text stores its string at object + 0x2c. */
#define DISC_DOP_DISPLAY_TEXT(o) (*(const char **)((char *)(o)+0x2c))
#endif
static unsigned uac_display_player,uac_display_lo,uac_display_hi,uac_display_advance,uac_display_repairs;
static int uac_display_flow(unsigned p,unsigned mode,unsigned digital,unsigned t){
 unsigned ring[4];if(mode!=4||digital!=1||!copy(p+0x130,ring,sizeof(ring))){uac_display_player=0;uac_display_advance=0;return 0;}
 if(uac_display_player==p&&(ring[2]!=uac_display_lo||ring[3]!=uac_display_hi))uac_display_advance=t;
 else if(uac_display_player!=p)uac_display_advance=0;
 uac_display_player=p;uac_display_lo=ring[2];uac_display_hi=ring[3];return uac_display_advance&&t-uac_display_advance<=1000;
}
static void dop_display_poll(unsigned t){
 if(t-dop_display_time<500)return;dop_display_time=t;
 if(*(unsigned *)0x8e16d4!=1){dop_display_protocol=dop_display_rate=0;dop_display_owned=0;uac_display_player=uac_display_advance=0;return;}
 LvObj *protocol=LV_GLOBAL(0x8de210),*label=LV_GLOBAL(0x8de20c);
 if(!protocol||!label||!lv_valid(protocol)||!lv_valid(label)){dop_display_protocol=dop_display_rate=0;dop_display_owned=0;return;}
 unsigned p,h[40],digital,mp,mode;discover(t);
 if(!copy(0x83dfc4,&p,4)||!copy(p,h,sizeof(h))||!copy(0x839a74,&digital,4)||!copy(0x83dfc0,&mp,4)||!copy(mp,&mode,4))return;
 int flowing=uac_display_flow(p,mode,digital,t);const char *current=DISC_DOP_DISPLAY_TEXT(protocol);int stale=current&&!strcmp(current,"未连接");
 char capability[24];int on=access("/usr/data/disc-runtime/components/disc-usbdac-dop/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0)&&textread("/usr/data/disc-runtime/components/disc-usbdac-dop/current/FEATURES",capability,sizeof(capability))&&(!strcmp(capability,"usbdac-dop-abi3\n")||!strcmp(capability,"usbdac-stock-abi1\n")||!strcmp(capability,"usbdac-serial-abi1\n"));
 int dop=on&&mode==4&&digital==1&&h[6]==0&&h[11]==2&&h[22]==1&&h[4]==2&&h[5]==32&&(h[3]==176400||h[3]==352800);
 if(dop){
  if(!dop_display_owned||dop_display_protocol!=protocol||dop_display_rate!=label||dop_display_last_rate!=h[3]||stale){ensurefonts();smallfont(label,font22);statictext(label);textalign(label,2,0);lv_size(label,240,30);lv_align(label,2,0,106);lv_text(protocol,h[3]==352800?"DSD128":"DSD64");lv_text(label,h[3]==352800?"DoP / 5.64 MHz":"DoP / 2.82 MHz");dop_display_protocol=protocol;dop_display_rate=label;dop_display_last_rate=h[3];dop_display_owned=1;}
 }else if(dop_display_owned){
  /* Reuse the stock connection/rate handler to restore PCM/disconnected. */
  if(dop_display_protocol==protocol&&dop_display_rate==label)FN(0x475f90,void (*)(unsigned))(mode==4&&digital==1?h[3]/1000:0);
  dop_display_owned=0;
 }else if(stale&&flowing&&h[6]==0&&h[11]!=2&&h[3]>=8000&&h[3]<=768000&&(h[4]==1||h[4]==2)&&(h[5]==16||h[5]==24||h[5]==32)){
  // Recover a missed GUI event using fresh input-ring progress, never a stale rate.
  FN(0x475f90,void (*)(unsigned))(h[3]/1000);uac_display_repairs++;
 }
}
