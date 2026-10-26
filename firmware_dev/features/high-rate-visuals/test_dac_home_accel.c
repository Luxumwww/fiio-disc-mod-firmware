/* Exercise the production lease/paint/handoff code, not a duplicate policy. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void *dc_copy(void *d,const void *s,unsigned n){return memcpy(d,s,n);}
static void *dc_zero(void *d,int c,unsigned n){return memset(d,c,n);}
#define memcpy dc_copy
#define memset dc_zero
static unsigned driver[8]={0,0,0,0,0,0,0,32};
static unsigned *display=driver;
static void *lv_disp_default(void){return &display;}
typedef struct {const void *data;} Image;
typedef struct {int x,y,w,h;Image image[5];} Tile;
static Tile tiles[128];
static unsigned pixels[129600],overlay[129600],dpu_content[3];
static Image frame={pixels};static Image *xy_frame;
static unsigned revision(void){return 1;}
static struct {unsigned (*revision)(void);} page={revision},*page_api=&page;
static unsigned xy_drawn_generation[3],xy_drawn_progress[3],xy_frame_progress=360;
static int active=1,enabled=0,xy_progress,menu_open,native_overlay_open,drawn[2];
static int visual_mode(void){return 1;}
#include "../bluetooth-meter/dpu_meter.h"
static unsigned claims,releases,begins,presents,live,allow=1;
static unsigned claim(const char *owner,DiscAccelPaint paint,void *context){
 assert(!strcmp(owner,"disc-bt-meter/page"));claims++;if(!allow)return 0;
 live=claims;for(unsigned i=0;i<3;i++)paint(overlay,i,context);return live;
}
static int valid(unsigned token){return live&&token==live;}
static void release(unsigned token){assert(token==live);releases++;live=0;}
static void begin(unsigned token){assert(valid(token));begins++;}
static void present(unsigned token){assert(valid(token));presents++;}
static const DiscAccelApi mock={.claim=claim,.valid=valid,.release=release,.begin=begin,.present=present};
static void transparent(void){for(unsigned i=0;i<129600;i++)assert(overlay[i]==0);}
int main(void){
 disc_bt_set_accel(&mock);for(unsigned i=0;i<129600;i++)overlay[i]=0x12345678;
 dpu_dac_home=dpu_base_only=1;dpu_begin(1);
 assert(dpu_on&&claims==1&&begins==1);transparent();
 for(unsigned i=0;i<100;i++){dpu_begin(1);disc_bt_after();disc_bt_rendered();}
 assert(claims==1&&presents==0&&releases==0);
 /* Keep the layer until a native replacement frame has actually rendered. */
 dpu_dac_home=0;dpu_begin(0);disc_bt_after();assert(live&&dpu_drop_pending);
 disc_bt_rendered();assert(!live&&!dpu_on&&releases==1);
 /* Existing scope presentation must still run at its own cadence. */
 xy_progress=360;xy_frame=&frame;dpu_base_only=0;enabled=1;
 for(unsigned i=0;i<129600;i++)pixels[i]=0xff012345;
 dpu_begin(1);assert(dpu_on&&claims==2&&overlay[100]==0xff012345);
 disc_bt_after();assert(presents==1);
 /* Returning home clears a previously opaque scope in every reused page. */
 xy_progress=0;xy_frame=0;dpu_dac_home=dpu_base_only=1;dpu_begin(1);
 for(unsigned i=0;i<3;i++){dpu_paint(overlay,i,0);transparent();}
 disc_bt_after();disc_bt_rendered();assert(presents==1&&releases==1);
 dpu_dac_home=0;dpu_begin(0);disc_bt_rendered();assert(releases==2);
 allow=0;dpu_dac_home=dpu_base_only=1;dpu_begin(1);assert(!dpu_on&&!accel_token);
 disc_bt_after();disc_bt_rendered();assert(presents==1);
 disc_bt_set_accel(0);dpu_begin(1);assert(!dpu_on);
 puts("PASS production DAC lease: transparent buffers, zero extra presents, deferred release, scope handoff, unavailable-provider fallback");
 fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
