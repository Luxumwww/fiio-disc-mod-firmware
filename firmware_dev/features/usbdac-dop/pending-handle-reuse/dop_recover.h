/* Exact stock tinyalsa 1.1.1 / MIPS o32 PCM layout, guarded below.
 * Execute only inside native output serialization, on its existing handle.
 * Never reopen PCM/DAC or synthesize PCM silence in a DoP stream. */
static int (*dop_pcm_stop)(void *),(*dop_pcm_prepare)(void *);
static int (*dop_pcm_htimestamp)(void *,unsigned *,long *);
static const unsigned *(*dop_pcm_config)(void *);
static void *dop_recovery_pcm;
static unsigned dop_recovery_fd,dop_recovery_rate,dop_recovery_buffer;
static unsigned dop_recovery_start_saved,dop_recovery_stop_saved;
static unsigned dop_recovery_muted,dop_recovery_failed;
static unsigned dop_recovery_thresholds,dop_recovery_xruns,dop_recovery_restarts,dop_recovery_failures;
#ifndef DOP_PCM_WORD
#define DOP_PCM_WORD(p,o) (*(volatile unsigned *)((unsigned char *)(p)+(o)))
#endif
#ifndef DOP_PCM_STATE
#define DOP_PCM_STATE(p) (*(volatile unsigned *)(unsigned long)DOP_PCM_WORD(p,0xb8))
#endif
static int dop_recovery_layout(void *pcm){
 unsigned sink=PC_WORD(0x83dfe4),p=PC_WORD(0x83dfc4);
 if(!dop_pcm_config||!dop_pcm_stop||!dop_pcm_prepare||!dop_pcm_htimestamp||!dop_real_ioctl||!sink||!p||
    PC_WORD(sink+6*0x54+0x74)!=(unsigned)(unsigned long)pcm)return 0;
 const unsigned *c=dop_pcm_config(pcm);unsigned size=DOP_PCM_WORD(pcm,0x10);
 return c==(const unsigned *)((unsigned char *)pcm+0x98)&&
  (int)DOP_PCM_WORD(pcm,0)>=0&&!(DOP_PCM_WORD(pcm,4)&0x10000003u)&&
  DOP_PCM_WORD(pcm,0xb8)&&DOP_PCM_WORD(pcm,0x14)>=size&&
  c[0]==2&&c[1]==PC_WORD(p+0xc)&&(c[1]==176400||c[1]==352800)&&
  c[2]==c[1]/25&&c[3]==2&&c[4]==7&&size==c[2]*c[3];
}
static int dop_recovery_sw(void *pcm,unsigned start,unsigned stop){
 /* snd_pcm_sw_params has 104 bytes on the verified MIPS o32 kernel ABI.
  * Retain every field initialized by this stock tinyalsa pcm_set_config. */
 unsigned sw[26]={0};sw[0]=1;sw[1]=1;sw[3]=1;
 sw[4]=DOP_PCM_WORD(pcm,0xa0)/2;sw[5]=start;sw[6]=stop;
 sw[7]=DOP_PCM_WORD(pcm,0xb4);sw[9]=DOP_PCM_WORD(pcm,0x14);
 if(dop_real_ioctl((int)DOP_PCM_WORD(pcm,0),0xc0684113u,sw))return 0;
 DOP_PCM_WORD(pcm,0xac)=start;DOP_PCM_WORD(pcm,0xb0)=stop;return 1;
}
static void dop_recovery_forget(void){dop_recovery_pcm=0;dop_recovery_muted=dop_recovery_failed=0;}
static int dop_recovery_track(void *pcm){
 unsigned rate=DOP_PCM_WORD(pcm,0x9c),fd=DOP_PCM_WORD(pcm,0),size=DOP_PCM_WORD(pcm,0x10);
 /* Native close/open can reuse both heap address and fd at the same rate.
  * Its reset thresholds distinguish that new configuration; a failed
  * transaction remains latched rather than retrying on every write. */
 int reconfigured=pcm==dop_recovery_pcm&&!dop_recovery_failed&&
   (DOP_PCM_WORD(pcm,0xac)!=size||DOP_PCM_WORD(pcm,0xb0)!=size);
 if(pcm!=dop_recovery_pcm||fd!=dop_recovery_fd||rate!=dop_recovery_rate||size!=dop_recovery_buffer||reconfigured){
  dop_recovery_forget();dop_recovery_pcm=pcm;dop_recovery_fd=fd;dop_recovery_rate=rate;dop_recovery_buffer=size;
  dop_recovery_start_saved=DOP_PCM_WORD(pcm,0xac);dop_recovery_stop_saved=DOP_PCM_WORD(pcm,0xb0);
  if(!dop_recovery_sw(pcm,size,size)){dop_recovery_failed=1;dop_recovery_failures++;return 0;}
  dop_recovery_thresholds++;
 }
 return !dop_recovery_failed;
}
static int dop_recovery_sync(void *pcm,unsigned *available){
 long ts[2]={0};*available=0;
 int rc=dop_pcm_htimestamp(pcm,available,ts);
 unsigned kernel_state=DOP_PCM_STATE(pcm);
 /* PREPARED (2) is normal during full-buffer prefill. A bad timestamp alone
  * is not evidence of starvation; only kernel XRUN or measured overrun. */
 return kernel_state==4||(rc==0&&kernel_state==3&&*available>dop_recovery_buffer);
}
static int dop_recovery_begin(void *pcm){
 dop_recovery_xruns++;
 if(dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2d,0))goto fail;
 dop_recovery_muted=1;
 if(dop_pcm_stop(pcm)||dop_pcm_prepare(pcm)||!dop_bus_check(dop_recovery_rate,1))goto fail;
 return 1;
