/* Included after verified PLAYER controls. File/mapping setup occurs solely
 * on the existing 100 ms control tick, never on the audio worker. */
#include "ring.h"
extern void *mmap(void *,unsigned,int,int,int,long);
extern int munmap(void *,unsigned),ftruncate(int,long);
static DiscVisualRing *volatile vs_player;
static volatile unsigned vs_clock;
static unsigned vs_retry;static volatile int vs_gate;
static int dop_eq_install(void);
static unsigned vs_milliseconds(void){long t[2];return clock_gettime(1,t)?0:(unsigned)t[0]*1000u+(unsigned)t[1]/1000000u;}
static int vs_pcm_format(unsigned *rate){
 unsigned p=PC_WORD(0x83dfc4);int mode=pc_workmode();
 if(!p||(mode!=2&&mode!=4)||PC_WORD(p+0x20)!=2||PC_WORD(p+0x24)!=32||PC_BYTE(0x899060)||PC_WORD(p+0x48)==0||PC_WORD(p+0x48)==3)return 0;
 if(mode==4&&PC_WORD(p+0x2c)==2&&PC_WORD(p+0x58)==1&&PC_WORD(p+0x18)==0)return 0;
 *rate=PC_WORD(p+0x1c);return *rate>=8000&&*rate<=384000;
}
static void vs_player_tick(void){
 unsigned t=vs_milliseconds();vs_clock=t;vs_gate=0;
 if(!vs_player){
  if(!pc_enabled()||(vs_retry&&(unsigned)(t-vs_retry)<2000))return;vs_retry=t;
  char stage[96];snprintf(stage,sizeof(stage),DISC_VS_PATH ".%d.new",getpid());
  int fd=open(stage,2|0x100|0x200,0600);if(fd<0)return;
  if(ftruncate(fd,sizeof(DiscVisualRing))){close(fd);unlink(stage);return;}
  DiscVisualRing *r=mmap(0,sizeof(*r),3,1,fd,0);close(fd);
  if(r==(void *)-1){unlink(stage);return;}
  /* Only the header needs initialization: PCM is invalid until published. */
  __builtin_memset(r,0,128);r->abi=1;r->owner=getpid();r->bytes=sizeof(*r);__sync_synchronize();r->magic=DISC_VS_MAGIC;
  if(rename(stage,DISC_VS_PATH)){munmap(r,sizeof(*r));unlink(stage);return;}
  __sync_synchronize();vs_player=r;
 }
 unsigned rate;int mode=pc_workmode();
 if(!pc_enabled()||!disc_vs_requested(vs_player,t,mode)||!vs_pcm_format(&rate)){disc_vs_invalidate(vs_player);return;}
 /* Also observes native EQ when the DSP provider itself is disabled. */
 vs_gate=dop_eq_install();
}
static void vs_post_eq(const void *data,int bytes,int rate){
 DiscVisualRing *r=vs_player;unsigned actual;
 if(!r||!vs_gate||bytes<=0||(bytes&7)||!vs_pcm_format(&actual)||actual!=(unsigned)rate)return;
 disc_vs_write(r,data,(unsigned)bytes/8,(unsigned)rate,pc_workmode(),pc_route(),vs_clock);
}
