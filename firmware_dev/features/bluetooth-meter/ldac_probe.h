#ifndef DISC_LDAC_PROBE_H
#define DISC_LDAC_PROBE_H
/* Passive LDAC tier discovery on the existing UI poll. The runtime pins both
 * accepted decoder executables by SHA256. Never scan a whole set of stacks in
 * one UI callback: maps and candidates advance independently across polls. */
#define DISC_LDAC_MAP_ROWS 32u
#define DISC_LDAC_CANDIDATES 64u
#define DISC_LDAC_BUDGET_MS 2u
#define DISC_LDAC_STEP_MS 16u
static int decoderfd=-1,decoderpid,decoder_tid;
static unsigned decoder_handle,dlow[160],dhigh[160],dn;
static int decoder_ids[17],decoder_count,decoder_thread;
static unsigned decoder_sampling,decoder_words[512],decoder_word,decoder_map_rows;
static unsigned decoder_retry_at,decoder_retry_ms,decoder_failures;
static unsigned decoder_step_at;
static int decoder_step_seen;
static int decoder_maps_ready,decoder_stack_ready;
static void *decoder_maps;
static int dcopy(unsigned p,void *b,unsigned n){
 if(decoderfd<0||p>0x7fffffffu-n)return 0;
 for(unsigned i=0;i<dn;i++)if(p>=dlow[i]&&p+n<=dhigh[i])return pread(decoderfd,b,n,p)==(int)n;
 return 0;
}
static void decoder_search_clear(void){
 if(decoder_maps)fclose(decoder_maps);decoder_maps=0;
 decoder_handle=0;decoder_tid=0;dn=decoder_map_rows=decoder_word=0;
 decoder_maps_ready=decoder_stack_ready=0;decoder_thread=1;
}
static void decoderclose(void){
 if(decoderfd>=0)close(decoderfd);decoderfd=-1;decoderpid=0;
 decoder_search_clear();decoder_count=0;decoder_sampling=0;
 decoder_retry_at=decoder_retry_ms=decoder_failures=0;
 decoder_step_at=0;decoder_step_seen=0;
}
static unsigned handle_bitrate(unsigned q,unsigned sampling){
 unsigned h[30],again[30],inner[6];if(q&3||q<0x800000||!dcopy(q,h,sizeof(h)))return 0;
 if(h[1]!=2||h[4]!=sampling||h[5]!=2||(h[13]!=128&&h[13]!=256)||h[29]!=1||h[20]<100||h[20]>1100||!dcopy(h[0],inner,sizeof(inner)))return 0;
 if(!dcopy(q,again,sizeof(again))||again[0]!=h[0]||again[1]!=2||again[4]!=sampling||again[20]!=h[20]||again[29]!=1)return 0;
 return h[20];
}
static void decoder_search_failed(unsigned t){
 decoder_search_clear();if(decoderfd>=0)close(decoderfd);decoderfd=-1;
 if(decoder_failures<4)decoder_failures++;
 decoder_retry_at=t;decoder_retry_ms=500u<<(decoder_failures-1);
}
static int decoder_over_budget(unsigned started){return now()-started>=DISC_LDAC_BUDGET_MS;}
/* Read-only fast-poll predicate. Cached tier verification remains with the
 * 250ms metadata poll; backoff causes no fast-poll file or process reads. */
