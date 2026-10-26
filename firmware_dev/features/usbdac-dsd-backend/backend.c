/* Independent USB DSD user-space backend, pinned DISC V2.57/kernel236.
 * Native code supplies only raw USB capture, generic rings and lifecycle.
 * No native DoP detector, DSD_init, DSD output callback or PCM handle reuse. */
#ifndef DISC_BACKEND_TEST
typedef unsigned size_t;
#else
#include <stddef.h>
#endif
extern int open(const char *,int,...),close(int),read(int,void *,size_t),write(int,const void *,size_t),access(const char *,int),getpid(void),usleep(unsigned),prctl(int,...),clock_gettime(int,void *),snprintf(char *,size_t,const char *,...);
extern int memcmp(const void *,const void *,size_t),strcmp(const char *,const char *);
extern int fcntl(int,int,...),*__errno_location(void);
extern void *memcpy(void *,const void *,size_t),*malloc(size_t);extern void free(void *);
extern void *dlsym(void *,const char *),*mmap(void *,size_t,int,int,int,long);
extern int munmap(void *,size_t),mprotect(void *,size_t,int),cacheflush(void *,size_t,int),pthread_mutex_lock(void *),pthread_mutex_unlock(void *);
#include "stream.h"
#include "dac.h"
#include "guard.h"
#ifndef DISC_BACKEND_TEST
#define W(a) (*(volatile unsigned *)(unsigned long)(a))
#define B(a) (*(volatile unsigned char *)(unsigned long)(a))
#define F(a,t) ((t)(unsigned long)(a))
#endif
static int mutex[16],enabled,installed,bus=-1;
static volatile unsigned owned,worker_live,ready,failures,generation;
static unsigned packets,frames,quiet_skips,health_skips,rate,xruns,error_code,report_at,stage;
static char pcm_error[192];
static unsigned clock_padding,clock_padding_error;
static unsigned char clock_buffer[16384];
static unsigned char levels[2]={255,255};
static void *pcm;static DsbDac dac;static DsbStream stream;
static DsbDac failed_dac;
static char bootstrap_status[512];
static void capture_bootstrap_status(void){
#ifndef DISC_BACKEND_TEST
 int f=open("/proc/asound/card0/pcm3p/sub0/status",0),n=f<0?0:read(f,bootstrap_status,sizeof(bootstrap_status)-1);
 if(f>=0)close(f);bootstrap_status[n>0?n:0]=0;
#endif
}
static int (*real_ioctl)(int,unsigned,void *),(*real_create)(void *,const void *,void *(*)(void *),void *);
static void *(*real_open)(unsigned,unsigned,unsigned,const unsigned *);
static int (*real_close)(void *),(*real_ready)(void *),(*real_prepare)(void *),(*real_stop)(void *);
static const char *(*real_error)(void *);
static unsigned seconds(void){long t[2];return clock_gettime(1,t)?0:(unsigned)t[0];}
static int scope(void){unsigned p=W(0x83dfc0),s=W(0x83dfe4);return p&&W(p)==4&&s&&W(s+0x58)==6&&B(0x83a75a)==6;}
static int eligible(unsigned p){return scope()&&p&&(W(p+0xc)==176400||W(p+0xc)==352800)&&W(p+0x10)==2&&W(p+0x14)==32&&W(p+0x18)==0;}
struct Message {unsigned short address,flags,length;unsigned char *data;};
struct Transfer {struct Message *messages;unsigned count;};
static int reg_read(void *ctx,unsigned a,unsigned r,unsigned char *v){
 (void)ctx;unsigned char x[4]={r>>16,r>>8,r,0};struct Message m[2]={{a,0,4,x},{a,1,1,v}};struct Transfer t={m,2};return real_ioctl(bus,0x707,&t)==2;
}
static int reg_write(void *ctx,unsigned a,unsigned r,unsigned char v){
 (void)ctx;unsigned char x[5]={r>>16,r>>8,r,0,v};struct Message m={a,0,5,x};struct Transfer t={&m,1};return real_ioctl(bus,0x707,&t)==1;
}
static int write_output(const unsigned char *,unsigned);
static void wait_us(unsigned n){
 /* Keep the serial clock alive through the required150ms external-supply
  * mute ramp. PCM bootstrap uses digital zero; DSD shutdown uses valid
  * DoP silence. Never inject PCM zero into an audible DSD stream. */
 while(n){unsigned delay=n>5000?5000:n;usleep(delay);n-=delay;
  if(clock_padding&&pcm&&!clock_padding_error){
   unsigned bytes=(rate*delay/1000000u)*8;
   if(clock_padding==2)dsb_frame(&stream,0,bytes,clock_buffer);
   else for(unsigned i=0;i<bytes;i++)clock_buffer[i]=0;
   if(bytes&&!write_output(clock_buffer,bytes))clock_padding_error=1;
  }
 }
}
static int bus_open(void){
 const char *path[2]={"/sys/bus/i2c/devices/3-0030/name","/sys/bus/i2c/devices/3-0032/name"};
 const char *expect[2]={"cs43131\n","cs43131c\n"};
 for(unsigned i=0;i<2;i++){char b[24];int f=open(path[i],0),n=f<0?0:read(f,b,23);if(f>=0)close(f);if(n<=0)return 0;b[n]=0;if(strcmp(b,expect[i]))return 0;}
 bus=open("/dev/i2c-3",2);return bus>=0;
}
/* Never acquire the native output mutex while holding this backend mutex.
 * Native pcm_open may call yield while it holds its output mutex. */
