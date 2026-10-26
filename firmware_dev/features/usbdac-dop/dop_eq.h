/* One verified MIPS call-site redirect. Keep every native EQ setting intact;
 * classified DoP is a packet stream, so it must never enter PCM EQ/gain. */
/* A diagnostic production profile can retain the stock DAC transition while
 * skipping only the player's zero-fill callback during classified USB DoP.
 * No valid DoP payload is changed. */
#ifdef DISC_STOCK_QUIET
#define DISC_DOP_STOCK_QUIET 1
#else
#define DISC_DOP_STOCK_QUIET 0
#endif
static int dop_eq_ready;
static int dop_eq_attempted;
static unsigned dop_eq_bypasses,dop_gain_bypasses;
static unsigned dop_quiet_bypasses;
static unsigned dop_pre_calls,dop_pre_max,dop_pre_oversize,dop_pre_truncated_bytes;
static int (*dop_original_gain)(void);
static int dop_source(void);
static int dop_quiet_filter(void *data,int bytes,int quiet){
 if(quiet==1&&dop_source()&&PC_BYTE(0x899060)==1){
  dop_quiet_bypasses++;
  /* Some stock callers rely on blocking pcm_write for pacing and add no
   * sleep themselves. A skipped quiet block must never become a busy loop. */
  PC_FN(0x826440,int (*)(unsigned))(5000);return 0;
 }
 return PC_FN(0x465718,int (*)(void *,int,int))(data,bytes,quiet);
}
/* Explicit one-shot capture. The audio callback only copies 256 bytes to RAM;
 * the existing control tick performs file I/O. No sampling unless requested. */
static volatile unsigned dop_capture_phase;
static unsigned dop_capture_rate,dop_capture_bytes;
static unsigned char dop_capture_data[256];
static void dop_capture_poll(void){
 if(dop_capture_phase==2){
  __sync_synchronize();int f=open("/tmp/disc-runtime/dop-output-sample.bin",1|0x100|0x200,0600);
  if(f>=0){write(f,dop_capture_data,sizeof(dop_capture_data));close(f);}
  char b[128];int n=snprintf(b,sizeof(b),"rate=%u\nchunk_bytes=%u\nsample_bytes=256\npoint=before_native_output_EQ\n",dop_capture_rate,dop_capture_bytes);
  f=open("/tmp/disc-runtime/dop-output-sample.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}dop_capture_phase=0;
 }
 if(!dop_capture_phase&&!access("/tmp/disc-runtime/dop-output-sample.request",0)){
  unlink("/tmp/disc-runtime/dop-output-sample.request");dop_capture_phase=1;
 }
}
static int dop_eq_filter(void *data,int bytes,int rate){
 if(dop_source()){
  if(bytes>0){dop_pre_calls++;if((unsigned)bytes>dop_pre_max)dop_pre_max=bytes;if(bytes>245760){dop_pre_oversize++;dop_pre_truncated_bytes+=(unsigned)bytes-245760;}}
  if(dop_capture_phase==1&&bytes>=(int)sizeof(dop_capture_data)){for(unsigned i=0;i<sizeof(dop_capture_data);i++)dop_capture_data[i]=((unsigned char *)data)[i];dop_capture_bytes=bytes;dop_capture_rate=rate;__sync_synchronize();dop_capture_phase=2;}
  dop_eq_bypasses++;return 0;
 }
#if !defined(DISC_DOP_TEST)||defined(DISC_VISUAL_TAP_TEST)
 dsp_process_buffer(data,bytes,rate);
#endif
 int result=PC_FN(0x45c648,int (*)(void *,int,int))(data,bytes,rate);
#if !defined(DISC_DOP_TEST)||defined(DISC_VISUAL_TAP_TEST)
 vs_post_eq(data,bytes,rate);
#endif
 return result;
}
static int dop_gain_filter(void){
 if(dop_source()){dop_gain_bypasses++;return 0;}
 return dop_original_gain?dop_original_gain():0;
}
#ifndef DISC_DOP_TEST
extern void *mmap(void *,unsigned,int,int,int,long);
extern int mprotect(void *,unsigned,int),munmap(void *,unsigned),cacheflush(void *,int,int);
static int dop_eq_install(void){
 if(dop_eq_ready)return 1;
 if(dop_eq_attempted)return 0;dop_eq_attempted=1;
 /* Exact native call and its delay slot, plus all pc_guards at installation. */
 if(PC_WORD(0x4658cc)!=0x0c117192u||PC_WORD(0x4658d0)!=0x02402025u)return 0;
 const unsigned quiet_calls[4]={0x465970,0x465af8,0x465d64,0x466420};
 const unsigned quiet_delay[4]={0x34058000,0x02202025,0x34058000,0x34058000};
 for(unsigned i=0;i<4;i++)if(PC_WORD(quiet_calls[i])!=0x0c1195c6u||PC_WORD(quiet_calls[i]+4)!=quiet_delay[i])return 0;
 unsigned *stub=mmap((void *)0x07000000,4096,3,0x802,-1,0);
 unsigned address=(unsigned)(unsigned long)stub,target=(unsigned)(unsigned long)dop_eq_filter;
 if(address<0x10000||address>=0x10000000u||(address&4095)){if(stub!=(void *)-1)munmap(stub,4096);return 0;}
 stub[0]=0x3c190000u|(target>>16); /* lui t9 */
 stub[1]=0x37390000u|(target&0xffff); /* ori t9 */
 stub[2]=0x03200008u;stub[3]=0; /* jr t9; nop, keep native jal's ra */
 target=(unsigned)(unsigned long)dop_quiet_filter;stub[4]=0x3c190000u|(target>>16);stub[5]=0x37390000u|(target&0xffff);stub[6]=0x03200008u;stub[7]=0;
 if(cacheflush(stub,32,3)||mprotect(stub,4096,5)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x465000,4096,7)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x466000,4096,7)){mprotect((void *)0x465000,4096,5);munmap(stub,4096);return 0;}
 PC_WORD(0x4658cc)=0x0c000000u|(address>>2);
 if(!dop_stock_profile||DISC_DOP_STOCK_QUIET)for(unsigned i=0;i<4;i++)PC_WORD(quiet_calls[i])=0x0c000000u|((address+16)>>2);
 __sync_synchronize();
 int flush=cacheflush((void *)0x465000,8192,3);
 if(flush){PC_WORD(0x4658cc)=0x0c117192u;for(unsigned i=0;i<4;i++)PC_WORD(quiet_calls[i])=0x0c1195c6u;cacheflush((void *)0x465000,8192,3);}
 int protect=mprotect((void *)0x465000,4096,5);protect|=mprotect((void *)0x466000,4096,5);
 /* A published stub stays mapped for the player lifetime, even on error. */
 dop_eq_ready=!flush&&!protect;return dop_eq_ready;
}
#else
static int dop_eq_install(void){dop_eq_ready=1;return 1;}
#endif
