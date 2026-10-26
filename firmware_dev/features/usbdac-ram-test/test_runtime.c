#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#define DISC_RAM_TEST
#include "replay.c"
static unsigned available=512,native_calls,accepted=13,last_requested,seen_frames,fresh_state=3;
static int mock_stamp(void *p,unsigned *a,long *t){(void)t;W(W((unsigned long)p+0xb8))=fresh_state;*a=available;return fresh_state==3?0:-1;}
static const unsigned *mock_config(void *p){return (const unsigned *)((unsigned char *)p+0x98);}
static int mock_write(void *p,const void *b,unsigned n){(void)p;(void)b;(void)n;native_calls++;return 0;}
static int mock_output(void *p,int n,int quiet){(void)p;(void)n;(void)quiet;native_calls++;return 0;}
static unsigned split_seen,split_native_calls;
static int split_mock(void *p,const void *b,unsigned n){
 assert((unsigned long)p==0x900b00&&b==clip+split_seen*8&&n<=4096&&!(n&7));
 unsigned frames=n/8;if(frames>13)frames=13;
 split_seen+=frames;split_native_calls++;return (int)frames;
}
int cacheflush(void *p,size_t n,int flags){(void)p;(void)n;(void)flags;return 0;}
static int mock_ioctl(int fd,unsigned req,void *arg){
 assert(fd==10&&req==0x800c4150u);struct WriteI *io=arg;last_requested=io->frames;
 unsigned n=io->frames<accepted?io->frames:accepted;const unsigned char *b=io->data;
 for(unsigned i=0;i<n;i++){unsigned expected=(seen_frames+i)&1?250:5;assert(b[i*8+3]==expected&&b[i*8+7]==expected);
  for(unsigned j=0;j<8;j++)if(j!=3&&j!=7)assert(b[i*8+j]==clip[(seen_frames+i)*8+j]);
 }
 seen_frames+=n;io->result=(int)n;return 0;
}
int main(void){
 assert(mmap((void *)0x800000,0x200000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)==(void *)0x800000);
 unsigned p=0x900200,sink=0x900500,pcm=0x900b00,status=0x900d00;
 W(0x83dfc0)=0x900100;W(0x900100)=4;W(0x83dfc4)=p;W(0x83dfe4)=sink;W(sink+0x58)=6;B(0x83a75a)=6;W(0x839a74)=1;
 clip_rate=352800;W(p+0xc)=W(p+0x1c)=clip_rate;W(p+0x10)=W(p+0x20)=2;W(p+0x14)=W(p+0x24)=32;W(p+0x2c)=2;W(p+0x58)=1;B(0x899060)=1;W(0x899084)=clip_rate;
 W(sink+6*0x54+0x74)=pcm;W(pcm)=10;W(pcm+0x10)=28224;W(pcm+0xb8)=status;W(status)=3;
 W(pcm+0x98)=2;W(pcm+0x9c)=clip_rate;W(pcm+0xa0)=14112;W(pcm+0xa4)=2;W(pcm+0xa8)=7;
 config=mock_config;stamp=mock_stamp;native_ioctl=mock_ioctl;native_write=mock_write;
 stock_output=mock_output;
 clip_bytes=8000;clip=malloc(clip_bytes);assert(clip);
 for(unsigned i=0;i<clip_bytes/8;i++)for(unsigned j=0;j<8;j++)clip[i*8+j]=(j==0||j==4)?0:(j==3||j==7)?((i&1)?250:5):(unsigned char)(i+j);
 assert(scope(clip_rate)&&layout((void *)(unsigned long)pcm));
 /* Native write does not refresh non-mmap status; SETUP cache is stale. */
 W(status)=1;phase=REPLAY;while(frames<clip_bytes/8)send_step(sink);
 assert(frames==1000&&position==clip_bytes&&partial>0&&last_requested<=512&&!errors);
 unsigned old=write_calls;available=0;send_step(sink);assert(write_calls==old);
 phase=IDLE;wrapped_write((void *)(unsigned long)pcm,clip,256);assert(native_calls==1);
 phase=REPLAY;wrapped_write((void *)(unsigned long)pcm,clip,256);assert(native_calls==1&&source_suppressed==1);
 W(p+0x2c)=0;wrapped_write((void *)(unsigned long)pcm,clip,256);assert(native_calls==2);send_step(sink);assert(phase==FAILED&&last_error==5);
 W(p+0x2c)=2;phase=REPLAY;fresh_state=4;send_step(sink);assert(phase==FAILED&&last_error==5);
 fresh_state=3;phase=REPLAY;available=30000;send_step(sink);assert(phase==FAILED&&last_error==6);
 phase=IDLE;output_filter(clip,256,0);assert(native_calls==3);
 phase=REPLAY;output_filter(clip,256,0);assert(native_calls==3); /* Sleep before native output mutex. */
 W(0x900100)=2;output_filter(clip,256,0);assert(native_calls==4);
 W(0x900100)=4;phase=IDLE;assert(capture_reset(352800));
 assert(wrapped_write((void *)(unsigned long)pcm,clip,256)==0&&native_calls==5);
 assert(capture_count==1&&capture_bytes==256&&!memcmp(capture_data,clip,256));
 assert(capture_events[0].bytes==256&&capture_events[0].copied==256&&capture_events[0].result==0);
 assert(!capture_reset(352800)); /* No concurrent reset/free while observing. */
 capture_bytes=4000000-8;
 wrapped_write((void *)(unsigned long)pcm,clip,256);
 assert(!capture_active&&capture_pending&&capture_inflight==0&&capture_bytes==4000000);
 assert(capture_events[1].copied==8&&capture_events[1].bytes==256&&native_calls==6);
 capture_pending=0;free(capture_data);capture_data=0;
 assert(capture_reset(352800));W(0x900100)=2;
 wrapped_write((void *)(unsigned long)pcm,clip,256);
 assert(!capture_active&&capture_error==2&&capture_pending&&capture_count==0&&native_calls==7);
 capture_pending=0;free(capture_data);capture_data=0;
 /* Exercise real wrapper selection: USB DoP64/128 only, existing locked
  * prepared PCM, exact sum return, PCM/mode/disable/fault forwarding. */
 W(0x900100)=4;phase=IDLE;enabled=1;W(pcm+8)=3;native_write=split_mock;
 assert(split_scope((void *)(unsigned long)pcm,clip,clip_bytes));
 assert(wrapped_write((void *)(unsigned long)pcm,clip,clip_bytes)==1000);
 assert(split_seen==1000&&split_native_calls>2&&split_selected==1&&split_stats.frames==1000);
 W(p+0xc)=W(p+0x1c)=W(0x899084)=W(pcm+0x9c)=176400;
 assert(split_scope((void *)(unsigned long)pcm,clip,clip_bytes));
 native_write=mock_write;unsigned before=native_calls;
 W(p+0x2c)=1;assert(wrapped_write((void *)(unsigned long)pcm,clip,256)==0&&native_calls==before+1);
 W(p+0x2c)=2;W(0x900100)=2;assert(!split_scope((void *)(unsigned long)pcm,clip,256));
 W(0x900100)=4;enabled=0;assert(!split_scope((void *)(unsigned long)pcm,clip,256));
 enabled=1;W(pcm+8)=0;assert(!split_scope((void *)(unsigned long)pcm,clip,256));
 W(pcm+8)=3;split_stats.fault=1;assert(!split_scope((void *)(unsigned long)pcm,clip,256));
 split_stats.fault=0;phase=REPLAY;assert(!split_scope((void *)(unsigned long)pcm,clip,256));
 puts("PASS: live splitter wrapper DoP64/128 selection, sum frame return, PCM/mode/disable/unprepared/fault/replay exclusion");
 puts("PASS: production writer partial transfer, payload continuity, PCM forwarding, format exit, XRUN/invalid availability abort");
 puts("PASS: final-output observer preserves native forwarding, exact RAM copy, bounded capacity and mode-change abort");
}