static int release_locked(void){
 ready=0;int ok=1;
 clock_padding=pcm?(dac.configured?2:1):0;clock_padding_error=0;
 if(bus>=0&&dac.saved_ok){ok=dsb_down(&dac);if(!ok){error_code=dac.error;failures++;}}
 clock_padding=0;
 if(pcm){real_stop(pcm);real_close(pcm);pcm=0;}
 if(ok&&bus>=0)ok=dsb_restore(&dac);
 if(bus>=0){close(bus);bus=-1;}
 if(!ok){error_code=dac.error;failures++;}
 dac.saved_ok=dac.configured=dac.up=0;owned=0;B(0x899060)=0;generation++;return ok;
}
static int yield(void){pthread_mutex_lock(mutex);int ok=release_locked();pthread_mutex_unlock(mutex);return ok;}
static int open_output(unsigned hz){
 unsigned count=8,period=hz/100;unsigned cfg[9]={2,hz,period,count,7,period*count,period*count,0,period};
 pcm=real_open(0,3,0,cfg);int ok=pcm&&real_ready(pcm);
 if(!ok){if(pcm&&real_error)snprintf(pcm_error,sizeof(pcm_error),"%s",real_error(pcm));return 0;}
 int fd=(int)W((unsigned long)pcm),flags=fcntl(fd,3);return flags>=0&&fcntl(fd,4,flags|0x80)==0;
}
/* Direct ALSA WRITEI, nonblocking MIPS fd. No tinyalsa automatic prepare or
 * silent retry: XRUN and partial writes remain visible to this state machine.
 * A stopped clock cannot hold the format-transition mutex indefinitely. */
