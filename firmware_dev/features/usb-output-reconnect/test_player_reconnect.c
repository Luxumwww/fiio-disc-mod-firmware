/* Actual production reconnect header, isolated filesystem/PCM/native model. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static unsigned char memory[0x100000];
#define PC_WORD(a) (*(unsigned *)(memory+(unsigned)(a)-0x800000))
#define PC_BYTE(a) (memory[(unsigned)(a)-0x800000])
static void *native(unsigned);
#define PC_FN(a,t) ((t)native(a))
static unsigned pc_last_mixer,pc_switches;static int pc_last_volume,pc_result,pc_volume_active;
static unsigned pc_volume_slot;
static int physical,epoch_ok,devnum=1,pcm_epoch,fail_open,bad_fd,disconnected,owner=1,mode=2,external_card=1,pcm_card=1,zombie_card1,native_card1_exists=1;
static unsigned seconds=10,closes,opens,routes,applies,control_depth,pcm_depth,state_depth;
static unsigned native_gain_calls;static float gain_db;
static struct {unsigned opens,reads,closes,links,status,locked;} test_io;
static int revoke_on_pcm_lock;
static int pc_enabled(void){return 1;}
static int pc_owned_request(void){return owner;}
static int pc_workmode(void){return mode;}
static unsigned pc_seconds(void){return seconds;}
static unsigned pc_native_volume_slot(void){return pc_volume_slot;}
static unsigned pc_route(void){return PC_WORD(0x820058);}
static int pc_pcm_open(unsigned r){return PC_WORD(0x820000+r*0x54+0x68)&&PC_WORD(0x820000+r*0x54+0x74);}
static void pc_apply_usb_volume(void){assert(control_depth==1&&pcm_depth==1&&pc_route()==3);applies++;gain_db=-30;}
static int lock(void *p){unsigned a=(unsigned long)p;if(a==0x8100d0){assert(!pcm_depth&&!state_depth);control_depth++;assert(control_depth==1);}else if(a==0x82001c){pcm_depth++;assert(pcm_depth==1);if(revoke_on_pcm_lock){owner=0;revoke_on_pcm_lock=0;}}else if(a==0x8100e8){assert(control_depth==1);state_depth++;assert(state_depth==1);}return 0;}
static int unlock(void *p){unsigned a=(unsigned long)p;if(a==0x8100d0){assert(!pcm_depth&&!state_depth);control_depth--;}else if(a==0x82001c)pcm_depth--;else if(a==0x8100e8)state_depth--;return 0;}
#define pthread_mutex_lock lock
#define pthread_mutex_unlock unlock
static char opened[16][128];static int next_fd;
static int file_open(const char *p,int mode,...){test_io.opens++;if(pcm_depth)test_io.locked++;(void)mode;if(!physical)return -1;const char *at=strstr(p,"card");int card=at?at[4]-'0':-1;if(card!=external_card&&!(zombie_card1&&card==1&&strstr(p,"usbid")))return -1;if(strstr(p,"usbid")||strstr(p,"busnum")||strstr(p,"devnum")){if(!epoch_ok&&!strstr(p,"usbid"))return -1;int fd=next_fd++%16;snprintf(opened[fd],128,"%s",p);return fd;}return -1;}
static int file_close(int fd){test_io.closes++;if(pcm_depth)test_io.locked++;(void)fd;return 0;}
static int file_read(int fd,void *data,unsigned n){test_io.reads++;if(pcm_depth)test_io.locked++;const char *p=opened[fd];char value[32];if(strstr(p,"usbid"))snprintf(value,sizeof value,"0661:0881\n");else if(strstr(p,"busnum"))snprintf(value,sizeof value,"1\n");else snprintf(value,sizeof value,"%d\n",devnum);unsigned size=(unsigned)strlen(value);if(size>n)size=n;memcpy(data,value,size);return (int)size;}
static long file_readlink(const char *p,char *out,unsigned n){
 test_io.links++;if(pcm_depth)test_io.locked++;
 const char *v=0;char fdpath[80];if(strstr(p,"/sys/class/sound/card")){const char *at=strstr(p,"card");if(!physical||!at||at[4]-'0'!=external_card)return -1;v="../../../devices/usb1/1-1/1-1:1.0";}
 else if(strstr(p,"/proc/self/fd/50")){snprintf(fdpath,sizeof fdpath,"/dev/snd/pcmC%dD0p%s",pcm_card,bad_fd?" (deleted)":"");v=fdpath;}
 if(!v)return -1;unsigned size=(unsigned)strlen(v);if(size>n)size=n;memcpy(out,v,size);return (long)size;
}
static int kernel_ioctl(int fd,unsigned request,void *data){test_io.status++;if(pcm_depth)test_io.locked++;assert(pcm_depth==1);assert(fd==50&&request==0x406c4120u);if(!physical||pcm_epoch!=devnum)return -1;((unsigned *)data)[0]=disconnected?8:2;return 0;}
#define open file_open
#define close file_close
#define read file_read
#define readlink file_readlink
#define ioctl kernel_ioctl
#include "../bluetooth-meter/usb_reconnect_player.h"
static void native_gain(float value){assert(value==-116.5f);gain_db=value;native_gain_calls++;}
static int close_pcm(void *p){assert(control_depth==1&&pcm_depth==1);assert(gain_db==-116.5f);assert((unsigned long)p==0x860000);closes++;return 0;}
static int pcm_is_ready(void *p){return (unsigned long)p==0x860000&&PC_WORD(0x860000)==50;}
static void close_current(unsigned flags){assert(!flags&&control_depth==1);unsigned e=0x820000+pc_route()*0x54;PC_WORD(e+0x68)=PC_WORD(e+0x74)=0;}
static unsigned route(unsigned outmode,unsigned desired){assert(control_depth==1&&!pcm_depth&&!outmode&&desired==3);assert(gain_db==-116.5f);routes++;if(physical&&epoch_ok&&native_card1_exists){PC_WORD(0x820058)=3;PC_WORD(0x820000+3*0x54+0x60)=(unsigned)external_card;PC_WORD(0x820000+3*0x54+0x64)=0;}else PC_WORD(0x820058)=7;return pc_route();}
static int choose_format(unsigned input,unsigned *bits,unsigned *format){(void)input;assert(control_depth==1);*bits=32;*format=7;return 0;}
static int open_pcm(unsigned rate,unsigned bits,unsigned channels,unsigned flags,int dsd){assert(control_depth==1&&!pcm_depth);assert(gain_db==-116.5f&&rate==96000&&bits==32&&channels==2&&!flags&&!dsd);opens++;if(fail_open)return -1;PC_WORD(0x820000+3*0x54+0x68)=1;PC_WORD(0x820000+3*0x54+0x74)=0x860000;PC_WORD(0x860000)=50;pcm_epoch=devnum;pcm_card=external_card;return 0;}
static unsigned chunk(unsigned r,unsigned c,unsigned b,unsigned d){(void)r;(void)c;(void)b;(void)d;return 8192;}
static void *native(unsigned a){switch(a){case 0x44dc80:return native_gain;case 0x478354:return close_current;case 0x47bf24:return route;case 0x47c2e8:return choose_format;case 0x47859c:return open_pcm;case 0x44c9cc:return chunk;default:fprintf(stderr,"unexpected native %x\n",a);abort();}}
static void reset(unsigned state){
 memset(&test_io,0,sizeof test_io);revoke_on_pcm_lock=0;
 memset(memory,0,sizeof memory);PC_WORD(0x83dfc4)=0x810000;PC_WORD(0x83dfe4)=0x820000;PC_WORD(0x810048)=state;
 PC_WORD(0x81001c)=96000;PC_WORD(0x810020)=2;PC_WORD(0x810024)=32;PC_WORD(0x810028)=7;PC_WORD(0x810098)=3;PC_WORD(0x820058)=6;
 PC_BYTE(0x83a759)=6;PC_BYTE(0x83a75a)=3;PC_BYTE(0x83a74c)=40;PC_BYTE(0x83a7a0)=25;PC_BYTE(0x83a789)=73;
 pc_usb_generation=pc_usb_pcm_generation=pc_usb_bound_generation=pc_usb_identity=pc_usb_stable=pc_usb_attempts=pc_usb_retry_at=0;
 pc_usb_present=pc_usb_epoch_valid=pc_usb_ready=pc_usb_recovering=pc_usb_error=pc_usb_need_rebind=pc_usb_was_owned=pc_usb_guarded=pc_usb_real_open=0;
 pc_usb_pcm_state=pc_usb_fd=-1;pc_usb_rebuilds=0;pc_last_mixer=19;pc_last_volume=20;pc_volume_active=0;pc_volume_slot=0x83a7a0;
 physical=epoch_ok=owner=1;mode=2;devnum=1;external_card=pcm_card=native_card1_exists=1;zombie_card1=0;pcm_epoch=0;fail_open=bad_fd=disconnected=0;seconds=10;closes=opens=routes=applies=control_depth=pcm_depth=state_depth=native_gain_calls=0;gain_db=-30;pc_usb_pcm_close=close_pcm;pc_usb_pcm_is_ready=pcm_is_ready;
}
static void stale_handle(void){PC_WORD(0x820000+3*0x54+0x68)=1;PC_WORD(0x820000+3*0x54+0x74)=0x860000;PC_WORD(0x860000)=50;}
int main(void){
 reset(4);pc_usb_reconnect_tick();assert(!opens&&!routes&&pc_usb_recovering);pc_usb_reconnect_tick();assert(opens==1&&pc_usb_ready&&pc_usb_generation==1&&pc_usb_pcm_generation==1&&PC_WORD(0x810048)==4);assert(PC_BYTE(0x83a759)==6&&PC_BYTE(0x83a74c)==40&&PC_BYTE(0x83a7a0)==25);
 pc_usb_apply_current_volume();assert(applies==1&&gain_db==-30);PC_WORD(0x820058)=7;pc_usb_apply_current_volume();assert(applies==1&&!pc_usb_ready&&gain_db==-116.5f);
 physical=0;pc_usb_reconnect_tick();assert(closes==1&&!pc_pcm_open(3)&&!pc_usb_ready);physical=1;devnum=2;PC_WORD(0x820058)=3;pc_usb_reconnect_tick();pc_usb_reconnect_tick();assert(opens==2&&pc_usb_ready&&pc_usb_generation==2&&pc_usb_pcm_generation==2);
 devnum=3;pc_usb_apply_current_volume();assert(applies==1&&!pc_usb_ready&&gain_db==-116.5f);pc_usb_reconnect_tick();pc_usb_reconnect_tick();assert(opens==3&&pc_usb_generation==3&&pc_usb_ready);bad_fd=1;pc_usb_apply_current_volume();assert(applies==1&&!pc_usb_ready);bad_fd=0;
 for(unsigned state=0;state<=3;state+=3){reset(state);stale_handle();PC_WORD(0x820058)=3;pc_usb_reconnect_tick();pc_usb_reconnect_tick();assert(closes==1&&!opens&&pc_route()==3&&PC_WORD(0x810048)==state&&!pc_usb_ready&&!pc_usb_recovering&&!pc_usb_error);assert(pc_usb_bound_generation==pc_usb_generation);}
 reset(2);stale_handle();pc_usb_reconnect_tick();pc_usb_reconnect_tick();assert(PC_WORD(0x810048)==2&&opens==1&&pc_usb_ready);
 for(int i=0;i<6;i++){bad_fd=1;pc_usb_reconnect_tick();bad_fd=0;seconds+=2;pc_usb_reconnect_tick();assert(pc_usb_ready&&!pc_usb_attempts&&!pc_usb_retry_at);}assert(opens>=7);
 reset(4);fail_open=1;pc_usb_reconnect_tick();pc_usb_reconnect_tick();assert(!pc_usb_ready&&pc_usb_error<0&&pc_route()==3&&gain_db==-116.5f&&PC_WORD(0x810048)==4);seconds+=2;pc_usb_reconnect_tick();seconds+=2;pc_usb_reconnect_tick();unsigned attempts=opens;seconds+=10;pc_usb_reconnect_tick();assert(opens==attempts&&opens==3&&!pc_usb_recovering&&pc_usb_error<0);
 reset(4);epoch_ok=0;pc_usb_reconnect_tick();pc_usb_reconnect_tick();assert(pc_usb_present&&!pc_usb_epoch_valid&&!opens&&pc_usb_error==-20&&!pc_usb_ready);
 reset(4);pc_usb_reconnect_tick();pc_usb_reconnect_tick();external_card=2;zombie_card1=1;devnum++;pc_usb_reconnect_tick();pc_usb_reconnect_tick();assert(pc_usb_card==2&&pc_usb_generation==2&&pc_usb_ready&&PC_WORD(0x820000+3*0x54+0x60)==2&&pcm_card==2);
 reset(4);external_card=2;native_card1_exists=0;pc_usb_reconnect_tick();pc_usb_reconnect_tick();assert(pc_usb_card==2&&!pc_usb_ready&&pc_usb_error==-12&&!opens&&gain_db==-116.5f);
 reset(4);pc_usb_reconnect_tick();pc_usb_reconnect_tick();PC_WORD(0x820000+3*0x54+0x60)=2;pc_usb_apply_current_volume();assert(!pc_usb_ready&&!applies&&gain_db==-116.5f);
 reset(4);mode=1;pc_volume_active=1;pc_volume_slot=0x83a789;PC_BYTE(0x83a789)=61;pc_usb_minimum();pc_usb_minimum();assert(PC_BYTE(0x83a789)==61&&gain_db==-116.5f);pc_volume_active=0;PC_BYTE(0x83a789)=47;pc_usb_minimum();assert(PC_BYTE(0x83a789)==47);
 gain_db=0;pc_usb_minimum();assert(gain_db==-116.5f&&PC_BYTE(0x83a789)==47);gain_db=-7;pc_usb_minimum();assert(gain_db==-116.5f&&PC_BYTE(0x83a789)==47);
 for(unsigned state=0;state<=3;state+=3){reset(state);stale_handle();PC_WORD(0x820058)=7;pc_usb_retire_output();assert(closes==1&&!pc_pcm_open(3)&&!PC_WORD(0x820000+3*0x54+0x74)&&PC_WORD(0x810048)==state&&pc_route()==7&&gain_db==-116.5f);gain_db=0;pc_usb_minimum();assert(gain_db==-116.5f);}
 reset(4);pc_usb_reconnect_tick();pc_usb_reconnect_tick();owner=0;pc_usb_apply_current_volume();assert(!applies&&gain_db==-116.5f);
 /* The production worker uses one scheduling entry, keeping all original
  * direct recovery tests above while checking the stable-path optimization. */
 for(unsigned m=1;m<=2;m++)for(int attached=0;attached<=1;attached++){
  reset(4);mode=(int)m;owner=0;physical=attached;
  for(unsigned i=0;i<100;i++)pc_usb_runtime_tick(pc_usb_owned());
  assert(!test_io.opens&&!test_io.reads&&!test_io.closes&&!test_io.links&&!test_io.status&&!opens&&!closes&&!applies);
 }
 printf("RUNTIME_IO_UNUSED_100_TICKS opens=%u reads=%u closes=%u links=%u status=%u locked=%u\n",test_io.opens,test_io.reads,test_io.closes,test_io.links,test_io.status,test_io.locked);
 reset(4);pc_usb_runtime_tick(pc_usb_owned());pc_usb_runtime_tick(pc_usb_owned());assert(pc_usb_ready&&opens==1&&applies==1&&gain_db==-30);
 memset(&test_io,0,sizeof test_io);unsigned stable_opens=opens;
 for(unsigned i=0;i<100;i++)pc_usb_runtime_tick(pc_usb_owned());
 assert(test_io.opens==300&&test_io.reads==300&&test_io.closes==300&&test_io.links==200&&test_io.status==100&&test_io.locked==1200&&opens==stable_opens&&applies==101);
 printf("RUNTIME_IO_STABLE_100_TICKS opens=%u reads=%u closes=%u links=%u status=%u locked=%u\n",test_io.opens,test_io.reads,test_io.closes,test_io.links,test_io.status,test_io.locked);
 /* Ready is only a scheduling hint. Identity changes and stale fds still
  * hold minimum before close/open and recover in this same worker tick. */
 devnum++;pc_usb_runtime_tick(pc_usb_owned());assert(pc_usb_ready&&pc_usb_generation==2&&pc_usb_pcm_generation==2&&opens==2&&closes==1&&gain_db==-30);
 PC_WORD(0x820058)=7;pc_usb_runtime_tick(pc_usb_owned());assert(pc_usb_ready&&pc_route()==3&&opens==3&&closes==2);
 physical=0;pc_usb_runtime_tick(pc_usb_owned());assert(!pc_usb_ready&&!pc_pcm_open(3)&&closes==3&&gain_db==-116.5f);
 physical=1;devnum++;pc_usb_runtime_tick(pc_usb_owned());pc_usb_runtime_tick(pc_usb_owned());assert(pc_usb_ready&&opens==4&&pc_usb_pcm_generation==pc_usb_generation);
 disconnected=1;pc_usb_runtime_tick(pc_usb_owned());assert(!pc_usb_ready&&pc_usb_error<0&&!pc_pcm_open(3)&&gain_db==-116.5f);disconnected=0;seconds+=2;pc_usb_runtime_tick(pc_usb_owned());assert(pc_usb_ready&&gain_db==-30);
 PC_WORD(0x83dfe4)=0;pc_usb_runtime_tick(pc_usb_owned());assert(!pc_usb_ready&&pc_usb_recovering&&pc_usb_error==-10&&gain_db==-116.5f);PC_WORD(0x83dfe4)=0x820000;pc_usb_runtime_tick(pc_usb_owned());assert(pc_usb_ready&&!pc_usb_error&&gain_db==-30);
 memset(&test_io,0,sizeof test_io);unsigned before_applies=applies;revoke_on_pcm_lock=1;pc_usb_runtime_tick(pc_usb_owned());assert(!owner&&!pc_usb_ready&&applies==before_applies&&!test_io.opens&&!test_io.links&&!test_io.status&&gain_db==-116.5f);
 for(unsigned state=0;state<=3;state+=3){reset(state);stale_handle();PC_WORD(0x820058)=3;pc_usb_runtime_tick(pc_usb_owned());pc_usb_runtime_tick(pc_usb_owned());assert(closes==1&&!opens&&!applies&&PC_WORD(0x810048)==state&&!pc_usb_ready&&!pc_usb_recovering&&!pc_usb_error);PC_WORD(0x810048)=4;pc_usb_runtime_tick(pc_usb_owned());assert(pc_usb_ready&&opens==1&&applies==1&&PC_WORD(0x810048)==4);}
 reset(4);fail_open=1;for(unsigned i=0;i<20;i++){pc_usb_runtime_tick(pc_usb_owned());seconds++;}assert(opens==3&&!pc_usb_ready&&!pc_usb_recovering&&pc_usb_error<0&&gain_db==-116.5f);
 puts("PASS: per-attach identity/short unplug ABA, two-sample settle, stale PCM close/open, fd/STATUS checks, pause/idle state retained, bounded failure without analogue fallback, apply-time route/epoch/fd/ownership races, native 83a789 key slot preserved, idle route7 explicit exit retires USB fd, fast off/on and native-key gain change cannot bypass minimum");
 puts("PASS: unused USB performs no probes; stable worker validates identity/fd/STATUS once under original locks; cached ready cannot bypass validation; ABA, route change, disconnect, STATUS failure, lock-time ownership revoke, paused user resume and failure budget remain covered");
 return 0;
}
