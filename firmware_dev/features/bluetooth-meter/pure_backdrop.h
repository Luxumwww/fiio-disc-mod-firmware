/* Pure-mode shade and its frozen background share the native base framebuffer.
 * The opaque holder occludes expensive artwork/blur; a raw cached image draws
 * above it. Native top-layer controls clip/reveal that image in the SAME frame,
 * so this path needs neither a moving DPU mask nor overlay lease handoff. */
static LvObj *pure_backdrop,*pure_backdrop_parent,*pure_backdrop_image;
static unsigned pure_backdrop_pixels[129600],pure_backdrop_generation;
static struct {unsigned header,size;const void *data;} pure_backdrop_desc={5u|(360u<<10)|(360u<<21),518400,pure_backdrop_pixels};
static LvObj *pure_shade_owner;
static void pure_backdrop_deleted(void *e){
 if(lv_event_target(e)==pure_backdrop){pure_backdrop=pure_backdrop_parent=pure_backdrop_image=0;pure_backdrop_generation=0;}
}
static void pure_backdrop_prepare(LvObj *screen,int wanted){
 if(pure_backdrop&&pure_backdrop_parent!=screen){
  LvObj *old=pure_backdrop;pure_backdrop=pure_backdrop_parent=pure_backdrop_image=0;pure_backdrop_generation=0;
  if(lv_valid(old))FN(0x4f7e98,void (*)(LvObj **))(&old);
 }
 if(wanted&&!pure_backdrop){
  pure_backdrop=lv_create(screen);
  if(pure_backdrop){
   pure_backdrop_parent=screen;stylezero(pure_backdrop);lv_size(pure_backdrop,360,360);lv_align(pure_backdrop,1,0,0);
   lv_bg(pure_backdrop,0xff000000,0);opa(pure_backdrop,255,0);lv_add_cb(pure_backdrop,pure_backdrop_deleted,33,0);
  }
 }
 if(wanted&&pure_backdrop&&!pure_backdrop_image){
  pure_backdrop_image=img(pure_backdrop);
  if(pure_backdrop_image){stylezero(pure_backdrop_image);lv_size(pure_backdrop_image,360,360);lv_align(pure_backdrop_image,1,0,0);lv_clear_flag(pure_backdrop_image,2|16);pure_backdrop_generation=0;}
 }
 if(wanted&&pure_backdrop_image&&pure_backdrop_generation!=dpu_main_generation){
  /* Capture is physical, whereas LVGL rotates logical images during flush. */
  disc_rotate_frame(pure_backdrop_pixels,dpu_frozen_xy,(4u-dpu_main_rotation)&3u);
  src(pure_backdrop_image,&pure_backdrop_desc);pure_backdrop_generation=dpu_main_generation;
 }
 if(pure_backdrop)show(pure_backdrop,wanted&&pure_backdrop_image&&pure_backdrop_generation==dpu_main_generation);
}
