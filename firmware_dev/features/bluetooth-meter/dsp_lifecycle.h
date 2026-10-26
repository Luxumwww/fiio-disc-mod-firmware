/* Native back can replace the active screen without our swipe completion.
 * Own references must be cleared independently of native delete callbacks. */
static void dsp_forget_page(void){
 src_ui_close(0);
 dsp_page=dsp_origin=dsp_message=0;dsp_drag=-1;dsp_back_pending=dsp_swiping=0;
 dsp_presets_forget();
 dsp_toolbar_status.bar=0;
 control_target=0;control_moved=1;
 for(unsigned i=0;i<16;i++)dsp_rows[i]=dsp_pills[i]=dsp_knobs[i]=0;
}
static void dsp_close_page(int return_origin){
 LvObj *old=dsp_page,*origin=dsp_origin;DiscToolbarStatus saved=dsp_toolbar_status;dsp_forget_page();
 if(return_origin){if(origin&&lv_valid(origin)){FN(0x4e2734,void (*)(LvObj *))(origin);disc_toolbar_restore(&saved);}else eq_return();}
 else if(origin&&lv_valid(origin)&&lv_active_screen(lv_disp_default())==origin)disc_toolbar_restore(&saved);
 if(old&&lv_valid(old))FN(0x4f7e98,void (*)(LvObj **))(&old);
}
static void dsp_cleanup_inactive(void){
 if(!dsp_page||src_ui_active())return;
 if(!lv_valid(dsp_page)){dsp_forget_page();return;}
 void *display=lv_disp_default();LvObj *pending=display?*(LvObj **)((char *)display+0x18):0;
 if(!dsp_swiping&&!dsp_back_pending&&pending!=dsp_page&&lv_active_screen(display)!=dsp_page)dsp_close_page(0);
}
