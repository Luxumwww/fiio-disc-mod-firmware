#include "../hardware-accel/runtime-provider/accel_api.h"
#define DU(a) (*(volatile unsigned *)(a))
#include "display_transform.h"
static unsigned display_rotation(void){void *disp=lv_disp_default();unsigned *driver=disp?*(unsigned **)disp:0;return driver?(driver[7]>>4)&3u:2;}
static unsigned dpu_rotation=2;
static const DiscAccelApi *accel;
static unsigned accel_token;
static int dpu_on,dpu_failed,dpu_fill[2],dpu_drawn[3][2];
static int dpu_drop_pending,dpu_base_only,dpu_xy_shade,dpu_main_shade,dpu_main_ready,dpu_pure_shade;
static int dpu_dac_home;
static int dpu_defer_frame;
static unsigned dpu_main_rotation,dpu_main_generation,dpu_frozen_kind;
static unsigned dpu_shade_height,dpu_shade_mask[3],dpu_frozen_generation,dpu_frozen_rotation=4;
static unsigned dpu_frozen_xy[129600];
#include "shade_plane.h"
static unsigned dpu_frames,dpu_copied,dpu_fullcopies;
static void dpu_paint(unsigned *dest,unsigned frame,void *context){
 (void)context;
 /* The provider still calls paint for the final native refresh before the
  * lease is released in rendered(). Its base now contains the restored page;
  * expose that base, never fall through to XY/frozen pixels or meter tiles.
  * In pure mode the old fallthrough flashed a meter-shaped patch at close. */
 if(dpu_drop_pending){if(dpu_content[frame]!=3){memset(dest,0,518400);dpu_content[frame]=3;}return;}
 if(dpu_main_shade||(dpu_xy_shade&&xy_frame&&page_api)){
  unsigned generation=dpu_main_shade?0:page_api->revision();
  if(!dpu_main_shade&&(dpu_frozen_kind!=1||dpu_frozen_rotation!=dpu_rotation||dpu_frozen_generation!=generation)){
   disc_rotate_frame(dpu_frozen_xy,xy_frame->data,dpu_rotation);
   dpu_frozen_kind=1;dpu_frozen_rotation=dpu_rotation;dpu_frozen_generation=generation;for(unsigned f=0;f<3;f++)if(dpu_content[f]==4)dpu_content[f]=0;
  }
  if(dpu_content[frame]!=4){memcpy(dest,dpu_frozen_xy,518400);dpu_shade_mask[frame]=0;dpu_content[frame]=4;}
  shade_mask_strip(dest,dpu_frozen_xy,dpu_rotation,dpu_shade_mask[frame],dpu_shade_height);dpu_shade_mask[frame]=dpu_shade_height;return;
 }
 /* Native shade/volume draws over the frozen XY in the base plane. Keep the
  * same buffer/flush ownership, with an entirely transparent overlay. */
 if(dpu_base_only){if(dpu_content[frame]!=3){extern void *memset(void *,int,unsigned);memset(dest,0,518400);dpu_content[frame]=3;}return;}
 if(xy_progress&&xy_frame&&page_api){
  unsigned generation=page_api->revision();if(dpu_content[frame]!=2||xy_drawn_generation[frame]!=generation||xy_drawn_progress[frame]!=xy_frame_progress){disc_rotate_frame(dest,xy_frame->data,dpu_rotation);xy_drawn_generation[frame]=generation;xy_drawn_progress[frame]=xy_frame_progress;dpu_content[frame]=2;}return;
 }
 if(dpu_content[frame]!=1){extern void *memset(void *,int,unsigned);memset(dest,0,518400);dpu_drawn[frame][0]=dpu_drawn[frame][1]=-1;dpu_content[frame]=1;}

 for(int c=0;c<2;c++){
  int old=dpu_drawn[frame][c],fill=dpu_fill[c];if(old==fill)continue;
  int first=old<0?0:(old<fill?old:fill)/4,last=old<0?63:(old>fill?old:fill)/4;if(last>63)last=63;
  for(int j=first;j<=last;j++){const Tile *tile=&tiles[c*64+j];int state=fill-j*4;if(state<0)state=0;if(state>4)state=4;
   const unsigned *source=tile->image[state].data;
   /* Stock LVGL rotates the logical image 180 degrees before fb0 flush. */
   for(int y=0;y<tile->h;y++)for(int x=0;x<tile->w;x++){unsigned pixel=source[y*tile->w+x];if(pixel>>24)dest[disc_physical_index(tile->x+x,tile->y+y,dpu_rotation)]=pixel;}
  }dpu_drawn[frame][c]=fill;
 }__sync_synchronize();
}

static void dpu_stop(void){if(accel&&accel_token)accel->release(accel_token);accel_token=0;if(dpu_on||dpu_drop_pending){drawn[0]=drawn[1]=-1;}dpu_on=dpu_drop_pending=0;}
void disc_bt_set_accel(const DiscAccelApi *api){if(api==accel)return;dpu_stop();accel=api;}
static void dpu_status(int reset){DiscAccelStats s;if(!accel)return;accel->stats(accel_token,reset,&s);dpu_failed=s.failed;
 if(accel_token&&s.lease==accel_token){dpu_frames=s.frames;dpu_copied=s.copy_bytes;dpu_fullcopies=s.fullcopies;}}
static void dpu_begin(int wanted){
 unsigned rotation=display_rotation();if(rotation!=dpu_rotation){dpu_rotation=rotation;for(unsigned f=0;f<3;f++){dpu_content[f]=0;dpu_drawn[f][0]=dpu_drawn[f][1]=-1;}}
 if(accel_token&&(!accel||!accel->valid(accel_token)))dpu_stop();
 /* First draw the native/software replacement into fb0. Restoring the saved
  * DPU layout before that draw exposes the stale main page for one frame. */
 if(!wanted){if(accel_token){dpu_drop_pending=1;dpu_on=0;}return;}if(!accel)return;dpu_drop_pending=0;
 if(!accel_token){for(unsigned f=0;f<3;f++){dpu_drawn[f][0]=dpu_drawn[f][1]=-1;dpu_content[f]=0;}accel_token=accel->claim("disc-bt-meter/page",dpu_paint,0);}
 if(accel_token)accel->begin(accel_token);
 int was=dpu_on;dpu_on=accel_token&&accel->valid(accel_token);if(was&&!dpu_on)drawn[0]=drawn[1]=-1;if(!dpu_on&&accel_token)dpu_stop();
}
static void dpu_finish(int rendered){
 if(dpu_drop_pending){if(rendered)dpu_stop();return;}
 if(dpu_defer_frame&&!rendered)return;
 /* The DAC home is already drawn in the base plane. The lease only enables
  * the provider's previous-damage synchronization before native flushes.
  * An independent present here would add redundant pans/vsync waits. */
 if(dpu_dac_home)return;
 if(!active||(!enabled&&!xy_progress&&!dpu_main_shade)||((menu_open||native_overlay_open)&&!dpu_base_only&&!dpu_xy_shade&&!dpu_main_shade)||(!visual_mode()&&!dpu_pure_shade)){dpu_stop();return;}
 if(accel_token&&!accel->valid(accel_token))dpu_stop();
 if(dpu_on)accel->present(accel_token);
}
void disc_bt_after(void){dpu_finish(0);}
int disc_bt_presentation_ready(void){return !dpu_defer_frame&&!dpu_drop_pending;}
void disc_bt_rendered(void){dpu_defer_frame=0;dpu_finish(1);}
