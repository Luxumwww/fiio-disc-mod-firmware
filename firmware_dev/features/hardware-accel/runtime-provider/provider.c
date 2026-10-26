typedef unsigned size_t;
extern int open(const char *,int,...),close(int),memcmp(const void *,const void *,unsigned),clock_gettime(int,void *);
extern void *memcpy(void *,const void *,unsigned);
#include "accel_api.h"
#include "provider_guard.h"
#include "code_guard.h"
#ifndef FN
#define FN(a,t) ((t)(a))
#endif
static unsigned now(void){struct {long s,n;} t;clock_gettime(1,&t);return (unsigned)t.s*1000+(unsigned)t.n/1000000;}
#include "frame_pacing.h"
static DiscFramePacer dpu_pacer;
static unsigned long long frame_us(void){struct {long s,n;} t;clock_gettime(1,&t);return (unsigned long long)t.s*1000000+(unsigned)t.n/1000;}
/* Exact V2.57 ABI. All operations run under the original UI mutex. */
extern int ioctl(int,unsigned,...),munmap(void *,unsigned);
extern void *mmap(void *,unsigned,int,int,int,long),*memcpy(void *,const void *,unsigned),*memset(void *,int,unsigned);
#define DU(a) (*(volatile unsigned *)(a))
static unsigned dpu_saved[72],dpu_trial[72];
static unsigned char *dpu_pixels;
static int dpu_fd=-1,dpu_on,dpu_failed,dpu_flushed;
static int provider_enabled,provider_guarded;
static DiscAccelPaint owner_paint;static void *owner_context;
static unsigned dpu_frames,dpu_copied,dpu_fullcopies,dpu_checkat,dpu_presentat;
static void *dpu_driver;
static int dpu_previous[32][4],dpu_pending[32][4],dpu_nprevious=-1,dpu_npending;
static void (*dpu_native)(void *,int *,void *);
static int dpu_shape(void){return DU(0x8e0910)==518400&&DU(0x8e0918)==1&&DU(0x8e0954)==1440&&DU(0x8e096c)==360&&DU(0x8e0970)==360&&DU(0x8e0984)==32&&DU(0x8e0914)<2&&DU(0x83abd8)<2&&DU(0x8e0924);}
static void dpu_sync(void){
 if(!DU(0x83abd4))return;
 unsigned char *base=(void *)DU(0x8e0924),*dst=base+518400*DU(0x83abd8),*source=base+518400*DU(0x8e0914);
 if(dpu_nprevious<0){memcpy(dst,source,518400);dpu_copied+=518400;dpu_fullcopies++;}
 else for(int i=0;i<dpu_nprevious;i++){int *r=dpu_previous[i];unsigned bytes=(r[2]-r[0]+1)*4;for(int y=r[1];y<=r[3];y++)memcpy(dst+y*1440+r[0]*4,source+y*1440+r[0]*4,bytes);dpu_copied+=bytes*(r[3]-r[1]+1);}
 DU(0x83abd4)=0;
}
static void dpu_paint(unsigned frame){if(owner_paint)owner_paint((void *)(dpu_pixels+frame*518400),frame,owner_context);__sync_synchronize();}
static int dpu_pan(void);
static void dpu_stop(void);
static void dpu_damage(int *area){
 if(dpu_npending>=0){int r[4]={area[0]<0?0:area[0],area[1]<0?0:area[1],area[2]>359?359:area[2],area[3]>359?359:area[3]};
  if(r[0]<=r[2]&&r[1]<=r[3]){if(dpu_npending==32)dpu_npending=-1;else memcpy(dpu_pending[dpu_npending++],r,sizeof r);}}
}
static void dpu_finish(unsigned long long submitted){
 disc_frame_commit(&dpu_pacer,submitted);
 dpu_flushed=1;dpu_presentat=submitted/1000;dpu_frames++;dpu_nprevious=dpu_npending;if(dpu_nprevious>0)memcpy(dpu_previous,dpu_pending,dpu_nprevious*sizeof dpu_previous[0]);dpu_npending=0;
}
#include "quarter_flush.h"
static void dpu_flush(void *driver,int *area,void *pixels){
 unsigned flags=DU((unsigned)driver+0x1c);
 if(!dpu_on||!dpu_shape()||(flags&3)){dpu_nprevious=-1;dpu_native(driver,area,pixels);return;}
 /* LVGL already rotated these rectangles. Its stock 90/270 flush writes
  * directly into the scanned front page; collect every chunk in the back
  * page and pan only on the genuine final rotated chunk instead. */
 if(flags&0x10){
  int width=area[2]-area[0]+1,height=area[3]-area[1]+1;
  if(width<=0||width>360||height<=0||height>360){dpu_nprevious=-1;dpu_native(driver,area,pixels);return;}
  dpu_sync();unsigned next=DU(0x83abd8);
  quarter_copy((unsigned *)(DU(0x8e0924)+next*518400),area,pixels);dpu_damage(area);
  if(FN(0x51ab20,int (*)(void *))(driver)){
   unsigned long long submitted=frame_us();dpu_paint(next);__sync_synchronize();
   DU(0x8e097c)=0;DU(0x8e0980)=next*360;DU(0x8e0914)=next;DU(0x83abd8)=1-next;
   if(!dpu_pan()){dpu_failed=1;dpu_stop();}else {DU(0x83abd4)=1;dpu_finish(submitted);}
  }
  FN(0x51ab0c,void (*)(void *))(driver);return;
 }
 dpu_sync();
 dpu_damage(area);
 int last=FN(0x51ab20,int (*)(void *))(driver);unsigned long long submitted=last?frame_us():0;if(last)dpu_paint(DU(0x83abd8));
 dpu_native(driver,area,pixels);
 if(last)dpu_finish(submitted);
}
static int dpu_pan(void){unsigned zero[2]={0,0};int fd=(int)DU(0x8e091c);if(ioctl(fd,0x4606,(void *)0x8e096c))return 0;return ioctl(fd,0x80044620,zero)==0;}
#include "whitepoint_plane.h"
static void dpu_stop(void){
 if(!dpu_on)return;unsigned current[72];int fd=(int)DU(0x8e091c);
 if(!ioctl(fd,0xc0044702u,current)&&!memcmp(current,dpu_trial,sizeof current)){
  if(!wp_write_checked(current,dpu_saved))dpu_failed=1;
  else if(DU(0x83abd0)&&dpu_shape())dpu_pan();
 }else {dpu_failed=1;wp_external();}
 if(dpu_driver){void (**callback)(void *,int *,void *)=(void *)((char *)dpu_driver+0x20);if(*callback==dpu_flush)*callback=dpu_native;}
 dpu_on=0;dpu_nprevious=-1;dpu_npending=0;memset(&dpu_pacer,0,sizeof dpu_pacer);
}
static int dpu_start(void){
 if(dpu_on)return 1;if(dpu_failed||!dpu_shape()||!DU(0x83abd0))return 0;
 void *display=FN(0x51a7e0,void *(*)(void))();void *driver=display?*(void **)display:0;
 if(!driver||(DU((unsigned)driver+0x1c)&~0x30u)!=0x410c)return 0;
 void (**callback)(void *,int *,void *)=(void *)((char *)driver+0x20);
 if(*callback!=dpu_flush&&(unsigned)*callback!=0x565dcc){dpu_failed=1;return 0;}
 int fd=(int)DU(0x8e091c);unsigned cfg[72],fix[32],mainfix[32];
 if(ioctl(fd,0xc0044702u,cfg))return 0;
 if(!wp_armed)wp_recover(cfg);
 if(wp_recovered_visual&&(disc_accel_whitepoint_set((unsigned)wp_value)<0||ioctl(fd,0xc0044702u,cfg))){if(wp_fault)dpu_failed=1;return 0;}
 if(!wp_base_ok(cfg)){dpu_failed=1;return 0;}
 for(unsigned i=0;i<4;i++)if(cfg[i*18+1]!=360||cfg[i*18+2]!=360||cfg[i*18+3]!=360){dpu_failed=1;return 0;}
 if(dpu_fd<0){dpu_fd=open("/dev/fb1",2);if(dpu_fd<0){dpu_failed=1;return 0;}
  if(ioctl(dpu_fd,0x4602,fix)||ioctl(fd,0x4602,mainfix)||!mainfix[4]||fix[4]<mainfix[4]||fix[4]-mainfix[4]!=2073600||(fix[4]&3)||fix[5]!=1555200||fix[11]!=1440||mainfix[5]!=1555200||mainfix[11]!=1440){close(dpu_fd);dpu_fd=-1;dpu_failed=1;return 0;}
  /* fb_mmap maps PAGE_DOWN(smem_start), not smem_start itself. */
  unsigned bias=fix[4]&4095;void *mapping=mmap(0,1555200+bias,3,1,dpu_fd,0);
  if(mapping==(void *)-1){dpu_pixels=0;close(dpu_fd);dpu_fd=-1;dpu_failed=1;return 0;}dpu_pixels=(unsigned char *)mapping+bias;}
 memset(dpu_pixels,0,1555200);
 for(unsigned f=0;f<3;f++)dpu_paint(f);
 memcpy(dpu_saved,cfg,sizeof cfg);memcpy(dpu_trial,cfg,sizeof cfg);
 wp_visual(dpu_trial,wp_value>0);
 if(!wp_write_checked(cfg,dpu_trial)){if(wp_fault)dpu_failed=1;return 0;}
 dpu_driver=driver;dpu_native=FN(0x565dcc,void (*)(void *,int *,void *));*callback=dpu_flush;
 dpu_on=1;dpu_nprevious=-1;dpu_npending=0;dpu_checkat=now();return 1;
}
static void dpu_begin(int wanted){
 dpu_flushed=0;if(!wanted||!dpu_shape()){dpu_stop();return;}if(!dpu_start())return;
 unsigned t=now();if(t-dpu_checkat>=200){unsigned cfg[72];dpu_checkat=t;
  if(ioctl((int)DU(0x8e091c),0xc0044702u,cfg)||memcmp(cfg,dpu_trial,sizeof cfg)){dpu_failed=1;wp_external();dpu_stop();}}
}
static void dpu_present(void){
 unsigned long long submitted=frame_us();if(!dpu_on||!dpu_shape()||!DU(0x83abd0)||!disc_frame_due(&dpu_pacer,submitted))return;
 /* No LVGL damage: submit a new hardware-composited frame without redrawing
  * the base UI. Synchronize its previous damage into the inactive buffer. */
 dpu_sync();unsigned next=DU(0x83abd8);dpu_paint(next);
 DU(0x8e097c)=0;DU(0x8e0980)=next*360;DU(0x8e0914)=next;DU(0x83abd8)=1-next;
 if(!dpu_pan()){dpu_failed=1;dpu_stop();return;}
 /* Frame interval begins at submission, not after blocking for vsync.
  * Measuring after the wait adds a second frame delay to every frame. */
 DU(0x83abd4)=1;dpu_nprevious=0;dpu_npending=0;disc_frame_commit(&dpu_pacer,submitted);dpu_presentat=submitted/1000;dpu_frames++;
}

