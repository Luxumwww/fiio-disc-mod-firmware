/* Exercise the actual paint/begin/finish implementation across lease release.
 * The provider may invoke paint on the last base refresh before rendered(). */
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned driver_words[8], *display_driver=driver_words;
static void *lv_disp_default(void){return &display_driver;}
static unsigned dpu_content[3],xy_drawn_generation[3],xy_drawn_progress[3];
static unsigned xy_frame_progress;
static int xy_progress,drawn[2],active=1,enabled=1,menu_open,native_overlay_open;
static int is_visual;
static int visual_mode(void){return is_visual;}
static struct {const unsigned *data;} image_frame;
static const __typeof__(image_frame) *xy_frame;
static unsigned revision(void){return 7;}
static const struct {unsigned (*revision)(void);} page_methods={revision},*page_api=&page_methods;
typedef struct {const unsigned *data;} TestImage;
typedef struct {int x,y,w,h;TestImage image[5];} Tile;
static Tile tiles[128];
static void *test_memset(void *p,int value,unsigned size){return memset(p,value,size);}
#define memset test_memset
#include "dpu_meter.h"
static unsigned claims,releases,presents,begins;
static unsigned claim(const char *owner,DiscAccelPaint paint,void *ctx){
 assert(!strcmp(owner,"disc-bt-meter/page")&&paint==dpu_paint&&!ctx);claims++;return claims;
}
static int valid(unsigned token){return token!=0;}
static void release(unsigned token){assert(token);releases++;}
static void begin(unsigned token){assert(token);begins++;}
static void present(unsigned token){assert(token);presents++;}
static const DiscAccelApi mock_api={.claim=claim,.valid=valid,.release=release,.begin=begin,.present=present};
static unsigned planes[3][129600],source[129600],tile_pixel=0xffabcdef;
static void assert_transparent(unsigned frame){
 for(unsigned i=0;i<129600;i++)assert(planes[frame][i]==0);
}
int main(void){
 for(unsigned i=0;i<128;i++){
  tiles[i].x=10+(int)(i%2);tiles[i].y=75+(int)(i%64);tiles[i].w=tiles[i].h=1;
  for(unsigned j=0;j<5;j++)tiles[i].image[j].data=&tile_pixel;
 }
 for(unsigned i=0;i<129600;i++)source[i]=0xff102030u+i;
 disc_bt_set_accel(&mock_api);
 for(unsigned rot=0;rot<4;rot++){
  driver_words[7]=rot<<4;
  for(unsigned cycle=0;cycle<12;cycle++){
   dpu_main_shade=dpu_pure_shade=dpu_main_ready=1;dpu_shade_height=180;
   memcpy(dpu_frozen_xy,source,sizeof source);
   dpu_begin(1);assert(dpu_on&&accel_token&&!dpu_drop_pending);
   for(unsigned frame=0;frame<3;frame++){
    dpu_paint(planes[frame],frame,0);
    for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++){
     unsigned i=disc_physical_index(x,y,rot);
     assert(planes[frame][i]==(y<180?0:source[i]));
    }
   }
   unsigned old_releases=releases,old_presents=presents;
   dpu_main_shade=dpu_pure_shade=dpu_main_ready=0;
   dpu_begin(0);assert(dpu_drop_pending&&!dpu_on&&accel_token);
   assert(!disc_bt_presentation_ready());
   disc_bt_after();assert(releases==old_releases&&presents==old_presents);
   /* Actual final LVGL flush paints the overlay BEFORE rendered releases it.
    * Every possible provider page must expose the newly drawn native base. */
   for(unsigned frame=0;frame<3;frame++){
    dpu_paint(planes[frame],frame,0);assert_transparent(frame);
    dpu_paint(planes[frame],frame,0);assert_transparent(frame);
   }
   /* A new menu/XY flag cannot revive old overlay content during retirement. */
   dpu_main_shade=1;dpu_shade_height=60;
   xy_progress=360;image_frame.data=source;xy_frame=&image_frame;
   for(unsigned frame=0;frame<3;frame++){dpu_paint(planes[frame],frame,0);assert_transparent(frame);}
   xy_progress=0;xy_frame=0;dpu_main_shade=0;
   disc_bt_rendered();assert(releases==old_releases+1&&!accel_token&&!dpu_drop_pending);
   assert(disc_bt_presentation_ready());
  }
 }
 /* The retained visual mode still draws its meter normally. */
 dpu_main_shade=dpu_xy_shade=dpu_base_only=0;is_visual=1;
 for(unsigned frame=0;frame<3;frame++){
  dpu_content[frame]=0;dpu_fill[0]=dpu_fill[1]=256;
  dpu_paint(planes[frame],frame,0);
  assert(planes[frame][disc_physical_index(10,75,dpu_rotation)]==tile_pixel);
 }
 printf("PASS: actual DPU handoff; 48 close/reopen cycles, four rotations, all three pages; final flush transparent, release after render, pumps gated; normal meter retained (%u claims/%u releases)\n",claims,releases);
}
