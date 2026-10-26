extern int fsync(int),unlink(const char *);
static void visual_source_toggle(void){
 int next=!visual_post,fd=open(DISC_VS_PREF ".new",1|0x100|0x200,0600),ok=0;
 if(fd>=0){char c=next?'1':'0';ok=write(fd,&c,1)==1&&!fsync(fd);if(close(fd))ok=0;if(ok)ok=!rename(DISC_VS_PREF ".new",DISC_VS_PREF);}
 visual_source_error=!ok;
 if(ok){
  vs_ui_update(now(),0,0);visual_post=next;
  lasttotal=lasttotalhi=lastadvance=lastsample=lastanimation=0;
  value[0]=value[1]=smooth[0]=smooth[1]=0;drawn[0]=drawn[1]=-1;dpu_fill[0]=dpu_fill[1]=0;
  xy_gain=(DiscVisualGain){0};dac_home_gain=(DiscVisualGain){0};disc_dac_home_spectrum_reset(&dac_home_spectrum);
  if(xy_api)xy_api->reset();if(polar_api)polar_api->reset();if(stereo_api)stereo_api->reset();if(spectrum_api)spectrum_api->reset();if(mandala_api)mandala_api->reset();
  /* The shade freezes this page until it is completely closed. Publishing
   * a null frame here uncovers the stock DAC page throughout that close.
   * Reset audio history, but immediately publish the renderer's cleared grid. */
  xy_transition_source=0;
  if(xy_progress&&page_api){const DiscVisualImage *cleared=page_api->compose(360,0);if(cleared)xy_frame=cleared;}
  else xy_frame=0;
  xy_frame_progress=xy_progress;xy_picture_stamp=(DiscImageStamp){0};for(unsigned i=0;i<3;i++)dpu_content[i]=0;
 }else unlink(DISC_VS_PREF ".new");
 refreshratebutton();dac_toggle_icons();
}