#include "lease.h"
static DiscLease ownership;
static char owner_name[64];
static void api_release(unsigned token){if(!lease_valid(&ownership,token))return;dpu_stop();lease_release(&ownership,token);owner_paint=0;owner_context=0;owner_name[0]=0;}
static unsigned api_claim(const char *owner,DiscAccelPaint paint,void *context){
 if(!provider_enabled||!provider_guarded||!owner||!owner[0]||!paint)return 0;
 unsigned length=0;while(length<64&&owner[length])length++;if(length==64)return 0;
 unsigned token=lease_claim(&ownership);if(!token)return 0;
 memcpy(owner_name,owner,length+1);owner_paint=paint;owner_context=context;
 if(!dpu_start()){api_release(token);return 0;}return token;
}
static int api_valid(unsigned token){return lease_valid(&ownership,token)&&dpu_on;}
static void api_begin(unsigned token){if(!lease_valid(&ownership,token))return;dpu_begin(provider_enabled&&DU(0x83abd0));if(!dpu_on)api_release(token);}
static void api_present(unsigned token){if(!lease_valid(&ownership,token))return;api_begin(token);if(!api_valid(token))return;dpu_present();if(!dpu_on)api_release(token);}
static void api_enabled(int yes){provider_enabled=yes&&provider_guarded;if(!provider_enabled){api_release(ownership.token);(void)disc_accel_whitepoint_set(0);}}
static void api_stats(unsigned token,int reset,DiscAccelStats *out){
 if(!out)return;memset(out,0,sizeof(*out));out->enabled=provider_enabled;out->active=dpu_on;out->failed=dpu_failed;out->lease=ownership.token;
 memcpy(out->owner,owner_name,sizeof(owner_name));out->frames=dpu_frames;out->copy_bytes=dpu_copied;out->fullcopies=dpu_fullcopies;
 if(reset&&lease_valid(&ownership,token))dpu_frames=dpu_copied=dpu_fullcopies=0;
}
static void api_pump(void){wp_tick();if(ownership.token)api_present(ownership.token);}
static const DiscAccelApi api={DISC_ACCEL_ABI,sizeof(DiscAccelApi),DISC_ACCEL_CAP_DPU,360,360,1440,180,api_claim,api_valid,api_release,api_begin,api_present,api_enabled,api_stats,api_pump};
const DiscAccelApi *disc_accel_get_api(unsigned abi){
 if(abi!=DISC_ACCEL_ABI)return 0;provider_guarded=1;
 for(unsigned i=0;i<sizeof(provider_guards)/sizeof(provider_guards[0]);i++)if(!provider_code_equal((const volatile unsigned char *)provider_guards[i].address,provider_guards[i].bytes,16))provider_guarded=0;
 return provider_guarded?&api:0;
}