static int decoder_probe_pending(unsigned t){
 return decoderpid>0&&decoder_count>1&&decoder_sampling&&!decoder_handle&&
  (!decoder_retry_ms||t-decoder_retry_at>=decoder_retry_ms)&&
  (!decoder_step_seen||t-decoder_step_at>=DISC_LDAC_STEP_MS);
}
static unsigned decoder_bitrate(unsigned sampling){
 char b[512],path[80],exe[64];int ids[17],count=0;unsigned started=now();
 if(!textread("/tmp/disc-runtime/bt-decoder",b,sizeof b)){decoderclose();return 0;}
 char *p=b;
 while(*p&&count<17){
  char *end;unsigned long id=strtoul(p,&end,10);
  if(end==p||!id||id>0x7ffffffful){decoderclose();return 0;}
  ids[count++]=(int)id;p=end;while(*p=='\n'||*p=='\r'||*p==' ')p++;
 }
 if(count<1||*p){decoderclose();return 0;}
 if(ids[0]!=decoderpid){decoderclose();decoderpid=ids[0];}
 int changed=count!=decoder_count;
 for(int i=0;!changed&&i<count;i++)if(ids[i]!=decoder_ids[i])changed=1;
 /* A live cached handle needs only the fresh producer thread list and three
  * guarded reads. Do not reopen/parse maps on every 250ms metadata refresh. */
 if(decoder_handle&&decoder_sampling==sampling){
  int alive=0;for(int i=1;i<count;i++)if(ids[i]==decoder_tid)alive=1;
  unsigned rate=alive?handle_bitrate(decoder_handle,sampling):0;
  if(rate)return rate;
  changed=1;
 }
 if(changed||decoder_sampling!=sampling){
  decoder_search_clear();decoder_count=count;decoder_sampling=sampling;
  for(int i=0;i<count;i++)decoder_ids[i]=ids[i];
  decoder_retry_at=decoder_retry_ms=decoder_failures=0;
 }
 if(decoder_retry_ms){if(started-decoder_retry_at<decoder_retry_ms)return 0;decoder_retry_ms=0;}
 /* Metadata and the visible LDAC fast poll can run in the same callback.
  * Both advance this one search, at most once per 16ms interval. */
 if(decoder_step_seen&&started-decoder_step_at<DISC_LDAC_STEP_MS)return 0;
 decoder_step_seen=1;decoder_step_at=started;
 if(decoder_count<2){decoder_search_failed(started);return 0;}
 if(decoderfd<0){
  snprintf(path,sizeof path,"/proc/%d/exe",decoderpid);long n=readlink(path,exe,sizeof exe-1);
  if(n<0||n>(long)sizeof exe-2){decoder_search_failed(started);return 0;}exe[n]=0;
  if(strcmp(exe,"/usr/bin/bluealsa")&&strcmp(exe,"/tmp/disc-runtime/lhdcv5-original/bluealsa")){decoder_search_failed(started);return 0;}
  snprintf(path,sizeof path,"/proc/%d/mem",decoderpid);decoderfd=open(path,0);
  if(decoderfd<0){decoder_search_failed(started);return 0;}
 }
 if(!decoder_maps_ready){
  if(!decoder_maps){snprintf(path,sizeof path,"/proc/%d/maps",decoderpid);decoder_maps=fopen(path,"r");if(!decoder_maps){decoder_search_failed(started);return 0;}}
  char line[256],permission[8];unsigned low,high,rows=0;int complete=0;
  while(rows<DISC_LDAC_MAP_ROWS&&dn<160&&decoder_map_rows<256&&!decoder_over_budget(started)){
   if(!fgets(line,sizeof line,decoder_maps)){complete=1;break;}rows++;decoder_map_rows++;
   if(sscanf(line,"%x-%x %4s",&low,&high,permission)==3&&permission[0]=='r'&&low<high&&high<=0x80000000u){dlow[dn]=low;dhigh[dn++]=high;}
  }
  if(complete||dn==160||decoder_map_rows==256){
   fclose(decoder_maps);decoder_maps=0;
   if(!dn){decoder_search_failed(started);return 0;}decoder_maps_ready=1;
  }
  return 0;
 }
 if(decoder_over_budget(started))return 0;
 if(!decoder_stack_ready){
  if(decoder_thread>=decoder_count){decoder_search_failed(started);return 0;}
  snprintf(path,sizeof path,"/proc/%d/task/%d/syscall",decoderpid,decoder_ids[decoder_thread]);unsigned sp,pc;
  if(!textread(path,b,sizeof b)||sscanf(b,"%*s %*s %*s %*s %*s %*s %*s %x %x",&sp,&pc)!=2||(sp&3)||!dcopy(sp,decoder_words,sizeof decoder_words)){
   decoder_thread++;if(decoder_thread>=decoder_count)decoder_search_failed(started);return 0;
  }
  decoder_word=0;decoder_stack_ready=1;
 }
 for(unsigned tried=0;tried<DISC_LDAC_CANDIDATES&&decoder_word<512&&!decoder_over_budget(started);tried++){
  unsigned rate=handle_bitrate(decoder_words[decoder_word++],sampling);
  if(rate){decoder_handle=decoder_words[decoder_word-1];decoder_tid=decoder_ids[decoder_thread];decoder_failures=decoder_retry_at=decoder_retry_ms=0;return rate;}
 }
 if(decoder_word==512){decoder_stack_ready=0;decoder_thread++;if(decoder_thread>=decoder_count)decoder_search_failed(started);}
 return 0;
}
#endif
