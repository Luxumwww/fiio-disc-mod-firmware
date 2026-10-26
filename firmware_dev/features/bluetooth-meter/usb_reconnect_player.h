/* Original V2.57 USB output lifecycle. The existing 100ms PLAYER worker is
 * the sole owner of USB PCM rebuilding; commands only publish USB intent.
 * No BT input restart, Play command, analogue fallback, or new audio thread. */
static unsigned pc_usb_mutex[10];
static int (*pc_usb_pcm_close)(void *),(*pc_usb_pcm_is_ready)(void *);
static unsigned pc_usb_generation,pc_usb_pcm_generation,pc_usb_bound_generation;
static unsigned pc_usb_identity,pc_usb_stable,pc_usb_attempts,pc_usb_retry_at;
static int pc_usb_present,pc_usb_epoch_valid,pc_usb_ready,pc_usb_recovering,pc_usb_error;
static int pc_usb_need_rebind,pc_usb_was_owned,pc_usb_guarded,pc_usb_real_open,pc_usb_pcm_state=-1;
static int pc_usb_fd=-1;
static int pc_usb_card=-1;
static unsigned pc_usb_rebuilds;
static int pc_usb_read(const char *path,char *out,unsigned size){
 if(size<2)return 0;int fd=open(path,0);if(fd<0){out[0]=0;return 0;}
 int n=read(fd,out,size-1);close(fd);if(n<0)n=0;out[n]=0;
 while(n>0&&(out[n-1]=='\n'||out[n-1]=='\r'||out[n-1]==' '))out[--n]=0;return n;
}
static unsigned pc_usb_hash(unsigned h,const char *s){for(unsigned i=0;s[i];i++)h=(h^(unsigned char)s[i])*16777619u;return (h^255u)*16777619u;}
static int pc_usb_decimal(const char *s){if(!s||!s[0])return 0;int nonzero=0;for(unsigned i=0;s[i];i++){if(s[i]<'0'||s[i]>'9')return 0;if(s[i]!='0')nonzero=1;}return nonzero;}
static int pc_usb_card_identity(int card,unsigned *identity){
 char file[112],id[48],path[256],bus[32],dev[32],fallback[48];
 snprintf(file,sizeof file,"/proc/asound/card%d/usbid",card);if(!pc_usb_read(file,id,sizeof id))return 0;
 snprintf(file,sizeof file,"/sys/class/sound/card%d/device",card);int n=readlink(file,path,sizeof path-1);if(n<1||n>=(int)sizeof path)return 1;path[n]=0;
 snprintf(file,sizeof file,"/sys/class/sound/card%d/device/../busnum",card);int have_bus=pc_usb_read(file,bus,sizeof bus);
 snprintf(file,sizeof file,"/sys/class/sound/card%d/device/../devnum",card);int have_dev=pc_usb_read(file,dev,sizeof dev);
 if(have_bus&&have_dev&&pc_usb_decimal(bus)&&pc_usb_decimal(dev)){
  *identity=pc_usb_hash(pc_usb_hash(pc_usb_hash(pc_usb_hash(2166136261u,id),path),bus),dev);return 2;
 }
 snprintf(file,sizeof file,"/proc/asound/card%d/usbbus",card);
 if(pc_usb_read(file,fallback,sizeof fallback)){
  unsigned i=0;while(fallback[i]>='0'&&fallback[i]<='9')i++;unsigned slash=i;
  if(slash&&fallback[i++]=='/'){unsigned start=i;while(fallback[i]>='0'&&fallback[i]<='9')i++;if(i>start&&!fallback[i]){*identity=pc_usb_hash(pc_usb_hash(pc_usb_hash(2166136261u,id),path),fallback);return 2;}}
 }
 return 1;
}
static void pc_usb_observe(void){
 unsigned identity=0;int known=0,present=0,card=-1;
 /* Observe the actual single-digit USB card selected by the stock router.
  * Its separate card1 prerequisite remains native; mismatches fail closed. */
 for(int c=1;c<=9;c++){unsigned candidate=0;int state=pc_usb_card_identity(c,&candidate);if(state){present=1;if(card<0)card=c;if(state==2){identity=candidate;card=c;known=1;break;}}}
 if(present&&known&&(!pc_usb_present||!pc_usb_epoch_valid||identity!=pc_usb_identity||card!=pc_usb_card)){
  pc_usb_generation++;if(!pc_usb_generation)pc_usb_generation=1;
  pc_usb_identity=identity;pc_usb_need_rebind=1;pc_usb_bound_generation=pc_usb_pcm_generation=0;
  pc_usb_stable=pc_usb_attempts=pc_usb_retry_at=0;pc_last_mixer=0;pc_last_volume=-1;
 }else if(!present&&pc_usb_present){
  pc_usb_need_rebind=1;pc_usb_bound_generation=pc_usb_pcm_generation=0;pc_usb_stable=0;
  pc_last_mixer=0;pc_last_volume=-1;
 }
 pc_usb_present=present;pc_usb_epoch_valid=known;pc_usb_card=card;if(present&&known&&pc_usb_stable<4)pc_usb_stable++;
}
static int pc_usb_owned(void){int mode=pc_workmode();return pc_enabled()&&pc_owned_request()&&(mode==1||(mode==2&&PC_BYTE(0x83a759)==6))&&PC_BYTE(0x83a75a)==3;}
static void pc_usb_minimum(void){
 /* Preserve native key slots and saved profiles; only the effective USB gain
  * is held at minimum until a current-generation PCM has been verified. */
 /* On native USB-AUDIO key selection this byte is itself the physical-key
  * volume slot. Keep that user value intact; the direct PCM gain hold below
  * still precedes close/open on this same serialized mode worker. */
 int percent_is_key=(pc_volume_active&&pc_volume_slot==0x83a789)||(pc_workmode()==1&&pc_native_volume_slot()==0x83a789);
 if(!percent_is_key)PC_BYTE(0x83a789)=0;
 /* Native keys and explicit output exit can change the real gain between
  * ticks. Never use a cached boolean as evidence that PCM is still muted. */
 PC_FN(0x44dc80,void (*)(float))(-116.5f);pc_usb_guarded=1;pc_last_volume=-1;
}
static void pc_usb_set_player_state(unsigned player,unsigned state){
 pthread_mutex_lock((void *)(unsigned long)(player+0xe8));PC_WORD(player+0x48)=state;pthread_mutex_unlock((void *)(unsigned long)(player+0xe8));
}
static int pc_usb_pcm_query_locked(unsigned sink){
 pc_usb_real_open=0;pc_usb_fd=pc_usb_pcm_state=-1;
 unsigned entry=sink+3*0x54,handle=PC_WORD(entry+0x74);
 if(pc_usb_card<1||PC_WORD(entry+0x60)!=(unsigned)pc_usb_card)return 0;
 if(!PC_WORD(entry+0x68)||!handle||!pc_usb_pcm_is_ready||!pc_usb_pcm_is_ready((void *)(unsigned long)handle))return 0;
 int fd=(int)PC_WORD(handle);if(fd<0)return 0;
 char path[64],actual[128],expected[64];snprintf(path,sizeof path,"/proc/self/fd/%d",fd);
 int n=readlink(path,actual,sizeof actual-1);if(n<1||n>=(int)sizeof actual)return 0;actual[n]=0;
 snprintf(expected,sizeof expected,"/dev/snd/pcmC%uD%up",PC_WORD(entry+0x60),PC_WORD(entry+0x64));
 if(strcmp(actual,expected))return 0; /* Includes old '(deleted)' device nodes. */
 /* Verified MIPS o32 snd_pcm_status is 108 bytes. STATUS is read-only and
  * distinguishes a disconnected kernel PCM from a nonzero heap pointer. */
 unsigned status[27]={0};if(ioctl(fd,0x406c4120u,status)<0)return 0;
 pc_usb_fd=fd;pc_usb_pcm_state=(int)status[0];if(status[0]>=8)return 0;
 pc_usb_real_open=1;return 1;
}
static int pc_usb_pcm_query(unsigned sink){
 pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));int ok=pc_usb_pcm_query_locked(sink);pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));return ok;
}
static void pc_usb_close_stale_locked(unsigned sink){
 /* Same fields, lock and tinyalsa close as original 0x478354, but target the
  * USB entry even after native hotplug has already changed selected route7.
  * Never rewrite route to a still-open analogue PCM to trick native dedup. */
 pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));unsigned entry=sink+3*0x54,handle=PC_WORD(entry+0x74);
 PC_WORD(entry+0x68)=0;PC_WORD(entry+0x74)=0;
 if(handle&&pc_usb_pcm_close)pc_usb_pcm_close((void *)(unsigned long)handle);
 pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));
 pc_usb_pcm_generation=0;pc_usb_real_open=0;pc_usb_fd=pc_usb_pcm_state=-1;
}
static void pc_usb_retire_output(void){
 unsigned player=PC_WORD(0x83dfc4),sink=PC_WORD(0x83dfe4);if(!player||!sink)return;
 /* Native idle route7->6 only changes selection. Retire the USB entry itself
  * before explicit exit, even while paused/idle or already unplugged. */
 if(PC_WORD(sink+3*0x54+0x74)){
  pc_usb_minimum();pthread_mutex_lock((void *)(unsigned long)(player+0xd0));unsigned state=PC_WORD(player+0x48);
  pc_usb_set_player_state(player,2);pc_usb_close_stale_locked(sink);pc_usb_set_player_state(player,state);pthread_mutex_unlock((void *)(unsigned long)(player+0xd0));
 }
 pc_usb_ready=pc_usb_recovering=pc_usb_error=0;pc_usb_real_open=0;pc_usb_fd=pc_usb_pcm_state=-1;
 pc_usb_bound_generation=pc_usb_pcm_generation=0;pc_usb_need_rebind=1;
}
static int pc_usb_rebind(unsigned player,unsigned sink){
 if(!pc_usb_pcm_close||!pc_usb_pcm_is_ready)return -11;
 pc_usb_minimum();pthread_mutex_lock((void *)(unsigned long)(player+0xd0));
 unsigned saved_state=PC_WORD(player+0x48),old_route=pc_route();
 /* 0x455c58 uses state2 as its serialized output hold, and identifies state0
  * and state3 as idle. Preserve the exact prior value, including pause. */
 pc_usb_set_player_state(player,2);
 if(old_route!=3&&old_route!=7&&old_route)PC_FN(0x478354,void (*)(unsigned))(0);
 pc_usb_close_stale_locked(sink);
 int result=0;
 if(!pc_usb_present||!pc_usb_epoch_valid)result=1;
 else{
  PC_FN(0x47bf24,unsigned (*)(unsigned,unsigned))(0,3);
  if(pc_route()!=3||PC_WORD(sink+3*0x54+0x60)!=(unsigned)pc_usb_card)result=-12;
  else if(saved_state==0||saved_state==3){pc_usb_bound_generation=pc_usb_generation;}
  else{
   unsigned bits=PC_WORD(player+0x24),format=PC_WORD(player+0x28),rate=PC_WORD(player+0x1c),channels=PC_WORD(player+0x20);
   int dsd=PC_WORD(player+0x58)!=0&&PC_WORD(player+0x98)==1;
   if(rate<8000||rate>1536000||channels<1||channels>8)result=-13;
   else if(PC_FN(0x47c2e8,int (*)(unsigned,unsigned *,unsigned *))(PC_WORD(player+0x5c)?PC_WORD(player+0x14):32,&bits,&format)<0)result=-14;
   else if(PC_FN(0x47859c,int (*)(unsigned,unsigned,unsigned,unsigned,int))(rate,bits,channels,0,dsd)<0)result=-15;
   else if(!pc_usb_pcm_query(sink))result=-16;
   else{
    PC_WORD(player+0x24)=bits;PC_WORD(player+0x28)=format;
    PC_WORD(player+4)=PC_FN(0x44c9cc,unsigned (*)(unsigned,unsigned,unsigned,unsigned))(rate,channels,bits,PC_WORD(player+0x58));
    pc_usb_bound_generation=pc_usb_pcm_generation=pc_usb_generation;pc_usb_rebuilds++;
   }
  }
 }
 if(result<0){pc_usb_close_stale_locked(sink);pc_usb_bound_generation=0;}
 pc_usb_set_player_state(player,saved_state);pthread_mutex_unlock((void *)(unsigned long)(player+0xd0));return result;
}
static void pc_usb_reset_unowned(void){
 pc_usb_ready=pc_usb_was_owned=pc_usb_recovering=pc_usb_error=0;pc_usb_guarded=0;pc_usb_real_open=0;pc_usb_fd=pc_usb_pcm_state=-1;
}
static void pc_usb_reconnect_tick(void){
 pc_usb_ready=0;int owned=pc_usb_owned();unsigned player=PC_WORD(0x83dfc4),sink=PC_WORD(0x83dfe4);
 if(!owned){pc_usb_reset_unowned();return;}
 pc_usb_observe();
 if(!pc_usb_was_owned){pc_usb_was_owned=1;pc_usb_need_rebind=1;pc_usb_attempts=pc_usb_retry_at=0;pc_usb_bound_generation=pc_usb_pcm_generation=0;}
 if(!player||!sink){pc_usb_error=-10;pc_usb_recovering=1;pc_usb_minimum();return;}
 if(!pc_usb_present){
  pc_usb_minimum();if(pc_pcm_open(3)||PC_WORD(sink+3*0x54+0x74)){
   pthread_mutex_lock((void *)(unsigned long)(player+0xd0));unsigned state=PC_WORD(player+0x48);pc_usb_set_player_state(player,2);pc_usb_close_stale_locked(sink);pc_usb_set_player_state(player,state);pthread_mutex_unlock((void *)(unsigned long)(player+0xd0));
  }
  pc_usb_error=0;pc_usb_recovering=1;return;
 }
 if(!pc_usb_epoch_valid){pc_usb_error=-20;pc_usb_recovering=1;pc_usb_minimum();return;}
 int healthy=pc_usb_pcm_query(sink);
 if(!pc_usb_need_rebind&&pc_usb_bound_generation==pc_usb_generation&&pc_route()==3){
  if(healthy){pc_usb_pcm_generation=pc_usb_generation;pc_usb_ready=1;pc_usb_recovering=pc_usb_error=0;return;}
  /* A native idle selection intentionally has no open PCM. Its next user
   * Play uses native startup; do not synthesize playback or reopen idle. */
  unsigned state=PC_WORD(player+0x48);
  if(!PC_WORD(sink+3*0x54+0x74)&&(state==0||state==3)){pc_usb_pcm_generation=0;pc_usb_recovering=pc_usb_error=0;pc_usb_minimum();return;}
  pc_usb_need_rebind=1;pc_usb_pcm_generation=0;
 }
 pc_usb_minimum();pc_usb_recovering=1;unsigned t=pc_seconds();
 if(pc_usb_stable<2)return;
 if(pc_usb_attempts>=3){if(!pc_usb_error)pc_usb_error=-21;pc_usb_recovering=0;return;}
 if(pc_usb_retry_at&&t-pc_usb_retry_at<2)return;
 pc_usb_retry_at=t;pc_usb_attempts++;int result=pc_usb_rebind(player,sink);pc_result=result;
 if(!result){pc_usb_need_rebind=0;pc_usb_error=pc_usb_recovering=0;pc_usb_attempts=pc_usb_retry_at=0;pc_usb_ready=pc_usb_pcm_generation==pc_usb_generation&&pc_usb_real_open;pc_switches++;}
 else if(result<0)pc_usb_error=result;
}
static void pc_usb_apply_current_volume(void){
 unsigned player=PC_WORD(0x83dfc4),sink=PC_WORD(0x83dfe4);if(!player||!sink){pc_usb_ready=0;pc_usb_recovering=1;pc_usb_error=-10;pc_usb_minimum();return;}
 pthread_mutex_lock((void *)(unsigned long)(player+0xd0));
 pthread_mutex_lock((void *)(unsigned long)(sink+0x1c));
 /* Native hotplug does not take our command mutex. Recheck the physical
  * identity and actual kernel PCM in the same original PCM critical section
  * that applies gain, rather than trusting the earlier ready snapshot. */
 /* Ownership can disappear between the worker hint and these native locks.
  * Do not probe an unused USB port after the handoff has been revoked. */
 if(!pc_usb_owned()){
  pc_usb_ready=0;pc_usb_recovering=1;pc_usb_minimum();
  pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));
  pthread_mutex_unlock((void *)(unsigned long)(player+0xd0));return;
 }
 pc_usb_observe();
 int current=pc_usb_owned()&&pc_route()==3&&pc_usb_present&&pc_usb_epoch_valid&&!pc_usb_need_rebind&&
  pc_usb_bound_generation==pc_usb_generation&&pc_usb_pcm_generation==pc_usb_generation&&pc_usb_pcm_query_locked(sink);
 if(current){pc_usb_guarded=0;pc_apply_usb_volume();}
 else{pc_usb_ready=0;pc_usb_recovering=1;pc_usb_minimum();}
 pthread_mutex_unlock((void *)(unsigned long)(sink+0x1c));
 pthread_mutex_unlock((void *)(unsigned long)(player+0xd0));
}
static void pc_usb_runtime_tick(int owned){
 /* The caller already checked ownership to choose the volume profile. Reuse
  * that negative result instead of adding more access() polling while idle. */
 if(!owned){pc_usb_reset_unowned();return;}
 /* A prior ready result selects a cheap path; it never authorizes gain.
  * The same original control->PCM locks still cover fresh identity, fd and
  * kernel STATUS checks immediately before every effective gain update. */
 if(pc_usb_ready&&!pc_usb_need_rebind&&!pc_usb_error){
  pc_usb_apply_current_volume();if(pc_usb_ready)return;
 }
 /* Initial attach, paused/idle, failed validation and explicit retry retain
  * the complete original recovery path, including same-tick stale close. */
 pc_usb_reconnect_tick();
 if(pc_usb_owned()){
  if(pc_usb_ready&&!pc_usb_need_rebind&&!pc_usb_error)pc_usb_apply_current_volume();
  else pc_usb_minimum();
 }
}