static int write_output(const unsigned char *data,unsigned bytes){
 unsigned offset=0,waits=0;int fd=(int)W((unsigned long)pcm);
 while(offset<bytes){
  struct {int result;const void *buffer;unsigned frames;} x={0,data+offset,(bytes-offset)/8};
  int rc=real_ioctl(fd,0x800c4150,&x); /* o32 MIPS _IOW, verified stock tinyalsa */
  if(rc>=0&&x.result>0&&(unsigned)x.result<=x.frames){offset+=(unsigned)x.result*8;waits=0;continue;}
  int err=rc<0?*__errno_location():(x.result<0?-x.result:0);
  if(err!=11&&err!=4&&!(rc>=0&&x.result==0)){error_code=(unsigned)err;return 0;}
  if(W(0x839a70)||W(0x839a74)==3||++waits>=200){error_code=110;return 0;}
  usleep(1000);
 }return 1;
}
static int start_output(unsigned hz,unsigned char *out){
 /* Called by raw worker, outside all backend locks for native close. */
 pthread_mutex_lock(mutex);if(owned&&ready&&rate==hz){pthread_mutex_unlock(mutex);return 1;}
 if(owned)release_locked();owned=1;ready=0;rate=hz;unsigned lease=++generation;pthread_mutex_unlock(mutex);
 F(0x478354,void (*)(unsigned))(0);
 pthread_mutex_lock(mutex);
 if(!owned||generation!=lease||!scope()){pthread_mutex_unlock(mutex);return 0;}
 dac.ctx=0;dac.read=reg_read;dac.write=reg_write;dac.wait=wait_us;
 stage=1;int ok=bus_open();if(ok){stage=2;ok=open_output(hz);}
 /* Closing the stock PCM stops LRCK. HP power-down completion requires the
  * current PCM subclocks: provide only digital zero to restart those clocks
  * before requesting PDN_DONE. No incoming DoP reaches a PCM-configured DAC.
  * Prepare again below discards the bootstrap before DSD prefill. */
 if(ok){stage=21;ok=dsb_snapshot(&dac);}
 if(ok){stage=20;ok=real_prepare(pcm)==0;}
 for(unsigned remaining=(hz/100)*8;ok&&remaining;){
  unsigned n=remaining>4096?4096:remaining;
  for(unsigned i=0;i<n*8;i++)out[i]=0;
  ok=write_output(out,n*8);remaining-=n;
 }
 if(ok){capture_bootstrap_status();stage=3;clock_padding=1;clock_padding_error=0;ok=dsb_setup(&dac,hz)&&!clock_padding_error;clock_padding=0;}
 /* Inherit current analogue attenuation, never multiply DoP samples. */
 for(unsigned c=0;c<2&&ok;c++){
  unsigned char volume;if(levels[c]!=255)volume=levels[c];else ok=reg_read(0,0x30+2*c,0x90001,&volume);
  if(ok){levels[c]=volume;ok=dsb_volume(&dac,c,volume);}
 }
 if(ok){stage=4;ok=real_prepare(pcm)==0;}if(ok){stage=5;ok=dsb_serial_up(&dac);}
 stream.out_marker=5;
 for(unsigned remaining=(hz/100)*8;ok&&remaining;){
  stage=6;unsigned n=remaining>4096?4096:remaining;dsb_frame(&stream,0,n*8,out);
  ok=write_output(out,n*8);remaining-=n;
 }
 if(ok){stage=7;ok=dsb_hp_up(&dac);}
 ready=ok;if(!ok){failed_dac=dac;if(dac.error)error_code=dac.error;else if(!error_code)error_code=1;failures++;
  int f=open("/tmp/disc-runtime/dsd-bootstrap-status",1|0x100|0x200,0600);
  if(f>=0){unsigned n=0;while(n<sizeof(bootstrap_status)&&bootstrap_status[n])n++;write(f,bootstrap_status,n);close(f);}
  release_locked();}
 else{stage=8;B(0x899060)=1;W(0x899084)=hz;}
 pthread_mutex_unlock(mutex);return ok;
}
static int emit(const unsigned char *in,unsigned n,unsigned char *out){
 pthread_mutex_lock(mutex);int ok=owned&&ready&&pcm;
 if(ok){dsb_frame(&stream,in,n,out);ok=write_output(out,n);
  if(ok){packets++;frames+=n/8;}else{xruns++;failures++;release_locked();}
 }
 pthread_mutex_unlock(mutex);return ok;
}
static int native_pcm(unsigned p){
 if(owned&&!yield())return 0;
 W(p+0x2c)=1;W(p+0x58)=0;
 unsigned s=W(0x83dfe4);if(!s)return 0;
 if(!W(s+6*0x54+0x68)||W(p+0x1c)!=W(p+0xc)){
  if(W(s+6*0x54+0x68))F(0x478354,void (*)(unsigned))(0);
  unsigned hz=W(p+0xc);W(p+0x1c)=hz;W(p+0x20)=2;W(p+0x24)=32;W(p+0x28)=7;
  F(0x4e9534,int (*)(unsigned,unsigned,unsigned))(hz,32,0);
  /* Generic PCM resource reopening, only after independent DSD is released. */
  if(F(0x47859c,int (*)(unsigned,unsigned,unsigned,unsigned,unsigned))(hz,32,2,0,0))return 0;
  F(0x4e8b94,void (*)(unsigned))(0);
 }return 1;
}
static void *worker(void *arg){
 unsigned p=(unsigned long)arg;unsigned char *raw=malloc(65536),*out=malloc(65536);unsigned candidate=0,invalid=0,failed=0;
 worker_live=1;W(0x839a70)=0;W(0x839a74)=1;W(0x839a7c)=W(0x839a78)=0;dsb_reset(&stream);prctl(15,"disc_dsd_raw",0,0,0);
 if(!raw||!out)goto done;
 while(!W(0x839a70)&&W(0x839a74)!=3){
  if(W(0x839a74)==2){if(owned)yield();failed=0;usleep(5000);continue;}
  if(!eligible(p)||W(p+0x50))break;
  unsigned n=(W(p+0xc)/100)*8;if(n>32768)n=32768;
  if(F(0x4cb6b0,int (*)(unsigned))(p+0x130)<(int)n){usleep(1000);continue;}
  unsigned got=F(0x4cb534,unsigned (*)(unsigned,void *,unsigned))(p+0x130,raw,n);if(!got)continue;
  int kind=dsb_scan(&stream,raw,got);
  if(kind>0){
   if(kind==1){if(candidate+got<=32768){memcpy(out+candidate,raw,got);candidate+=got;}continue;}
   W(p+0x2c)=2;W(p+0x58)=1;invalid=0;
   /* Typical10ms blocks exceed32 markers; tiny candidates are held rather
    * than ever reaching a PCM DAC. Preserve their payload before prefill. */
   if(candidate){memcpy(raw+32768,out,candidate);}
   if(!failed&&start_output(W(p+0xc),out)){
    if(candidate&&!emit(raw+32768,candidate,out))failed=1;
    if(!failed&&!emit(raw,got,out))failed=1;
   }else{failed=1;usleep(10000);}
   candidate=0;
  }else if((owned||failed)&&(dsb_zero(raw,got)||dsb_marked(raw,got)||++invalid<3)){
   candidate=0;emit(0,got,out); /* Pauses/seams get valid DSD silence. */
  }else{
   candidate=0;invalid=0;failed=0;
   if(native_pcm(p)){
    /* Unchanged S32 stereo PCM enters only the native PCM output ring. */
    while(!W(0x839a70)&&W(0x839a74)!=3){
     unsigned capacity=W(p+0x170+0x24),queued=F(0x4cb6b0,unsigned (*)(unsigned))(p+0x170);
     if(capacity>=got&&queued<=capacity-got){F(0x4cb3e4,int (*)(unsigned,void *,unsigned))(p+0x170,raw,got);break;}usleep(1000);
    }
   }else usleep(10000);
  }
 }
done:
 if(owned)yield();if(raw)free(raw);if(out)free(out);worker_live=0;W(0x839a7c)=1;return 0;
}
static int create_hook(void *thread,const void *attr,void *(*start)(void *),void *arg){
 if(enabled&&start==F(0x4591b0,void *(*)(void *))&&eligible((unsigned long)arg))return real_create(thread,attr,worker,arg);
 return real_create(thread,attr,start,arg);
}
static void *open_hook(unsigned card,unsigned device,unsigned flags,const unsigned *cfg){
 if(owned&&card==0&&device==3&&!(flags&0x10000000u)&&!yield())return 0;
 return real_open(card,device,flags,cfg);
}
static int configure_hook(unsigned hz,unsigned bits,unsigned dsd){
 if(worker_live&&scope()&&dsd)return 0;
 if(owned&&!yield())return -1;
 return F(0x4e9534,int (*)(unsigned,unsigned,unsigned))(hz,bits,dsd);
}
static int ioctl_hook(int fd,unsigned request,void *arg){
 int c=fd==(int)W(0x83a7d0)?0:(fd==(int)W(0x83a7cc)?1:-1);
 if(owned&&c>=0){
  if(!scope()){if(!yield())return -1;return real_ioctl(fd,request,arg);}
  unsigned vol0=c?0x80014d2f:0x80014d2d,vol1=c?0x80014d44:0x80014d42;
  if(request==vol0||request==vol1){
   if(!arg)return -1;pthread_mutex_lock(mutex);levels[c]=*(unsigned char *)arg;
   int ok=!ready||dsb_volume(&dac,(unsigned)c,levels[c]);pthread_mutex_unlock(mutex);return ok?0:-1;
  }
  /* While leased, native health/mode/mute writes cannot reinitialise DSD.
   * Native resource release and format callbacks yield the lease first. */
  if((request&0xffff00)==0x004d00||(request&0xffff00)==0x014d00){health_skips++;return 0;}
 }
 return real_ioctl(fd,request,arg);
}
static int output_hook(void *data,int bytes,int quiet){
 if(owned&&scope()){quiet_skips++;usleep(2000);return 0;}
 return F(0x465718,int (*)(void *,int,int))(data,bytes,quiet);
}
static int patch_output(void){
 const unsigned sites[]={0x465970,0x465af8,0x465d64,0x465ec0,0x466338,0x466420,0x466980};
 unsigned *stub=mmap((void *)0x07300000,4096,3,0x802,-1,0),a=(unsigned long)stub,t=(unsigned long)output_hook;
 if(a<0x10000||a>=0x10000000||(a&4095)){if(stub!=(void *)-1)munmap(stub,4096);return 0;}
 stub[0]=0x3c190000|(t>>16);stub[1]=0x37390000|(t&65535);stub[2]=0x03200008;stub[3]=0;
 if(cacheflush(stub,16,3)||mprotect(stub,4096,5)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x465000,8192,7))return 0;
 for(unsigned i=0;i<7;i++)W(sites[i])=0x0c000000|(a>>2);
 __sync_synchronize();int rc=cacheflush((void *)0x465000,8192,3);
 if(rc){for(unsigned i=0;i<7;i++)W(sites[i])=0x0c1195c6;cacheflush((void *)0x465000,8192,3);}
 return !mprotect((void *)0x465000,8192,5)&&!rc;
}
static int gate(void){
 const char *old[]={"/usr/data/disc-runtime/components/disc-usbdac-dop/disabled","/usr/data/disc-runtime/components/disc-usbdac-d2p/disabled","/usr/data/disc-runtime/components/disc-usbdac-dsd-engine/disabled"};
 for(unsigned i=0;i<3;i++)if(access(old[i],0))return 0;
 if(!access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0)||!access("/usr/data/disc-runtime/components/disc-usbdac-dsd-backend/disabled",0))return 0;
 char b[384];int f=open("/proc/version",0),n=f<0?0:read(f,b,383);if(f>=0)close(f);if(n<20||memcmp(b,"Linux version 4.4.94 ",20))return 0;
 for(int i=20;i+5<n;i++)if(!memcmp(b+i,"#236 ",5))return 1;return 0;
}
__attribute__((visibility("default"))) int disc_dsd_backend_install(unsigned abi){
 if(abi!=1||!gate())return 0;if(installed)return 1;
 for(unsigned i=0;i<sizeof(dsb_guards)/sizeof(dsb_guards[0]);i++)if(memcmp((void *)(unsigned long)dsb_guards[i].address,dsb_guards[i].bytes,16))return 0;
 real_ioctl=dlsym(0,"ioctl");real_create=dlsym(0,"pthread_create");real_open=dlsym(0,"pcm_open");real_close=dlsym(0,"pcm_close");real_ready=dlsym(0,"pcm_is_ready");real_prepare=dlsym(0,"pcm_prepare");real_stop=dlsym(0,"pcm_stop");real_error=dlsym(0,"pcm_get_error");
 if(!real_ioctl||!real_create||!real_open||!real_close||!real_ready||!real_prepare||!real_stop||W(0x83a6bc)!=0x4e9534)return 0;
 /* GOT entries can still be lazy-resolver stubs. Exact binary guard above
  * establishes ABI; all targets resolve from already loaded shared libs. */
 if(!patch_output())return 0;
 W(0x83867c)=(unsigned long)create_hook;W(0x838740)=(unsigned long)open_hook;W(0x83820c)=(unsigned long)ioctl_hook;W(0x83a6bc)=(unsigned long)configure_hook;
 installed=enabled=1;return 1;
}
__attribute__((visibility("default"))) void disc_dsd_backend_tick(void){
 if(installed&&W(0x83a6bc)==0x4e9534)W(0x83a6bc)=(unsigned long)configure_hook;
 enabled=gate();unsigned now=seconds();if(now==report_at)return;report_at=now;
 DsbDac *diag=failed_dac.failure_kind?&failed_dac:&dac;
 char b[1200];int n=snprintf(b,sizeof(b),"version=1.0.5\npid=%d\ninstalled=%d\nenabled=%d\nraw_worker=%u\nowned=%u\nready=%u\ncarrier=%u\nDSD_conversion=0\nstock_DSD_calls=0\npackets=%u\npayload_frames=%u\nxruns=%u\nfailures=%u\nerror_code=%x\nstage=%u\npcm_error=%s\nquiet_skips=%u\nhealth_skips=%u\nmarker_seams=%u\ndac_phase=%u\ndac_failure_kind=%u\ndac_chip=%u\ndac_pending=%u\ndac_polls=%u\ndac_irq=%x,%x\ndac_power_before=%x,%x\ndac_power_after=%x,%x\ndac_mask=%x,%x\n",getpid(),installed,enabled,worker_live,owned,ready,rate,packets,frames,xruns,failures,error_code,stage,pcm_error,quiet_skips,health_skips,stream.seams,diag->phase,diag->failure_kind,diag->error_chip,diag->pending,diag->polls,diag->last_irq[0],diag->last_irq[1],diag->before_power[0],diag->before_power[1],diag->after_power[0],diag->after_power[1],diag->after_mask[0],diag->after_mask[1]);
 int fd=open("/tmp/disc-runtime/usbdac-dsd-backend.state",1|0x100|0x200,0600);if(fd>=0){write(fd,b,n);close(fd);}
}
