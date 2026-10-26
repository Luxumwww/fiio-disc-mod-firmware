/* Bluetooth HOST binding; renderer itself knows no vendor addresses/mode. */
extern void *memcpy(void *,const void *,unsigned),*memset(void *,int,unsigned);
static LvObj *xy_cover,*xy_picture,*xy_bound[11];
static unsigned xy_snapshot[129600],xy_started,xy_start_position,xy_target,xy_old_frame,xy_state_at;
static DiscPageNavigation page_navigation_state;
static int xy_animating,xy_dragging,xy_child_transition;
static unsigned xy_child_position,xy_child_from,xy_last_left[3]={DISC_PAGE_XY,DISC_PAGE_XY,DISC_PAGE_XY};
static unsigned xy_last_bottom[3]={DISC_PAGE_POLAR,DISC_PAGE_POLAR,DISC_PAGE_POLAR};
static unsigned xy_child_pixels[129600];
static unsigned xy_child_snapshot[129600];
static const unsigned *xy_transition_source;
#include "slide_compose.h"
static const DiscVisualImage xy_child_image={5u|(360u<<10)|(360u<<21),518400,xy_child_pixels};
static short xy_pcm[DISC_SCOPE_MAX_SAMPLES][2];
static DiscVisualGain xy_gain;
static unsigned xy_read_count;
static unsigned xy_composed,xy_render_max_ms;
static int xy_native_gesture;
#include "image_stamp.h"
static DiscImageStamp xy_picture_stamp;
static void xy_bind_picture(void){
 if(!xy_picture||!xy_frame)return;
 int change=image_stamp_update(&xy_picture_stamp,xy_picture,xy_frame,page_api?page_api->revision():0,xy_frame_progress);
 if(change==DISC_IMAGE_SOURCE)src(xy_picture,xy_frame);
 else if(change==DISC_IMAGE_PIXELS)FN(0x4ece1c,void (*)(LvObj *))(xy_picture);
}
static void xy_move(unsigned pos){if(pos>360)pos=360;/* Only first/last native layer handoff needs a refresh. Interior frames are opaque snapshots. */if(pos!=xy_progress&&(!pos||!xy_progress))dpu_defer_frame=1;xy_progress=pos;}
static void xy_reset_page(void){xy_progress=xy_animating=xy_dragging=xy_child_transition=0;page_navigation_state=(DiscPageNavigation){0};page_kind=DISC_PAGE_HOME;page_api=0;xy_frame=0;xy_picture_stamp=(DiscImageStamp){0};show(xy_cover,0);}
void disc_bt_set_xy(const DiscXyApi *api){if(api==xy_api)return;if(page_kind==DISC_PAGE_XY)xy_reset_page();xy_api=api;if(api)api->reset();}
void disc_bt_set_polar(const DiscPolarApi *api){if(api==polar_api)return;if(page_kind==DISC_PAGE_POLAR)xy_reset_page();polar_api=api;if(api)api->reset();}
void disc_bt_set_stereo(const DiscStereoApi *api){if(api==stereo_api)return;if(page_kind==DISC_PAGE_STEREO)xy_reset_page();stereo_api=api;if(api)api->reset();}
void disc_bt_set_spectrum(const DiscSpectrumApi *api){if(api==spectrum_api)return;if(page_kind==DISC_PAGE_SPECTRUM)xy_reset_page();spectrum_api=api;if(api)api->reset();}
void disc_bt_set_mandala(const DiscMandalaApi *api){if(api==mandala_api)return;if(page_kind==DISC_PAGE_MANDALA)xy_reset_page();mandala_api=api;if(api)api->reset();}
static void page_select(unsigned kind){
 page_kind=kind;page_api=kind==DISC_PAGE_STEREO?stereo_api:kind==DISC_PAGE_MANDALA?mandala_api:kind==DISC_PAGE_POLAR?polar_api:kind==DISC_PAGE_SPECTRUM?spectrum_api:xy_api;
 if(kind==DISC_PAGE_STEREO&&stereo_api)stereo_api->reset();
 xy_frame=0;xy_picture_stamp=(DiscImageStamp){0};
 xy_transition_source=0;
 /* A new renderer can have the same revision counter as the previous one. */
 for(unsigned f=0;f<3;f++)dpu_content[f]=0;dpu_frozen_kind=0;
}
static void xy_feed_pcm(const unsigned char *data,unsigned bits,unsigned channels,unsigned rate,unsigned t,unsigned samples){
 if(!page_api||!xy_progress||samples>DISC_SCOPE_MAX_SAMPLES)return;
 if(xy_dragging||xy_animating||xy_child_transition)return;
 unsigned step=page_kind==DISC_PAGE_SPECTRUM?disc_spectrum_step(rate):disc_scope_step(rate);
 if(page_kind==DISC_PAGE_SPECTRUM)disc_visual_spectrum_convert(&xy_gain,data,bits,channels,samples,step,t,visual_normalized(),xy_pcm);
 else {unsigned converted=disc_visual_scope_convert(&xy_gain,data,bits,channels,samples,rate,t,visual_normalized(),xy_pcm);if(!converted)return;if(disc_scope_padding(rate))rate*=4;samples=converted;}
 page_api->feed(xy_pcm,samples,rate/step,t);xy_read_count++;
}
static void xy_capture_main(void){
 unsigned *base=(unsigned *)DU(0x8e0924);unsigned frame=DU(0x8e0914);
 if(base&&frame<2&&DU(0x8e0910)==518400){unsigned rotation=display_rotation();for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++)xy_snapshot[(359-y)*360+359-x]=base[frame*129600+disc_physical_index(x,y,rotation)];}else memset(xy_snapshot,0,sizeof(xy_snapshot));
 if(*(unsigned *)0x8e16d4==3&&enabled&&dpu_on&&xy_progress==0){
  for(unsigned j=0;j<128;j++){const Tile *tile=&tiles[j];int state=dpu_fill[j/64]-(int)(j%64)*4;if(state<0)state=0;if(state>4)state=4;const unsigned *source=tile->image[state].data;
   for(int y=0;y<tile->h;y++)for(int x=0;x<tile->w;x++){unsigned pixel=source[y*tile->w+x];if(pixel>>24)xy_snapshot[(359-tile->y-y)*360+359-tile->x-x]=pixel;}
  }
 }
 for(unsigned i=0;i<129600;i++)xy_child_snapshot[i]=xy_snapshot[129599-i]|0xff000000u;
}
static void xy_settle(unsigned target){xy_started=now();xy_start_position=xy_child_transition?xy_child_position:xy_progress;xy_target=target>=180?360:0;xy_animating=xy_start_position!=xy_target;xy_dragging=0;}
static void xy_compose(unsigned t){
 unsigned began=now();
 if(xy_child_transition||xy_dragging||xy_animating||xy_progress<360){
  if(!xy_transition_source){page_api->tick(t);const DiscVisualImage *full=page_api->compose(360,0);if(!full)return;xy_transition_source=full->data;}
  unsigned direction=xy_child_transition?(page_bottom(xy_child_from)?(xy_child_from==DISC_PAGE_POLAR?DISC_SLIDE_RIGHT:DISC_SLIDE_LEFT):(xy_child_from==DISC_PAGE_XY?DISC_SLIDE_DOWN:DISC_SLIDE_UP)):(page_bottom(page_kind)?DISC_SLIDE_UP:page_kind==DISC_PAGE_SPECTRUM?DISC_SLIDE_LEFT:DISC_SLIDE_RIGHT);
  disc_slide_compose(xy_child_pixels,xy_child_snapshot,xy_transition_source,xy_child_transition?xy_child_position:xy_progress,direction);xy_frame=&xy_child_image;
 }else {xy_transition_source=0;page_api->tick(t);xy_frame=page_api->compose(360,xy_snapshot);}
 xy_frame_progress=xy_child_transition?65536+xy_child_position:xy_progress;
 unsigned elapsed=now()-began;if(elapsed>xy_render_max_ms)xy_render_max_ms=elapsed;xy_composed++;
}
static void xy_touch(void *e){
 if((!xy_api&&!polar_api&&!stereo_api&&!spectrum_api&&!mandala_api)||!active||!visual_mode()||menu_open)return;
 unsigned code=lv_event_code(e);void *input=FN(0x4e42e8,void *(*)(void))();int pt[2];if(!input)return;FN(0x4e45bc,void (*)(void *,int *))(input,pt);
 page_navigation_state.last_left=xy_last_left[visual_pref_index()];page_navigation_state.last_bottom=xy_last_bottom[visual_pref_index()];
 DiscPageNavigationResult event=page_navigation(&page_navigation_state,code,pt[0],pt[1],now(),page_kind,xy_progress,xy_animating,xy_api!=0,polar_api!=0,spectrum_api!=0,mandala_api!=0,stereo_api!=0);
 DiscSwipeResult action=event.motion;
 if((action.flags&SWIPE_BEGIN)&&page_navigation_state.child){
  if(page_api)xy_compose(now());
  if(!xy_frame)return;const unsigned *pixels=xy_frame->data;
  memcpy(xy_child_snapshot,pixels,518400);
  xy_child_from=page_kind;xy_child_position=0;xy_child_transition=1;page_select(event.kind);
 }else if(action.flags&SWIPE_BEGIN){
  xy_transition_source=0;
  if(xy_progress==0){xy_capture_main();page_select(event.kind);}
  else for(unsigned i=0;i<129600;i++)xy_child_snapshot[i]=xy_snapshot[129599-i]|0xff000000u;
 }
 xy_dragging=page_navigation_state.swipe.dragging;
 if(action.flags&SWIPE_MOVE){if(xy_child_transition){xy_child_position=action.position;}else xy_move(action.position);
  /* Compose the first transition frame before exposing any new layer. */
  if(page_api&&xy_progress&&!xy_frame)xy_compose(now());
  if(page_navigation_state.swipe.base==0)lv_clear_flag(xy_cover,2);show(xy_cover,xy_frame!=0);
 }
 if(action.flags&SWIPE_SETTLE){xy_last_left[visual_pref_index()]=page_navigation_state.last_left;xy_last_bottom[visual_pref_index()]=page_navigation_state.last_bottom;xy_settle(action.target);}
}
/* Main left edge keeps the stock exit. One full-screen cover receives both
 * visualization pages. Top-edge downward strokes remain native shade input. */
