#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned memory[0x100000/4];
#define PC_WORD(a) memory[((a)-0x800000)/4]
#define PC_BYTE(a) (*(unsigned char *)&PC_WORD((a)&~3u))
static unsigned dop_quiet_bypasses,dop_pre_calls,dop_pre_max,dop_pre_oversize,dop_pre_truncated_bytes;
static int source_enabled;static int dop_source(void){return source_enabled;}
static int fake_result;
static unsigned clock_ms=100,writes;static unsigned char packet[512],saved[512];
static int pc_workmode(void){return 4;}static unsigned pc_route(void){return 6;}
static int fake_clock(int kind,long *p){(void)kind;p[0]=clock_ms/1000;p[1]=(clock_ms%1000)*1000000;return 0;}
static int fake_write_pcm(void *p,const void *b,unsigned n){assert(p==(void *)1&&n==512&&!memcmp(b,saved,n));writes++;return fake_result;}
static int fake_read_pcm(void *p,void *b,unsigned n){assert(p==(void *)1&&n==512);memset(b,0x55,n);clock_ms+=10;return fake_result;}
static int fake_access(const char *p,int mode){(void)p;(void)mode;return -1;}
static int fake_open(const char *p,int flags,...){(void)p;(void)flags;return -1;}
static int fake_write(int f,const void *p,unsigned n){(void)f;(void)p;return n;}
static int fake_close(int f){(void)f;return 0;}
#define clock_gettime fake_clock
#define access fake_access
#define open fake_open
#define write fake_write
#define close fake_close
#define DISC_DOP_TRACE_TEST 1
#include "dop_trace.h"
int main(void){
 PC_WORD(0x83dfc4)=0x810000;PC_WORD(0x810000+0xc)=PC_WORD(0x810000+0x1c)=352800;PC_WORD(0x810000+0x2c)=2;
 dop_real_pcm_write=fake_write_pcm;dop_trace_enabled=1;
 for(unsigned i=0;i<64;i++){packet[i*8+3]=packet[i*8+7]=i&1?250:5;}
 memcpy(saved,packet,512);assert(!dop_trace_write((void *)1,packet,512));assert(dop_trace_calls==1&&!dop_trace_bad&&!dop_trace_phase_errors);
 clock_ms+=40;assert(!dop_trace_write((void *)1,packet,512));assert(!dop_trace_bad&&!dop_trace_phase_errors&&dop_trace_gap_max==40);
 /* Malformed final packet is captured; observation remains byte-for-byte. */
 packet[200+3]=0;memcpy(saved,packet,512);clock_ms+=100;assert(!dop_trace_write((void *)1,packet,512));assert(dop_trace_bad==1&&dop_trace_first_bad_offset==200&&dop_trace_sample_ready==1&&!memcmp(dop_trace_bad_sample,packet+200,256));
 dop_trace_enabled=0;assert(!dop_trace_write((void *)1,packet,512)&&dop_trace_calls==3&&writes==4);
 source_enabled=1;PC_BYTE(0x899060)=1;unsigned before=writes;assert(!dop_trace_write((void *)1,packet,512)&&writes==before&&dop_guard_dropped==1);
 source_enabled=0;dop_trace_enabled=1;fake_result=64;assert(dop_trace_write((void *)1,packet,512)==64&&!dop_trace_errors);fake_result=-32;assert(dop_trace_write((void *)1,packet,512)==-32&&dop_trace_errors==1);
 fake_result=-32;dop_real_pcm_read=fake_read_pcm;assert(dop_trace_read((void *)1,packet,512)==-32&&dop_read_errors==1&&dop_read_calls==1&&dop_read_max_ms==10);
 dop_trace_enabled=0;fake_result=64;assert(dop_trace_read((void *)1,packet,512)==64&&dop_read_calls==1);
 dop_trace_enabled=1;source_enabled=1;fake_result=32;for(unsigned i=0;i<64;i++)packet[i*8+3]=packet[i*8+7]=i&1?250:5;memcpy(saved,packet,512);assert(dop_trace_write((void *)1,packet,512)==32&&dop_trace_short_writes==1&&dop_trace_written_frames==32);
 puts("PASS: final DoP full-packet integrity/continuity/timing, corruption capture, unchanged audio and disabled zero inspection");
}
