/* Exact V2.57 native cover ABI, installed only in the guarded PLAYER.
 * The private worker record is path[512], directory[512], picture_bytes,
 * generation. Only that record's admission hint is clamped; the native worker
 * retains all generation/online-cover/fallback/report/free handling. */
#include "cover_policy.h"
#include "flac_picture.h"
#include "cover_guard.h"
#ifndef DISC_COVER_STATE
#define DISC_COVER_STATE "/tmp/disc-runtime/embedded-cover.state"
#endif
typedef struct {const char *path;unsigned hint,metadata_bytes,bytes,width,height,sequence,expand,jpeg_active,decoded_width,decoded_height,jpeg_heap,jpeg_heap_rejected;int result;} DiscCoverJob;
static int (*cover_real_create)(void *,const void *,void *(*)(void *),void *);
static int (*cover_real_get)(const char *,void **,unsigned);
static void *(*cover_real_malloc)(unsigned);
static int (*cover_key_create)(unsigned *,void (*)(void *)),(*cover_key_set)(unsigned,const void *);
static void *(*cover_key_get)(unsigned);
static unsigned cover_key,cover_sequence,cover_report_mutex[10];
static int cover_installed;
static void cover_report(const DiscCoverJob *);
static int cover_sync(const char *path,const char *directory,const char *artist,unsigned hint){
 DiscCoverJob job={0};job.path=path;job.hint=hint;job.sequence=__sync_add_and_fetch(&cover_sequence,1);
 if(pc_enabled()&&path&&!cover_large_hint(hint))job.metadata_bytes=cover_flac_picture_bytes(path);
 int admitted=pc_enabled()&&(cover_large_hint(hint)||job.metadata_bytes);
 int context=pc_enabled()&&!cover_key_set(cover_key,&job);if(!context)admitted=0;job.expand=admitted;
 int result=PC_FN(0x462440,int (*)(const char *,const char *,const char *,unsigned))(path,directory,artist,admitted?DISC_COVER_STOCK_LIMIT-1:hint);
 if(context)cover_key_set(cover_key,0);
 cover_report(&job);return result;
}
static void cover_report(const DiscCoverJob *job){
 char b[512];int n=snprintf(b,sizeof b,"version=0.1.3\npid=%d\nhook=%d\nsequence=%u\nhint=%u\nmetadata_bytes=%u\nbytes=%u\nwidth=%u\nheight=%u\ndecoded_width=%u\ndecoded_height=%u\njpeg_heap=%u\njpeg_heap_rejected=%u\nextract_result=%d\nmax_bytes=%u\nmax_png_pixels=%u\nmax_jpeg_pixels=%u\n",getpid(),cover_installed,job?job->sequence:0,job?job->hint:0,job?job->metadata_bytes:0,job?job->bytes:0,job?job->width:0,job?job->height:0,job?job->decoded_width:0,job?job->decoded_height:0,job?job->jpeg_heap:0,job?job->jpeg_heap_rejected:0,job?job->result:0,DISC_COVER_MAX_BYTES,DISC_COVER_MAX_PIXELS,DISC_COVER_JPEG_MAX_PIXELS);
 pthread_mutex_lock(cover_report_mutex);
 int f=open(DISC_COVER_STATE ".new",1|0x100|0x200,0600);
 if(f>=0){int ok=write(f,b,n)==n;close(f);if(ok)rename(DISC_COVER_STATE ".new",DISC_COVER_STATE);}
 pthread_mutex_unlock(cover_report_mutex);
}
static int cover_get(const char *path,void **out,unsigned limit){
 DiscCoverJob *job=cover_key_get(cover_key);
 if(!job||job->path!=path||limit!=DISC_COVER_STOCK_LIMIT||!pc_enabled())return cover_real_get(path,out,limit);
 int n=cover_real_get(path,out,job->expand?DISC_COVER_MAX_BYTES:limit);job->result=n;
 if(n>0&&out&&*out){
  job->bytes=(unsigned)n;
  if((job->expand||(unsigned)n>=DISC_COVER_STOCK_LIMIT)&&!cover_large_image_ok(*out,(unsigned)n,&job->width,&job->height)){
   PC_FN(0x4ce8c0,void (*)(void **))(out);job->result=-2;return -2;
  }
 }
 return n;
}
/* Pinned IJG v9 ABI: scale_num/denom at48/52, output dimensions at112/116.
 * Native loader returns source dimensions; repair its OUT parameters only,
 * leaving the decoder's source geometry intact during scanline/finish calls. */
