#ifndef DISC_DSP_CONFIG_CACHE_H
#define DISC_DSP_CONFIG_CACHE_H
/* This control-thread cache deliberately compares bytes, not mtime/inode or a
 * hash. Atomic replacement and equal-size edits are noticed without relying on
 * the MIPS libc stat layout. No file access occurs in the audio callback. */
typedef struct {unsigned seen,kind,size;} DiscDspConfigText;
static DiscDspConfigText dsp_settings_text,dsp_shared_text;
static DiscDspConfigText dsp_param_text[4],dsp_chain_text[4];
static char dsp_settings_bytes[40],dsp_shared_bytes[8192];
static char dsp_param_bytes[4][4096],dsp_chain_bytes[4][8192];
static unsigned dsp_config_source;
#ifndef DSP_CONFIG_PARSED
#define DSP_CONFIG_PARSED(kind) ((void)0)
#endif
/* kind 0: open failed (legacy fallback); 1: read failed; 2: readable.
 * Read errors retain the old empty/invalid-file behavior, never a stale chain. */
static unsigned dsp_config_read(const char *path,char *buf,unsigned cap,unsigned *kind){
 int f=open(path,0),n=f<0?0:read(f,buf,cap-1);*kind=f<0?0:n<0?1:2;
 if(f>=0)close(f);if(n<0)n=0;buf[n]=0;return (unsigned)n;
}
static int dsp_config_changed(DiscDspConfigText *cache,char *saved,const char *buf,unsigned n,unsigned kind){
 if(cache->seen&&cache->kind==kind&&cache->size==n&&!memcmp(saved,buf,n))return 0;
 cache->seen=1;cache->kind=kind;cache->size=n;memcpy(saved,buf,n+1);return 1;
}
static void dsp_config_poll(void){
 /* Component enable/safe-mode checks still run once a second in dsp_tick. */
 if(!dsp_enabled)return;
 char buf[8192];unsigned kind,n=dsp_config_read("/usr/data/disc-runtime/module-data/disc-remote-control/shared-profile.conf",buf,sizeof(buf),&kind);
 if(kind){
  int changed=dsp_config_changed(&dsp_shared_text,dsp_shared_bytes,buf,n,kind);
  if(dsp_config_source!=1)changed=1;dsp_config_source=1;dsp_shared_scope=1;
  if(!changed)return;
  DiscChain next;unsigned enabled=1;const char *payload=disc_shared_payload(buf,&enabled);
  DSP_CONFIG_PARSED(shared);int valid=payload&&disc_chain_parse(payload,&next);
  dsp_shared_enabled=valid?enabled:1;
  for(unsigned i=0;i<4;i++){
   dsp_chain_valid[i]=valid;dsp_chain_hashes[i]=disc_chain_hash(payload?payload:buf);
   if(valid)dsp_chains[i]=next;
  }
  return;
 }
 /* A shared file which opens but is corrupt must not enable an old mode chain.
  * Only the same open-failure condition used previously permits fallback. */
 if(dsp_config_source!=2)for(unsigned i=0;i<4;i++)dsp_chain_text[i].seen=0;
 dsp_config_source=2;dsp_shared_scope=dsp_shared_enabled=0;
 n=dsp_config_read("/usr/data/disc-runtime/module-data/disc-audio-dsp/settings",buf,sizeof(dsp_settings_bytes),&kind);
 if(dsp_config_changed(&dsp_settings_text,dsp_settings_bytes,buf,n,kind)){
  DSP_CONFIG_PARSED(settings);disc_dsp_read_masks(buf,dsp_masks);
 }
 if(!(dsp_masks[0]|dsp_masks[1]|dsp_masks[2]|dsp_masks[3]))return;
 for(unsigned mode=0;mode<4;mode++){
  char path[180];snprintf(path,sizeof(path),"/usr/data/disc-runtime/module-data/disc-remote-control/dsp-%u.params",mode);
  n=dsp_config_read(path,buf,sizeof(dsp_param_bytes[mode]),&kind);
  if(dsp_config_changed(&dsp_param_text[mode],dsp_param_bytes[mode],buf,n,kind)){
   float next[12][12];memcpy(next,disc_param_default,sizeof(next));
   if(n>2){DSP_CONFIG_PARSED(params);disc_profile_parse(buf,next);}
   memcpy(dsp_params[mode],next,sizeof(next));
  }
  snprintf(path,sizeof(path),"/usr/data/disc-runtime/module-data/disc-remote-control/chain-%u.conf",mode);
  n=dsp_config_read(path,buf,sizeof(dsp_chain_bytes[mode]),&kind);
  if(dsp_config_changed(&dsp_chain_text[mode],dsp_chain_bytes[mode],buf,n,kind)){
   dsp_chain_hashes[mode]=n?disc_chain_hash(buf):0;
   DSP_CONFIG_PARSED(chain);dsp_chain_valid[mode]=n&&disc_chain_parse(buf,&dsp_chains[mode]);
  }
 }
}
#endif