fail:
 dop_recovery_failed=1;dop_recovery_failures++;return 0;
}
static int dop_recovery_finish(void *pcm){
 if(!dop_recovery_muted)return 1;
 unsigned available;int starved=dop_recovery_sync(pcm,&available);
 if(starved)return 1; /* Next write restarts once; never unmute an XRUN. */
 if(DOP_PCM_STATE(pcm)!=3)return 1;
 if(dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2e,0)){
  dop_recovery_failed=1;dop_recovery_failures++;return 0;
 }
 dop_recovery_muted=0;dop_recovery_restarts++;return 1;
}
static int dop_recovery_write(void *pcm,const void *data,unsigned bytes,int guard){
 if(!guard){
  /* Never dereference a retired native handle. Native reopen/configure owns
   * rate/route changes. Restore only a still-identical managed PCM handle. */
  if(pcm==dop_recovery_pcm&&dop_recovery_layout(pcm)&&
     DOP_PCM_WORD(pcm,0)==dop_recovery_fd&&DOP_PCM_WORD(pcm,0x9c)==dop_recovery_rate&&
     DOP_PCM_WORD(pcm,0xac)==dop_recovery_buffer&&DOP_PCM_WORD(pcm,0xb0)==dop_recovery_buffer){
   if(!dop_recovery_sw(pcm,dop_recovery_start_saved,dop_recovery_stop_saved))return -1;
   if(dop_recovery_muted&&dop_real_ioctl((int)PC_WORD(0x83a7d0),0x20004d2e,0))return -1;
  }
  dop_recovery_forget();return dop_real_pcm_write(pcm,data,bytes);
 }
 if(!dop_recovery_layout(pcm))return dop_real_pcm_write(pcm,data,bytes);
 if(!dop_recovery_track(pcm))return -1;
 unsigned available;int restarted=dop_recovery_sync(pcm,&available);
 if(restarted&&!dop_recovery_begin(pcm))return -1;
 unsigned underruns=DOP_PCM_WORD(pcm,0xc);
 int result=dop_real_pcm_write(pcm,data,bytes);
 /* Stock tinyalsa hides EPIPE with prepare/retry, returning zero. Detect
  * that race using its counter, discard its newly queued copy under mute,
  * and requeue this unchanged valid block exactly once after resync. */
 if(!restarted&&DOP_PCM_WORD(pcm,0xc)!=underruns){
  if(!dop_recovery_begin(pcm))return -1;
  result=dop_real_pcm_write(pcm,data,bytes);
 }
 if(result<0){dop_recovery_failed=1;dop_recovery_failures++;return result;}
 if(!dop_recovery_finish(pcm))return -1;
 return result;
}
static void dop_recovery_report(void){
 char b[384];int n=snprintf(b,sizeof b,"threshold_updates=%u\nxrun_recoveries=%u\ncompleted_restarts=%u\nfailures=%u\nmanaged_rate=%u\nbuffer_frames=%u\nmuted_prefill=%u\nfailure_latched=%u\n",dop_recovery_thresholds,dop_recovery_xruns,dop_recovery_restarts,dop_recovery_failures,dop_recovery_pcm?dop_recovery_rate:0,dop_recovery_pcm?dop_recovery_buffer:0,dop_recovery_muted,dop_recovery_failed);
 int f=open("/tmp/disc-runtime/dop-recovery.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}
}