static void *cover_malloc(unsigned bytes){
 DiscCoverJob *job=cover_key_get(cover_key);
 if(job&&job->jpeg_active){if(bytes>DISC_COVER_JPEG_HEAP_LIMIT-job->jpeg_heap){job->jpeg_heap_rejected++;return 0;}job->jpeg_heap+=bytes;}
 return cover_real_malloc(bytes);
}
static int cover_jpeg_start(unsigned *cinfo){
 DiscCoverJob *job=cover_key_get(cover_key);
 if(job&&job->jpeg_active){unsigned d=cover_jpeg_scale(cinfo[7],cinfo[8]);cinfo[12]=1;cinfo[13]=d;}
 int result=PC_FN(0x636084,int (*)(unsigned *))(cinfo);
 if(result&&job&&job->jpeg_active){job->decoded_width=cinfo[28];job->decoded_height=cinfo[29];}
 return result;
}
static int cover_jpeg_load(const void *data,void **out,unsigned bytes,unsigned *width,unsigned *height,int *components){
 DiscCoverJob *job=cover_key_get(cover_key);unsigned w=0,h=0;
 if(job&&pc_enabled()){
  if(!cover_large_image_ok(data,bytes,&w,&h)){*out=0;return -1;}
  job->width=w;job->height=h;job->jpeg_active=1;job->decoded_width=job->decoded_height=job->jpeg_heap=job->jpeg_heap_rejected=0;
 }
 int result=PC_FN(0x47e848,int (*)(const void *,void **,unsigned,unsigned *,unsigned *,int *))(data,out,bytes,width,height,components);
 if(job&&job->jpeg_active){job->jpeg_active=0;if(!result&&out&&*out){*width=job->decoded_width;*height=job->decoded_height;}}
 return result;
}
static void *cover_native_worker(void *arg){
 DiscCoverJob job={0};unsigned *hint=(unsigned *)((char *)arg+0x400);job.path=arg;job.hint=*hint;
 job.sequence=__sync_add_and_fetch(&cover_sequence,1);
 if(!cover_large_hint(job.hint))job.metadata_bytes=cover_flac_picture_bytes(job.path);
 if(!cover_large_hint(job.hint)&&!job.metadata_bytes){void *result=PC_FN(0x44ea10,void *(*)(void *))(arg);cover_report(&job);return result;}
 if(cover_key_set(cover_key,&job))return PC_FN(0x44ea10,void *(*)(void *))(arg);
 job.expand=1;
 *hint=DISC_COVER_STOCK_LIMIT-1;
 void *result=PC_FN(0x44ea10,void *(*)(void *))(arg);
 cover_key_set(cover_key,0);cover_report(&job);return result;
}
static int cover_create(void *thread,const void *attr,void *(*start)(void *),void *arg){
 if(start==PC_FN(0x44b0f0,void *(*)(void *))&&arg&&rg_hook_ready)start=rg_worker;
 if(start==PC_FN(0x44ea10,void *(*)(void *))&&arg&&pc_enabled()){
  const char *path=arg;unsigned i=0;while(i<512&&path[i])i++;
  if(i&&i<512)start=cover_native_worker;
 }
 return cover_real_create(thread,attr,start,arg);
}
/* Explicit RAM-only diagnostic: PID FD exact-path. Refreshes only art for an
 * already-open local FLAC; never selects a track, starts PCM or changes volume. */
