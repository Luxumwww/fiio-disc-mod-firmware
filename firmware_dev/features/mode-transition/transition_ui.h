/* V2.57 work-mode wait popup: 46b7a0 creates global 8ddf44 on the top
 * layer, with one GIF (class7c2050) and one localized label (class7ffd00).
 * Restyle before native display refresh, so no white frame is submitted.
 * Original wait timer46b570, text/font, input flags and lifetime stay native. */
static LvObj *mt_wait;
static void mt_deleted(void *e){if(lv_event_target(e)==mt_wait)mt_wait=0;}
static void mode_transition_prepare(void){
 LvObj *root=LV_GLOBAL(0x8ddf44);if(!root||root==mt_wait||!lv_valid(root)||child_count(root)!=2)return;
 LvObj *gif=0,*label=0;for(unsigned i=0;i<2;i++){
  LvObj *o=child(root,(int)i);if(!o||!lv_valid(o))return;unsigned klass=*(unsigned *)o;
  if(klass==0x7c2050u&&!gif)gif=o;else if(klass==0x7ffd00u&&!label)label=o;else return;
 }
 if(!gif||!label)return;
 /* Verified GIF timer at +54; pause, never delete it. Native GIF destructor
  * continues to own timer/resource cleanup when the popup is removed. */
 void *timer=*(void **)((char *)gif+0x54);if(timer)FN(0x5253fc,void (*)(void *))(timer);
 lv_add_flag(gif,DISC_LV_HIDDEN);lv_bg(root,0xff000000u,0);bg_opa(root,255,0);
 lv_color(label,0xffa8a8a8u,0);lv_align(label,9,0,0);
 lv_add_cb(root,mt_deleted,DISC_LV_DELETE,0);mt_wait=root;
}
