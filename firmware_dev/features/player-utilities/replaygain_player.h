/* ReplayGain is attached to the exact native local decoder worker/read.
 * Thus preloaded next-track tags cannot affect queued previous-track PCM.
 * Read callbacks do no added file I/O, allocation or metadata parsing. */
#include "replaygain_core.h"
#include "player_guard.h"
extern int pthread_key_create(unsigned *,void (*)(void *)),pthread_setspecific(unsigned,const void *);
extern void *pthread_getspecific(unsigned);
static int (*rg_real_open)(const char *,const char *,const void *,unsigned);
static int (*rg_real_read)(int,void *,unsigned),(*rg_real_close)(int);
static unsigned rg_mutex[10],rg_tls_key,rg_hook_ready,rg_worker_ready,rg_checked,rg_revisions,rg_clock_seen;
static volatile unsigned rg_active;static unsigned rg_tag_reads,rg_pcm_blocks,rg_pcm_bytes,rg_last_source,rg_last_protected,rg_missing_peak;
static int rg_last_handle=-1;static float rg_last_db;static DiscRgConfig rg_config={0,1,0};
typedef struct {int handle;unsigned occupied,revision;char path[1024];DiscRgTags tags;DiscRgGain gain;} DiscRgRecord;
static DiscRgRecord rg_records[4];static unsigned rg_slot;
typedef struct {const unsigned *native;int handle;DiscRgGain gain;DiscRgRamp ramp;unsigned revision;} DiscRgWorker;
static int rg_prefix(const char *path,const char *prefix){if(!path)return 0;for(unsigned i=0;prefix[i];i++)if(path[i]!=prefix[i])return 0;return 1;}
static int rg_local_path(const char *path){return rg_prefix(path,"/tmp/sdcard/")||rg_prefix(path,"/usr/data/");}
static void rg_refresh_record(DiscRgRecord *record){record->gain=disc_rg_gain(&record->tags,&rg_config);record->revision=++rg_revisions;}
__attribute__((noinline)) static int rg_open(const char *path,const char *resample,const void *format,unsigned rate){
 int handle=rg_real_open(path,resample,format,rate);if(handle<0||!rg_local_path(path))return handle;
 unsigned length=0;while(length<1024&&path[length])length++;if(length>=1024)return handle;
 DiscRgRecord record={0};record.handle=handle;record.occupied=1;for(unsigned i=0;i<=length;i++)record.path[i]=path[i];
 if(rg_active){record.tags=disc_rg_tags(path);__sync_add_and_fetch(&rg_tag_reads,1);}
 pthread_mutex_lock(rg_mutex);unsigned slot=rg_slot++%4;for(unsigned i=0;i<4;i++)if(rg_records[i].occupied&&rg_records[i].handle==handle)slot=i;
 rg_refresh_record(&record);rg_records[slot]=record;pthread_mutex_unlock(rg_mutex);return handle;
}
static int rg_close(int handle){
 pthread_mutex_lock(rg_mutex);for(unsigned i=0;i<4;i++)if(rg_records[i].occupied&&rg_records[i].handle==handle)rg_records[i].occupied=0;if(rg_last_handle==handle){rg_last_handle=-1;rg_last_source=0;rg_last_db=0;rg_last_protected=rg_missing_peak=0;}pthread_mutex_unlock(rg_mutex);return rg_real_close(handle);
}
static int rg_read_pcm(int handle,void *data,unsigned count){
 int bytes=rg_real_read(handle,data,count);if(bytes<=0||(unsigned)bytes>count||pc_workmode()!=1)return bytes;
 DiscRgWorker *w=pthread_getspecific(rg_tls_key);if(!w||!w->native||w->native[3])return bytes; /* Native DSD conversion / packet data. */
 /* Stock AudioCodecRead normalizes 16/24/32-bit sources to signed S32LE.
  * The verified native worker describes this decoded container, not tags.
  * Refuse any unexpected native format rather than reinterpret its bytes. */
 unsigned rate=w->native[0],bits=w->native[1],channels=w->native[2];if(rate<8000||rate>768000||bits!=32||channels<1||channels>8)return bytes;
 if(w->handle!=handle){w->handle=handle;w->gain=(DiscRgGain){1,0,0,0,0};w->ramp=rg_active?(DiscRgRamp){0}:(DiscRgRamp){.current=1,.started=1,.target=1};w->revision=0;}
 if(!rg_active){w->gain=(DiscRgGain){1,0,0,0,0};disc_rg_pcm_bytes(&w->ramp,data,(unsigned)bytes,rate,bits,channels,1);return bytes;}
 if(!pthread_mutex_trylock(rg_mutex)){
  int found=0;for(unsigned i=0;i<4;i++)if(rg_records[i].occupied&&rg_records[i].handle==handle){w->gain=rg_records[i].gain;w->revision=rg_records[i].revision;found=1;break;}
  if(!found)w->gain=(DiscRgGain){1,0,0,0,0};pthread_mutex_unlock(rg_mutex);
 }
 disc_rg_pcm_bytes(&w->ramp,data,(unsigned)bytes,rate,bits,channels,w->gain.gain);
 rg_last_handle=handle;rg_last_source=w->gain.source;rg_last_db=w->gain.db;rg_last_protected=w->gain.protected_peak;rg_missing_peak=w->gain.missing_peak;rg_pcm_blocks++;rg_pcm_bytes+=(unsigned)bytes;return bytes;
}
static void *rg_worker(void *arg){
 DiscRgWorker worker={0};worker.native=arg;worker.handle=-1;
 int context=rg_hook_ready&&!pthread_setspecific(rg_tls_key,&worker);
 void *result=PC_FN(0x44b0f0,void *(*)(void *))(arg);if(context)pthread_setspecific(rg_tls_key,0);return result;
}
static void rg_install(void){
 if(rg_hook_ready)return;for(unsigned i=0;i<sizeof(pu_player_guards)/sizeof(pu_player_guards[0]);i++)if(memcmp((const void *)(unsigned long)pu_player_guards[i].address,pu_player_guards[i].bytes,16))return;
 rg_real_open=dlsym((void *)0,"AudioCodecOpen");rg_real_read=dlsym((void *)0,"AudioCodecRead");rg_real_close=dlsym((void *)0,"AudioCodecClose");
 if(!rg_real_open||!rg_real_read||!rg_real_close||pthread_key_create(&rg_tls_key,0))return;
 PC_WORD(PU_CODEC_OPEN_GOT)=(unsigned)rg_open;PC_WORD(PU_CODEC_READ_GOT)=(unsigned)rg_read_pcm;PC_WORD(PU_CODEC_CLOSE_GOT)=(unsigned)rg_close;rg_hook_ready=1;
}
static void rg_tick(void){
 unsigned now=pc_seconds();if(rg_clock_seen&&now==rg_checked)return;rg_clock_seen=1;rg_checked=now;
 char b[192];DiscRgConfig next={0,1,0};int f=open("/usr/data/disc-runtime/module-data/disc-bt-meter/replaygain.conf",0);
 if(f>=0){int n=read(f,b,sizeof b-1);close(f);if(n>0&&n<(int)sizeof b-1){b[n]=0;if(!disc_rg_config_parse(b,&next))next=(DiscRgConfig){0,1,0};}}
 unsigned on=pc_enabled()&&next.mode;
 pthread_mutex_lock(rg_mutex);int reread=!rg_active&&on,changed=rg_config.mode!=next.mode||rg_config.preamp!=next.preamp||rg_config.protect!=next.protect;rg_config=next;rg_active=on;pthread_mutex_unlock(rg_mutex);
 if(reread)for(unsigned i=0;i<4;i++){
  DiscRgRecord old;pthread_mutex_lock(rg_mutex);old=rg_records[i];pthread_mutex_unlock(rg_mutex);if(!old.occupied)continue;
  DiscRgTags tags=disc_rg_tags(old.path);rg_tag_reads++;pthread_mutex_lock(rg_mutex);if(rg_records[i].occupied&&rg_records[i].handle==old.handle&&rg_records[i].revision==old.revision)rg_records[i].tags=tags;pthread_mutex_unlock(rg_mutex);
 }
 if(changed||reread){pthread_mutex_lock(rg_mutex);for(unsigned i=0;i<4;i++)if(rg_records[i].occupied)rg_refresh_record(&rg_records[i]);pthread_mutex_unlock(rg_mutex);}
 /* Gain/source is reported from the actual decoder callback, never inferred
  * from the requested UI switch. No filenames or personal metadata here. */
 int n=snprintf(b,sizeof b,"mode=%u\nready=%u\nactive=%u\nsource=%u\ngain_db=%.2f\npeak_limited=%u\nmissing_peak=%u\ndsd=%d\n",next.mode,rg_hook_ready&&rg_worker_ready,on,rg_last_source,(double)(on?rg_last_db:0),rg_last_protected,rg_missing_peak,dsp_is_dsd());
 char state[256];int z=snprintf(state,sizeof state,"%spid=%d\nupdated=%u\nwork_mode=%d\nblocks=%u\ntag_reads=%u\n",b,getpid(),now,pc_workmode(),rg_pcm_blocks,rg_tag_reads);
 if(n>0&&n<(int)sizeof b&&z>0&&z<(int)sizeof state){f=open("/tmp/disc-runtime/replaygain.state.new",1|0x100|0x200,0600);if(f>=0){int ok=write(f,state,z)==z;close(f);if(ok)rename("/tmp/disc-runtime/replaygain.state.new","/tmp/disc-runtime/replaygain.state");}}
}