static void cover_refresh_tick(void){
 if(!cover_installed||!pc_enabled()||pc_workmode()!=1)return;
 char b[640],link[64],path[512];int f=open("/tmp/disc-runtime/cover-refresh.request",0);if(f<0)return;
 int n=read(f,b,sizeof b-1);close(f);unlink("/tmp/disc-runtime/cover-refresh.request");if(n<=0)return;b[n]=0;
 char *end=0;long pid=strtol(b,&end,10);if(pid!=getpid()||*end!=' ')return;
 long fd=strtol(end+1,&end,10);if(fd<0||fd>1024||*end!='\n')return;const char *expected=end+1;
 snprintf(link,sizeof link,"/proc/self/fd/%ld",fd);n=readlink(link,path,sizeof path-1);if(n<=0||(unsigned)n>=sizeof path-1)return;path[n]=0;
 if(strcmp(path,expected)||!cover_flac_picture_bytes(path))return;
 cover_sync(path,0,0,0);
}
static void cover_install(void){
 if(cover_installed||!pc_enabled())return;
 for(unsigned i=0;i<sizeof(cover_guards)/sizeof(cover_guards[0]);i++)if(memcmp((const void *)cover_guards[i].address,cover_guards[i].bytes,16))return;
 cover_real_create=(int (*)(void *,const void *,void *(*)(void *),void *))dlsym((void *)0,"pthread_create");
 cover_real_get=(int (*)(const char *,void **,unsigned))dlsym((void *)0,"AudioCodecGetPicBytesNew");
 /* MIPS executable's canonical malloc symbol may be its own import stub.
  * Resolve libc explicitly before replacing that stub's GOT, avoiding recursion. */
 void *libc=dlopen("libc.so.6",2);if(!libc)return;
 cover_real_malloc=(void *(*)(unsigned))dlsym(libc,"malloc");
 cover_key_create=(int (*)(unsigned *,void (*)(void *)))dlsym((void *)0,"pthread_key_create");
 cover_key_get=(void *(*)(unsigned))dlsym((void *)0,"pthread_getspecific");
 cover_key_set=(int (*)(unsigned,const void *))dlsym((void *)0,"pthread_setspecific");
 if(!cover_real_create||!cover_real_get||!cover_real_malloc||!cover_key_create||!cover_key_get||!cover_key_set||cover_key_create(&cover_key,0))return;
 /* The active V2.57 path calls metadata_extract_local_cover synchronously.
  * Replace its single verified call with a standard MIPS t9 thunk. Original
  * function, generation checks, fallback, native decode and report stay native. */
 const unsigned sites[]={0x44f5f8,0x47e930,0x480bbc},originals[]={0x0c118910,0x0c18d821,0x0c11fa12},pages[]={0x44f000,0x47e000,0x480000};
 const unsigned targets[]={(unsigned)cover_sync,(unsigned)cover_jpeg_start,(unsigned)cover_jpeg_load};
 for(unsigned i=0;i<3;i++)if(PC_WORD(sites[i])!=originals[i])return;
 unsigned *stub=mmap((void *)0x07905000,4096,3,0x802,-1,0);
 if(stub==(void *)-1)return;if((unsigned)stub>=0x10000000u){munmap(stub,4096);return;}
 for(unsigned i=0;i<3;i++){unsigned target=targets[i];stub[i*4]=0x3c190000|(target>>16);stub[i*4+1]=0x37390000|(target&65535);stub[i*4+2]=0x03200008;stub[i*4+3]=0;}
 if(cacheflush(stub,48,3)||mprotect(stub,4096,5)){munmap(stub,4096);return;}
 for(unsigned i=0;i<3;i++)if(mprotect((void *)(unsigned long)pages[i],4096,7)){for(unsigned j=0;j<i;j++)mprotect((void *)(unsigned long)pages[j],4096,5);munmap(stub,4096);return;}
 PC_WORD(COVER_PIC_GET_GOT)=(unsigned)cover_get;PC_WORD(COVER_PTHREAD_CREATE_GOT)=(unsigned)cover_create;
 PC_WORD(COVER_MALLOC_GOT)=(unsigned)cover_malloc;
 for(unsigned i=0;i<3;i++)PC_WORD(sites[i])=0x0c000000|(((unsigned)stub+i*16)>>2);__sync_synchronize();
 int ok=1;for(unsigned i=0;i<3;i++)if(cacheflush((void *)(unsigned long)pages[i],4096,3))ok=0;
 if(!ok){
  for(unsigned i=0;i<3;i++){PC_WORD(sites[i])=originals[i];cacheflush((void *)(unsigned long)pages[i],4096,3);}
  PC_WORD(COVER_PIC_GET_GOT)=(unsigned)cover_real_get;PC_WORD(COVER_PTHREAD_CREATE_GOT)=(unsigned)cover_real_create;
  PC_WORD(COVER_MALLOC_GOT)=(unsigned)cover_real_malloc;
  for(unsigned i=0;i<3;i++)mprotect((void *)(unsigned long)pages[i],4096,5);return;
 }
 for(unsigned i=0;i<3;i++)mprotect((void *)(unsigned long)pages[i],4096,5);pc_cover_tick=cover_refresh_tick;
 __sync_synchronize();cover_installed=1;rg_worker_ready=1;cover_report(0);
}
