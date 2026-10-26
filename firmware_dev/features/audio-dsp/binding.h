/* Pinned PLAYER binding: PCM before the native EQ; never consumes audio itself. */
#include "dsp_api.h"
#include "provider_guard.h"
extern int sscanf(const char *,const char *,...);
#include "mode_preferences.h"
#include "parameters.h"
extern float strtof(const char *,char **);
#include "profile_format.h"
#include "chain_format.h"
#include "shared_profile.h"
extern void *memcpy(void *,const void *,size_t);
extern void *memset(void *,int,size_t);
extern char *realpath(const char *,char *);
static const DiscDspApi *dsp_api;
static void *dsp_context,*dsp_library;
static unsigned dsp_mutex[10],dsp_checked,dsp_mode,dsp_rate,dsp_route,dsp_requested,dsp_masks[4],dsp_generation;
static unsigned dsp_processed,dsp_bypassed,dsp_busy,dsp_dop_skipped,dsp_failed,dsp_allocated_mask;
static unsigned dsp_max_process_us,dsp_max_cpu_us,dsp_max_chunk_frames,dsp_budget_exceeded;
static volatile unsigned dsp_reset_needed;
static int dsp_enabled;
#include "native_eq_accel.h"
static void *(*dsp_configured_create)(unsigned,unsigned,const char *,const float [12][12]);
static float dsp_params[4][12][12],dsp_applied_params[12][12];
static unsigned dsp_param_fading,dsp_param_fade_at;
static unsigned dsp_param_retry_at,dsp_effective_mask,dsp_applied_chain_hash,dsp_chain_error;
static unsigned dsp_last_attempt_mask,dsp_last_attempt_hash,dsp_shared_scope,dsp_shared_enabled;
static DiscChain dsp_chains[4];static unsigned dsp_chain_valid[4],dsp_chain_hashes[4];
static void *(*dsp_chain_create)(unsigned,const DiscChain *);
static unsigned (*dsp_chain_last_error)(void);
static unsigned dsp_hash_bytes(const void *data,unsigned size){const unsigned char *b=data;unsigned h=2166136261u;for(unsigned i=0;i<size;i++)h=(h^b[i])*16777619u;return h;}
#include "config_cache.h"
static unsigned dsp_time_us(unsigned clock){long t[2];return clock_gettime(clock,t)?0:(unsigned)t[0]*1000000u+(unsigned)t[1]/1000u;}
#include "audio_priority.h"
static unsigned dsp_live_rate(void){unsigned p=PC_WORD(0x83dfc4);return p?PC_WORD(p+0x1c):0;}
static int dsp_is_dsd(void){unsigned p=PC_WORD(0x83dfc4),mode=pc_workmode();return PC_BYTE(0x899060)||(mode==1&&p&&PC_WORD(p+0x18)!=0)||((mode==1||mode==4)&&p&&PC_WORD(p+0x2c)==2&&PC_WORD(p+0x58)==1&&PC_WORD(p+0x18)==0);}
#include "../dsp-resample/binding.h"
static void dsp_load(void){
 if(dsp_api||dsp_library)return;
 const char *selected="/usr/data/disc-runtime/components/disc-audio-dsp/current/libdisc-audio-dsp.so";char path[4096];if(!realpath(selected,path))return;
 int f=open(path,0);if(f<0)return;unsigned size=0,h=2166136261;char b[4096];int n;
 while((n=read(f,b,sizeof(b)))>0){size+=n;for(int i=0;i<n;i++)h=(h^(unsigned char)b[i])*16777619;}close(f);
 if(n<0||size!=DSP_PROVIDER_SIZE||h!=DSP_PROVIDER_FNV)return;
 // A mapped TF library pins the mount during stock UI remounts. Copy to RAM.
 const char *ram="/tmp/disc-runtime/audio-dsp-provider.so";f=open(path,0);if(f<0)return;
 int out=open("/tmp/disc-runtime/audio-dsp-provider.new",1|0x100|0x200,0600);if(out<0){close(f);return;}size=0;h=2166136261;int ok=1;
 while((n=read(f,b,sizeof(b)))>0){if(write(out,b,n)!=n){ok=0;break;}size+=n;for(int i=0;i<n;i++)h=(h^(unsigned char)b[i])*16777619;}close(f);close(out);
 if(!ok||n<0||size!=DSP_PROVIDER_SIZE||h!=DSP_PROVIDER_FNV||rename("/tmp/disc-runtime/audio-dsp-provider.new",ram)){unlink("/tmp/disc-runtime/audio-dsp-provider.new");return;}
 dsp_library=dlopen(ram,2);if(!dsp_library||!module_cache_sync(ram))return;
 const DiscDspApi *(*get_api)(void)=dlsym(dsp_library,"disc_dsp_get_api");if(get_api){const DiscDspApi *api=get_api();if(api&&api->abi==3&&api->create&&api->destroy&&api->process&&api->stats&&api->set_mask&&api->health&&api->diagnostics)dsp_api=api;}
 dsp_configured_create=dlsym(dsp_library,"disc_dsp_create_configured");
 native_eq_kernel=(DiscNativeEqKernel)dlsym(dsp_library,"disc_dsp_native_eq");
 dsp_chain_create=dlsym(dsp_library,"disc_dsp_create_chain");dsp_chain_last_error=dlsym(dsp_library,"disc_dsp_chain_error");
 const DiscResampleApi *(*get_src)(void)=dlsym(dsp_library,"disc_resample_get_api");if(get_src){const DiscResampleApi *api=get_src();if(api&&api->abi==2&&api->create&&api->destroy&&api->process&&api->stats&&api->diagnostics)dsp_src_api=api;}
 dsp_float_process=(DiscResampleProcessFloat)dlsym(dsp_library,"disc_dsp_process_float");
}
static void dsp_process_buffer_core(void *data,int bytes,int rate){
 dsp_audio_priority(disc_dsp_player_index(pc_workmode())<4&&dsp_enabled&&dsp_api&&dsp_context&&dsp_requested&&!dsp_is_dsd());
 if(disc_dsp_player_index(pc_workmode())>=4)return;
 if(dsp_is_dsd()){dsp_reset_needed=1;dsp_dop_skipped++;return;}
 // Examine packet markers even when the original classifier missed a block.
 if(pc_workmode()==4&&data&&bytes>=128){const unsigned char *b=data;int valid=1;unsigned first=b[3];
  if(first==5||first==250){for(unsigned i=0;i<16&&valid;i++)if(b[i*8+3]!=(i&1?(first^255):first)||b[i*8+7]!=b[i*8+3]||b[i*8]||b[i*8+4])valid=0;
   if(valid){dsp_reset_needed=1;dsp_dop_skipped++;return;}}
 }
 if(dsp_src_fade_mode==2&&data&&bytes>0&&!(bytes&7)){memset(data,0,(unsigned)bytes);dsp_bypassed++;return;}
 if(dsp_reset_needed&&dsp_src_context&&data&&bytes>0&&!(bytes&7)){memset(data,0,(unsigned)bytes);dsp_bypassed++;return;}
 unsigned pcm_player=PC_WORD(0x83dfc4);if(!pcm_player||PC_WORD(pcm_player+0x20)!=2||PC_WORD(pcm_player+0x24)!=32){dsp_reset_needed=1;dsp_bypassed++;return;}
 if(!dsp_enabled||!dsp_api||!dsp_context||dsp_reset_needed||rate<8000||rate>768000||!data||bytes<=0||(bytes&7)){dsp_bypassed++;return;}
 if(pthread_mutex_trylock(dsp_mutex)){dsp_busy++;if(dsp_src_context)memset(data,0,(unsigned)bytes);return;}
 if(!dsp_enabled||!dsp_context||dsp_mode!=(unsigned)pc_workmode()||dsp_rate!=(unsigned)rate||dsp_route!=pc_route())dsp_bypassed++;
 else {unsigned frames=(unsigned)bytes/8,begin=dsp_requested?dsp_time_us(1):0,cpu_begin=dsp_requested?dsp_time_us(3):0;if(dsp_src_context){if(!dsp_src_api->process(dsp_src_context,data,frames,dsp_float_process,dsp_context)){dsp_failed++;dsp_reset_needed=1;memset(data,0,(unsigned)bytes);}dsp_src_capture_stats();}else dsp_api->process(dsp_context,data,frames);dsp_processed++;if(begin){unsigned cost=dsp_time_us(1)-begin,cpu_cost=dsp_time_us(3)-cpu_begin;if(cost>dsp_max_process_us)dsp_max_process_us=cost;if(cpu_begin&&cpu_cost>dsp_max_cpu_us)dsp_max_cpu_us=cpu_cost;if(frames>dsp_max_chunk_frames)dsp_max_chunk_frames=frames;if(cost>(unsigned)((float)frames*(1000000.f/rate)))dsp_budget_exceeded++;}}
 pthread_mutex_unlock(dsp_mutex);
}
static void dsp_process_buffer(void *data,int bytes,int rate){
 unsigned epoch=dsp_generation;dsp_process_buffer_core(data,bytes,rate);
 if(!dsp_is_dsd()&&!dsp_reset_needed)dsp_src_fade_apply(data,bytes,rate,epoch);
}
static void dsp_tick(void){
 unsigned now=pc_seconds();if(now!=dsp_checked){dsp_checked=now;char b[40];int f=open("/usr/data/disc-runtime/components/disc-audio-dsp/current/FEATURES",0),n=f<0?0:read(f,b,sizeof(b)-1);if(f>=0)close(f);if(n<0)n=0;b[n]=0;
  dsp_enabled=!strcmp(b,"audio-dsp-abi3\n")&&!access("/usr/data/disc-runtime/components/disc-audio-dsp/enabled",0)&&access("/usr/data/disc-runtime/components/disc-audio-dsp/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0);
  dsp_config_poll();dsp_src_poll();
  if(dsp_enabled){dsp_load();if(dsp_api&&!dop_eq_ready)dop_eq_install();if(dsp_api&&native_eq_kernel&&!native_eq_ready)native_eq_install();}
 }
 unsigned mode=pc_workmode(),rate=dsp_live_rate(),route=pc_route(),index=disc_dsp_player_index(mode);unsigned requested=dsp_enabled&&index<4&&!dsp_is_dsd()?(dsp_shared_scope?(dsp_shared_enabled?DISC_DSP_CUSTOM:0):dsp_masks[index]):0;
 unsigned p=PC_WORD(0x83dfc4);if(!p||(PC_WORD(p+0x20)!=2||PC_WORD(p+0x24)!=32)||rate<8000||rate>768000)requested=0;
 // Native CPU measurements: retain headroom for the actual audio worker.
 if(rate>384000)requested&=~(DISC_DSP_CUSTOM|DISC_DSP_HALL|DISC_DSP_TUBE|DISC_DSP_V8|DISC_DSP_WARM|DISC_DSP_DRAGONFLY|DISC_DSP_GALACTIC|DISC_DSP_STEREO|DISC_DSP_DELAY|DISC_DSP_DEESSER|DISC_DSP_DYNAMIC_EQ);
 if(requested&DISC_DSP_CUSTOM)requested=DISC_DSP_CUSTOM;
 dsp_requested=requested;unsigned desired_hash=index<4?(requested==DISC_DSP_CUSTOM?dsp_chain_hashes[index]:dsp_hash_bytes(dsp_params[index],sizeof(dsp_applied_params))):0;
 int stream_changed=dsp_mode!=mode||dsp_rate!=rate||dsp_route!=route;
 int route_changed=dsp_reset_needed||stream_changed;
 unsigned processing_rate=requested?disc_src_target(&dsp_src_rules,rate):rate;
 int src_changed=processing_rate!=dsp_processing_rate;
 int src_transition=!stream_changed&&!dsp_is_dsd()&&(dsp_src_context||processing_rate!=rate);
 int changed=route_changed||src_changed||requested!=dsp_effective_mask||(requested==DISC_DSP_CUSTOM?desired_hash!=dsp_applied_chain_hash:(index<4&&memcmp(dsp_applied_params,dsp_params[index],sizeof(dsp_applied_params))));
 int retry_wait=!route_changed&&processing_rate==dsp_src_last_attempt_rate&&dsp_param_retry_at&&(unsigned)(now-dsp_param_retry_at)<5&&dsp_last_attempt_mask==requested&&dsp_last_attempt_hash==desired_hash;
 /* A quick second edit can cancel a pending transition before publication.
  * Resume the existing prepared engine instead of leaving its gain at zero. */
 if(!changed&&dsp_param_fading){dsp_param_fading=0;if(dsp_src_fade_mode)dsp_src_fade_up(dsp_generation,0);else if(dsp_context&&!pthread_mutex_trylock(dsp_mutex)){dsp_api->set_mask(dsp_context,dsp_effective_mask);pthread_mutex_unlock(dsp_mutex);}}
 if(changed&&!retry_wait&&dsp_api){
  if(src_transition&&!dsp_param_fading){dsp_src_fade_down();dsp_param_fading=1;dsp_param_fade_at=dsp_time_us(1);}
  else if(dsp_context&&!route_changed&&!dsp_param_fading){if(pthread_mutex_trylock(dsp_mutex))return;dsp_api->set_mask(dsp_context,0);pthread_mutex_unlock(dsp_mutex);dsp_param_fading=1;dsp_param_fade_at=dsp_time_us(1);}
  if(dsp_param_fading&&!route_changed&&(unsigned)(dsp_time_us(1)-dsp_param_fade_at)<100000)return;
  unsigned player_state=p?PC_WORD(p+0x48):0;
  if(src_transition&&dsp_src_fade_mode!=2&&!dsp_reset_needed&&player_state!=0&&player_state!=3)return;
  void *next=0,*next_src=0;unsigned error=0,created_src=0;
  if(requested==DISC_DSP_CUSTOM){if(index>=4||!dsp_chain_valid[index])error=4;else if(!dsp_chain_create)error=5;else{next=dsp_chain_create(processing_rate,&dsp_chains[index]);if(!next)error=dsp_chain_last_error?dsp_chain_last_error():3;}}
  else if(requested)next=dsp_configured_create&&index<4?dsp_configured_create(processing_rate,requested,"/usr/data/disc-runtime/components/disc-audio-dsp/current",dsp_params[index]):dsp_api->create(processing_rate,requested,".");
  if(requested&&!next&&!error)error=3;
  if(next&&requested!=DISC_DSP_CUSTOM){unsigned av,lat,clip;dsp_api->stats(next,&av,&lat,&clip);if((av&requested)!=requested){dsp_api->destroy(next);next=0;error=3;}}
  if(next&&processing_rate!=rate){
   if(!dsp_src_api||!dsp_float_process)error=5;
   else if(!route_changed&&dsp_src_context&&dsp_processing_rate==processing_rate)next_src=dsp_src_context;
   else {next_src=dsp_src_api->create(rate,processing_rate);created_src=next_src!=0;if(!next_src)error=3;}
   if(error){dsp_api->destroy(next);next=0;}
  }
  if(pthread_mutex_trylock(dsp_mutex)){if(next)dsp_api->destroy(next);if(created_src)dsp_src_api->destroy(next_src);return;}
  dsp_chain_error=error;dsp_last_attempt_mask=requested;dsp_last_attempt_hash=desired_hash;dsp_src_last_attempt_rate=processing_rate;
  if(error&&!route_changed&&dsp_context){dsp_failed++;dsp_param_retry_at=now;dsp_param_fading=0;if(src_transition)dsp_src_fade_up(dsp_generation,0);else dsp_api->set_mask(dsp_context,dsp_effective_mask);pthread_mutex_unlock(dsp_mutex);if(created_src)dsp_src_api->destroy(next_src);}
  else{void *old=dsp_context,*old_src=dsp_src_context;dsp_context=next;dsp_src_context=next_src;dsp_processing_rate=processing_rate;dsp_src_snapshot_sequence=0;dsp_mode=mode;dsp_rate=rate;dsp_route=route;dsp_allocated_mask=dsp_effective_mask=next?requested:0;dsp_reset_needed=0;dsp_generation++;dsp_param_fading=0;
   if(src_transition){unsigned latency=0,e=0,u=0,v=0;if(next_src)dsp_src_api->stats(next_src,&latency,&e,&u,&v);dsp_src_fade_up(dsp_generation,latency);}else dsp_src_fade_clear();
   if(error){dsp_failed++;dsp_param_retry_at=now;}else{dsp_param_retry_at=0;if(index<4)memcpy(dsp_applied_params,dsp_params[index],sizeof(dsp_applied_params));dsp_applied_chain_hash=requested==DISC_DSP_CUSTOM?desired_hash:0;}
   dsp_max_process_us=dsp_max_cpu_us=dsp_max_chunk_frames=dsp_budget_exceeded=0;pthread_mutex_unlock(dsp_mutex);if(old)dsp_api->destroy(old);if(old_src&&old_src!=next_src)dsp_src_api->destroy(old_src);
  }
 }
 if(dsp_context&&!dsp_src_context&&!dsp_param_fading&&!pthread_mutex_trylock(dsp_mutex)){dsp_api->set_mask(dsp_context,dsp_effective_mask);pthread_mutex_unlock(dsp_mutex);}
 static unsigned reported,available,latency,clipped,limited,invalid,report_generation;static float peak,gain=1;if(now!=reported){reported=now;if(!dsp_context||report_generation!=dsp_generation){available=latency=clipped=limited=invalid=0;peak=0;gain=1;report_generation=dsp_generation;}
  if(dsp_context&&dsp_src_context){DiscSrcSnapshot snapshot;if(dsp_src_read_stats(&snapshot)){available=snapshot.available;latency=snapshot.latency;clipped=snapshot.clipped;limited=snapshot.limited;invalid=snapshot.invalid;peak=snapshot.peak;gain=snapshot.gain;}}
  else if(dsp_context&&!dsp_src_fade_mode&&!pthread_mutex_trylock(dsp_mutex)){dsp_api->stats(dsp_context,&available,&latency,&clipped);dsp_api->diagnostics(dsp_context,&limited,&invalid,&peak,&gain);pthread_mutex_unlock(dsp_mutex);}
  char b[2048];int n=snprintf(b,sizeof(b),"version=1.17.0\nenabled=%d\nprovider=%d\nwork_mode=%u\nrate=%u\nroute=%u\nbt_mask=%u\nusbdac_mask=%u\ndlna_mask=%u\npure_mask=%u\nrequested=%u\navailable=%u\nDoP_bypass=%d\nprocessed=%u\nbypassed=%u\nbusy=%u\ndop_skipped=%u\nfailures=%u\ngeneration=%u\nlatency_frames=%u\nclipped=%u\nlimited_frames=%u\ninvalid_samples=%u\npre_guard_peak_milli=%u\nguard_gain_ppm=%u\nmax_process_us=%u\nmax_chunk_frames=%u\nbudget_exceeded=%u\nmax_cpu_us=%u\naudio_tid=%u\naudio_nice=%d\npriority_result=%d\nstate_time=%u\nplayer_pid=%d\napplied_mask=%u\napplied_params_hash=%u\ndesired_chain_hash=%u\napplied_chain_hash=%u\nchain_error=%u\nchain_capacity=16\nchain_active_limit=8\nshared_supported=1\nshared_scope=%u\nshared_master=%u\n",dsp_enabled,dsp_api!=0,mode,rate,route,dsp_masks[0],dsp_masks[1],dsp_masks[2],dsp_masks[3],requested,available,dsp_is_dsd(),dsp_processed,dsp_bypassed,dsp_busy,dsp_dop_skipped,dsp_failed,dsp_generation,latency,clipped,limited,invalid,(unsigned)(peak<1000000?peak*1000:1000000000),(unsigned)(gain*1000000),dsp_max_process_us,dsp_max_chunk_frames,dsp_budget_exceeded,dsp_max_cpu_us,dsp_audio_tid,dsp_audio_nice,dsp_priority_result,now,getpid(),dsp_effective_mask,dsp_hash_bytes(dsp_applied_params,sizeof(dsp_applied_params)),index<4?dsp_chain_hashes[index]:0,dsp_applied_chain_hash,dsp_chain_error,dsp_shared_scope,dsp_shared_enabled);
  n+=snprintf(b+n,sizeof b-(unsigned)n,"native_eq_accel=%d\nnative_eq_calls=%u\nnative_eq_fallbacks=%u\nnative_eq_stub=%u\n",native_eq_ready,native_eq_calls,native_eq_fallbacks,native_eq_stub_address);
  n+=snprintf(b+n,sizeof b-(unsigned)n,"processing_rate=%u\nsrc_enabled=%u\nsrc_active=%d\nsrc_config_error=%u\nsrc_quality=soxr_vhq28\nsrc_errors=%u\nsrc_underflows=%u\nsrc_transition=%u\n",dsp_processing_rate,dsp_src_rules.enabled,dsp_src_context!=0,dsp_src_config_error,dsp_src_snapshot.src_errors,dsp_src_snapshot.src_underflows,dsp_src_fade_mode);
  int f=open("/tmp/disc-runtime/audio-dsp.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}
 }
}
