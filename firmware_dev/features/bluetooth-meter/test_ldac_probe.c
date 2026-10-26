/* Exercises the production header with bounded, read-only /proc fixtures. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BASE_ADDRESS 0x1000000u
#define STACK_ADDRESS (BASE_ADDRESS+0x1000u)
#define HANDLE_ADDRESS (BASE_ADDRESS+0x8000u)
#define INNER_ADDRESS (BASE_ADDRESS+0xa000u)
static unsigned char memory[0x20000];
static unsigned fake_time,read_cost,full_map_rows,map_row;
static unsigned reads,maps_reads,maps_opens,mem_opens,thread_reads,header_reads,closed;
static unsigned producer_reads;
static int mutate,short_read,bad_exe,missing_producer,empty_maps;
static char producer[512];
static unsigned now(void){return fake_time;}
static int fixture_textread(const char *path,char *out,unsigned size){
 if(!strcmp(path,"/tmp/disc-runtime/bt-decoder")){
  producer_reads++;
  if(missing_producer)return 0;snprintf(out,size,"%s",producer);return (int)strlen(out);
 }
 if(strstr(path,"/task/")){thread_reads++;snprintf(out,size,"4003 0 0 0 0 0 0 %x 1234",STACK_ADDRESS);return (int)strlen(out);}
 return 0;
}
static long fixture_readlink(const char *path,char *out,unsigned size){
 (void)path;const char *name=bad_exe?"/tmp/not-bluealsa":"/usr/bin/bluealsa";unsigned n=(unsigned)strlen(name);assert(n<size);memcpy(out,name,n);return n;
}
static int fixture_open(const char *path,int flags,...){(void)flags;assert(strstr(path,"/mem"));mem_opens++;return 7;}
static int fixture_close(int fd){assert(fd==7);closed++;return 0;}
static void *fixture_fopen(const char *path,const char *mode){assert(strstr(path,"/maps")&&!strcmp(mode,"r"));maps_opens++;map_row=0;return (void *)1;}
static char *fixture_fgets(char *out,int size,void *stream){
 assert(stream==(void *)1);maps_reads++;
 if(empty_maps||map_row++>=full_map_rows)return 0;
 if(map_row==1)snprintf(out,(unsigned)size,"%x-%x rw-p 0 0:0 0\n",BASE_ADDRESS,BASE_ADDRESS+(unsigned)sizeof memory);
 else snprintf(out,(unsigned)size,"1000-2000 ---p 0 0:0 0\n");
 return out;
}
static int fixture_fclose(void *stream){assert(stream==(void *)1);return 0;}
static int fixture_pread(int fd,void *out,unsigned n,long address){
 assert(fd==7);reads++;fake_time+=read_cost;
 assert(address>=BASE_ADDRESS&&(unsigned long)address+n<=BASE_ADDRESS+sizeof memory);
 if(short_read)return (int)n-1;
 memcpy(out,memory+(unsigned)address-BASE_ADDRESS,n);
 if((unsigned)address==HANDLE_ADDRESS&&n==120){header_reads++;if(mutate&&!(header_reads&1))((unsigned *)out)[20]++;}
 return (int)n;
}
#define textread fixture_textread
#define readlink fixture_readlink
#define open fixture_open
#define close fixture_close
#define fopen fixture_fopen
#define fgets fixture_fgets
#define fclose fixture_fclose
#define pread fixture_pread
#include "ldac_probe.h"
#undef textread
#undef readlink
#undef open
#undef close
#undef fopen
#undef fgets
#undef fclose
#undef pread

static void fixture_reset(void){
 decoderclose();memset(memory,0,sizeof memory);snprintf(producer,sizeof producer,"100\n101\n");
 fake_time=1000;read_cost=0;full_map_rows=1;map_row=0;
 reads=maps_reads=maps_opens=mem_opens=thread_reads=header_reads=closed=0;
 producer_reads=0;
 mutate=short_read=bad_exe=missing_producer=empty_maps=0;
}
static unsigned *at(unsigned address){return (unsigned *)(void *)(memory+address-BASE_ADDRESS);}
static void fixture_handle(unsigned word,unsigned rate,unsigned tier){
 unsigned *h=at(HANDLE_ADDRESS);h[0]=INNER_ADDRESS;h[1]=2;h[4]=rate;h[5]=2;h[13]=128;h[20]=tier;h[29]=1;
 at(STACK_ADDRESS)[word]=HANDLE_ADDRESS;
}
static unsigned call(unsigned rate){
 unsigned before_reads=reads,before_maps=maps_reads,before_threads=thread_reads;
 unsigned result=decoder_bitrate(rate);
 assert(reads-before_reads<=193);assert(maps_reads-before_maps<=DISC_LDAC_MAP_ROWS);
 assert(thread_reads-before_threads<=1);fake_time+=250;return result;
}
static unsigned find(unsigned rate,unsigned limit){unsigned result=0;while(limit--&&!(result=call(rate))){}return result;}

int main(void){
 fixture_reset();fixture_handle(140,96000,990);full_map_rows=100;
 assert(!call(96000)&&decoder_map_rows==32);assert(!call(96000)&&decoder_map_rows==64);
 assert(!call(96000)&&decoder_map_rows==96);assert(!call(96000)&&decoder_maps_ready);
 assert(!call(96000)&&decoder_word==64);assert(!call(96000)&&decoder_word==128);assert(call(96000)==990);
 unsigned old_maps=maps_reads,old_opens=maps_opens,old_threads=thread_reads,old_mem=mem_opens;
 for(unsigned i=0;i<100;i++){unsigned before=reads;assert(call(96000)==990&&reads-before==3);}
 assert(maps_reads==old_maps&&maps_opens==old_opens&&thread_reads==old_threads&&mem_opens==old_mem);
 at(HANDLE_ADDRESS)[20]=660;assert(call(96000)==660);at(HANDLE_ADDRESS)[20]=330;assert(call(96000)==330);

 /* A rate transition invalidates the old candidate; it is never reported for
  * the new rate. Discovery resumes with fresh maps/stack and the same guards. */
 assert(!call(48000)&&!decoder_handle);at(HANDLE_ADDRESS)[4]=48000;at(HANDLE_ADDRESS)[20]=660;
 assert(find(48000,20)==660);
 snprintf(producer,sizeof producer,"100\n102\n");memset(at(STACK_ADDRESS),0,2048);
 unsigned before=reads;assert(!call(48000)&&reads==before&&!decoder_handle);

 fixture_reset();fixture_handle(0,96000,990);bad_exe=1;assert(!call(96000)&&!mem_opens&&decoder_retry_ms==500);
 fake_time=decoder_retry_at+100;assert(!call(96000)&&!mem_opens);
 bad_exe=0;fake_time=decoder_retry_at+decoder_retry_ms;assert(find(96000,12)==990);
 missing_producer=1;assert(!call(96000)&&decoderfd==-1&&!decoder_handle);
 fixture_reset();snprintf(producer,sizeof producer,"4294967296\n101\n");assert(!call(96000)&&!mem_opens);

 /* Every miss is bounded, including the worst case where all 64 candidates
  * pass the first two reads but change before the consistency reread. */
 fixture_reset();fixture_handle(0,96000,990);for(unsigned i=0;i<512;i++)at(STACK_ADDRESS)[i]=HANDLE_ADDRESS;mutate=1;
 assert(!call(96000));before=reads;assert(!call(96000)&&reads-before==193&&decoder_word==64);
 for(unsigned i=0;i<7;i++)assert(!call(96000));assert(decoder_retry_ms==500&&decoderfd==-1);
 old_maps=maps_reads;old_mem=mem_opens;fake_time=decoder_retry_at+100;
 assert(!call(96000)&&maps_reads==old_maps&&mem_opens==old_mem);
 for(unsigned retry=1;retry<4;retry++){
  fake_time=decoder_retry_at+decoder_retry_ms;
  for(unsigned i=0;i<9;i++)assert(!call(96000));
  assert(decoder_retry_ms==(500u<<retry));
 }
 mutate=0;fake_time=decoder_retry_at+decoder_retry_ms;assert(find(96000,10)==990&&!decoder_failures&&!decoder_retry_ms);

 fixture_reset();fixture_handle(0,96000,990);assert(!call(96000));read_cost=1;
 before=reads;assert(call(96000)==990&&reads-before==4); /* one stack + one verified candidate */
 fixture_reset();fixture_handle(0,96000,990);assert(!call(96000));short_read=1;
 assert(!call(96000)&&!decoder_handle&&decoder_retry_ms==500);
 fixture_reset();at(STACK_ADDRESS)[0]=HANDLE_ADDRESS+1;at(STACK_ADDRESS)[1]=0x7ffffffcu;
 assert(!call(96000));before=reads;assert(!call(96000)&&reads-before==1);
 fixture_reset();empty_maps=1;assert(!call(96000)&&decoder_retry_ms==500);

 fixture_reset();unsigned used=(unsigned)snprintf(producer,sizeof producer,"100\n");
 for(unsigned i=101;i<=116;i++)used+=(unsigned)snprintf(producer+used,sizeof producer-used,"%u\n",i);
 assert(!call(96000));
 for(unsigned i=0;i<128;i++)assert(!call(96000));
 assert(thread_reads==16&&decoder_retry_ms==500);

 /* Thread lists are bounded and a changed list cancels even a long backoff. */
 fixture_reset();snprintf(producer,sizeof producer,"100\n");assert(!call(96000)&&decoder_retry_ms==500);
 snprintf(producer,sizeof producer,"100\n101\n");fixture_handle(1,96000,990);assert(find(96000,4)==990);

 /* Model the actual visible-page cadence: metadata starts discovery once,
  * then the existing 16ms callback advances only pending work. Calling both
  * entry points in one callback must never advance two search batches. */
 fixture_reset();fixture_handle(500,96000,990);full_map_rows=100;
 unsigned began=fake_time,result=0,cold_one_thread_ms=0;
 for(unsigned tick=0;tick<20&&!result;tick++){
  unsigned before_maps=maps_reads,before_reads=reads,before_threads=thread_reads;
  if(!tick)result=decoder_bitrate(96000);
  if(decoder_probe_pending(now()))result=decoder_bitrate(96000);
  unsigned after_maps=maps_reads,after_reads=reads,after_threads=thread_reads;
  assert(!decoder_probe_pending(now()));
  if(!result)assert(!decoder_bitrate(96000)); /* redundant same-time metadata */
  assert(maps_reads==after_maps&&reads==after_reads&&thread_reads==after_threads);
  assert(maps_reads-before_maps<=32&&reads-before_reads<=193&&thread_reads-before_threads<=1);
  if(result)cold_one_thread_ms=fake_time-began;else fake_time+=16;
 }
 assert(result==990&&cold_one_thread_ms<=192);

 fixture_reset();used=(unsigned)snprintf(producer,sizeof producer,"100\n");
 for(unsigned i=101;i<=116;i++)used+=(unsigned)snprintf(producer+used,sizeof producer-used,"%u\n",i);
 began=fake_time;assert(!decoder_bitrate(96000));
 for(unsigned tick=0;tick<128;tick++){
  fake_time+=16;assert(decoder_probe_pending(now()));assert(!decoder_bitrate(96000));
  assert(!decoder_probe_pending(now()));
 }
 unsigned miss_16_threads_ms=fake_time-began;assert(miss_16_threads_ms==2048&&decoder_retry_ms==500);
 before=producer_reads;old_maps=maps_reads;old_mem=mem_opens;unsigned before_threads=thread_reads;
 for(unsigned tick=0;tick<31;tick++){fake_time+=16;assert(!decoder_probe_pending(now()));}
 assert(producer_reads==before&&maps_reads==old_maps&&mem_opens==old_mem&&thread_reads==before_threads);
 fake_time=decoder_retry_at+decoder_retry_ms;assert(decoder_probe_pending(now()));
 decoderclose();
 puts("LDAC production probe: cache/rate/thread/identity/short-read/backoff/budgets PASS");
 puts("Cache: 3 guarded preads, 0 maps/stack refreshes; search: <=32 maps rows OR <=64 candidates + 1 stack per poll (<=193 preads).");
 printf("Visible 16ms cadence: cold single-thread last-word lookup %u ms; complete 16-thread miss %u ms; duplicate-step and backoff I/O suppression PASS.\n",cold_one_thread_ms,miss_16_threads_ms);
 return 0;
}