static void xy_root_touch(void *e){
 unsigned code=lv_event_code(e);
 if(code==1){int pt[2];void *input=FN(0x4e42e8,void *(*)(void))();if(!input)return;FN(0x4e45bc,void (*)(void *,int *))(input,pt);xy_native_gesture=xy_progress==0&&pt[0]<60;}
 if(xy_native_gesture)FN(visual_native_callback(),void (*)(void *))(e);else xy_touch(e);
 if(code==8||code==3)xy_native_gesture=0;
}
static void xy_deleted(void *e){LvObj *o=lv_event_target(e);if(o==xy_cover){xy_cover=xy_picture=0;xy_reset_page();}for(unsigned i=0;i<11;i++)if(o==xy_bound[i])xy_bound[i]=0;}
static void xy_bind(LvObj *panel){
 if(!panel)return;
 /* Wrap ONLY the three known stock left-edge navigation callbacks, preserving
  * native edge navigation and dispatching other strokes to XY. Scan the live
  * LVGL event array anew each prepare; never retain reallocatable pointers. */
 LvObj *event_panel=dlna_visual()?LV_GLOBAL(0x8de290):*(unsigned *)0x8e16d4==1?LV_GLOBAL(0x8de218):panel;
 unsigned native_callback=visual_native_callback();unsigned *spec=event_panel&&lv_valid(event_panel)?*(unsigned **)((char *)event_panel+8):0;if(spec){unsigned count=(spec[8]>>16)&63;unsigned *events=(unsigned *)spec[3];
  if(events&&count<=32)for(unsigned i=0;i<count;i++){unsigned code=events[i*3+2]&255;
   if(code!=1&&code!=2&&code!=8)continue;
   if((xy_api||polar_api||stereo_api||spectrum_api||mandala_api)&&active&&visual_mode()&&events[i*3]==native_callback)events[i*3]=(unsigned)xy_root_touch;
   else if(((!xy_api&&!polar_api&&!stereo_api&&!spectrum_api&&!mandala_api)||!active||!visual_mode())&&events[i*3]==(unsigned)xy_root_touch)events[i*3]=native_callback;
  }
 }
 const unsigned bt_addresses[]={0x8de3d4,0x8de3d0,0x8de3cc},dac_addresses[]={0x8de214,0x8de210,0x8de20c};
 const unsigned net_addresses[]={0x8de26c,0x8de288,0x8de28c};
 const unsigned *addresses=dlna_visual()?net_addresses:*(unsigned *)0x8e16d4==1?dac_addresses:bt_addresses;unsigned binding_offset=dlna_visual()?8:*(unsigned *)0x8e16d4==1?4:0;
 for(unsigned i=0;i<3;i++){LvObj *o=LV_GLOBAL(addresses[i]);if(!o||!lv_valid(o)||o==xy_bound[binding_offset+i])continue;xy_bound[binding_offset+i]=o;lv_add_cb(o,xy_touch,1,0);lv_add_cb(o,xy_touch,2,0);lv_add_cb(o,xy_touch,8,0);lv_add_cb(o,xy_deleted,33,0);}
 if((xy_api||polar_api||stereo_api||spectrum_api||mandala_api)&&!xy_cover){xy_cover=lv_create(panel);stylezero(xy_cover);lv_size(xy_cover,360,360);lv_align(xy_cover,1,0,0);lv_add_flag(xy_cover,2);lv_bg(xy_cover,0xff091116,0);opa(xy_cover,255,0);lv_add_cb(xy_cover,xy_touch,1,0);lv_add_cb(xy_cover,xy_touch,2,0);lv_add_cb(xy_cover,xy_touch,8,0);lv_add_cb(xy_cover,xy_deleted,33,0);
  xy_picture=img(xy_cover);stylezero(xy_picture);lv_align(xy_picture,1,0,0);show(xy_cover,0);
 }
}
static void xy_prepare_picture(void){
 if(!xy_progress&&xy_cover){show(xy_cover,dpu_on&&dpu_main_shade);show(xy_picture,0);return;}
 if(!xy_cover||!xy_picture||!xy_progress||!xy_frame||!meter_visible)return;
 int software=!dpu_on||dpu_base_only;
 show(xy_cover,1);show(xy_picture,software);
 if(software){xy_bind_picture();if(dpu_drop_pending)FN(0x4ece1c,void (*)(LvObj *))(xy_picture);}
}
static void xy_poll(unsigned t){
 if(!active||!visual_mode()||(!xy_api&&!polar_api&&!stereo_api&&!spectrum_api&&!mandala_api)||(xy_progress&&!page_api)){if(xy_progress)xy_reset_page();return;}
 /* Temporary shade/EQ/sleep visibility never changes the selected page. */
 if(menu_open||native_overlay_open||!meter_visible||!DU(0x83abd0)){if(xy_dragging||xy_animating){if(xy_child_transition){if(xy_child_position<180)page_select(xy_child_from);xy_child_transition=0;}xy_move(xy_progress>=180?360:0);xy_animating=xy_dragging=0;page_navigation_state=(DiscPageNavigation){0};}int behind=(menu_open||native_overlay_open)&&meter_visible&&DU(0x83abd0)&&xy_progress&&xy_frame;int software=behind&&(!dpu_on||!dpu_xy_shade);show(xy_cover,behind||(dpu_on&&dpu_main_shade));show(xy_picture,software);if(software)xy_bind_picture();return;}
 if(!xy_dragging&&!xy_animating&&xy_progress>0&&xy_progress<360)xy_settle(xy_progress>=180?360:0);
 if(xy_animating){unsigned elapsed=t-xy_started;unsigned position=xy_target;
  if(elapsed<220){unsigned v=elapsed*1024/220,ease=(v*v*(3072-2*v))>>20;int delta=(int)xy_target-(int)xy_start_position;position=(unsigned)((int)xy_start_position+delta*(int)ease/1024);}
  if(xy_child_transition)xy_child_position=position;else xy_move(position);
  if(elapsed>=220){xy_animating=0;if(xy_child_transition){if(!xy_target)page_select(xy_child_from);xy_child_transition=0;if(page_bottom(page_kind))xy_last_bottom[visual_pref_index()]=page_kind;else xy_last_left[visual_pref_index()]=page_kind;}}
 }
 if(xy_child_transition&&!xy_dragging&&!xy_animating){if(xy_child_position<180)page_select(xy_child_from);xy_child_transition=0;if(page_bottom(page_kind))xy_last_bottom[visual_pref_index()]=page_kind;else xy_last_left[visual_pref_index()]=page_kind;}
 if(xy_progress){xy_compose(t);if(xy_dragging&&page_navigation_state.swipe.base==0)lv_clear_flag(xy_cover,2);else lv_add_flag(xy_cover,2);show(xy_cover,1);show(xy_picture,!dpu_on||dpu_base_only);
  if((!dpu_on||dpu_base_only)&&t-xy_old_frame>=16){xy_old_frame=t;xy_bind_picture();}
 }else {show(xy_cover,0);if(!xy_dragging&&!xy_animating){page_kind=DISC_PAGE_HOME;page_api=0;}}
 if(t-xy_state_at>=1000){xy_state_at=t;char b[440];unsigned n=0,rate=0,age=0;if(page_api)page_api->stats(&n,&rate,&age,t);int z=snprintf(b,sizeof(b),"kind=%u\npage=%u\nanimating=%d\ndragging=%d\npoints=%u\nrate=%u\nage_ms=%u\nfeeds=%u\nhardware=%d\ncompositions=%u\nrender_max_ms=%u\n",page_kind,xy_progress,xy_animating,xy_dragging,n,rate,age,xy_read_count,dpu_on,xy_composed,xy_render_max_ms);xy_read_count=xy_composed=xy_render_max_ms=0;int f=open("/tmp/disc-runtime/visual-page.state",1|0x100|0x200,0600);if(f>=0){write(f,b,z);close(f);}f=open(page_kind==DISC_PAGE_STEREO?"/tmp/disc-runtime/stereo.state":page_kind==DISC_PAGE_MANDALA?"/tmp/disc-runtime/mandala.state":page_kind==DISC_PAGE_POLAR?"/tmp/disc-runtime/polar.state":page_kind==DISC_PAGE_SPECTRUM?"/tmp/disc-runtime/spectrum.state":"/tmp/disc-runtime/xy.state",1|0x100|0x200,0600);if(f>=0){write(f,b,z);close(f);}}
}
