/* Exercise the actual provider worker/hooks with mocked transport/hardware. */
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#define DISC_BACKEND_TEST 1
static unsigned char memory[0x900000];
static void *address(unsigned long a){return a<sizeof(memory)?memory+a:(void *)a;}
static void *native(unsigned);
#define W(a) (*(unsigned *)address((unsigned long)(a)))
#define B(a) (*(unsigned char *)address((unsigned long)(a)))
#define F(a,t) ((t)native(a))
#define usleep fake_sleep
#define open fake_open
#define read fake_read
#define write fake_write
#define close fake_close
#define access fake_access
#define fcntl fake_fcntl
#define prctl fake_prctl
#define pthread_mutex_lock fake_lock
#define pthread_mutex_unlock fake_unlock
#define cacheflush fake_cacheflush
#include "backend.c"
static unsigned char hw[2][0x100000],source[28224],sent[2000000];
static unsigned simulated_us,hp_pending[2],pll_pending[2],write_count,sent_bytes,consume_count,native_writes,pcm_opens,pcm_closes,locks;
static int inject_errno,errno_value,partial,pcm_only,reject_clock,board_clock;
static unsigned handle[52];
int fake_lock(void *p){assert(p==mutex);assert(!locks);locks=1;return 0;}
int fake_unlock(void *p){assert(p==mutex&&locks);locks=0;return 0;}
int fake_sleep(unsigned n){simulated_us+=n;return 0;}
int fake_open(const char *path,int flags,...){(void)flags;return strstr(path,"0030")?30:strstr(path,"0032")?32:5;}
int fake_close(int f){(void)f;return 0;}
int fake_read(int fd,void *b,size_t n){const char *name=fd==30?"cs43131\n":"cs43131c\n";assert(n>strlen(name));memcpy(b,name,strlen(name));return (int)strlen(name);}
int fake_write(int fd,const void *b,size_t n){(void)fd;(void)b;return (int)n;}
int fake_access(const char *p,int m){(void)p;(void)m;return -1;}
int fake_fcntl(int fd,int cmd,...){assert(fd==27&&(cmd==3||cmd==4));return 0;}
int fake_prctl(int cmd,...){assert(cmd==15);return 0;}
int fake_cacheflush(void *p,size_t n,int c){(void)p;(void)n;(void)c;return 0;}
static int mock_ioctl(int fd,unsigned req,void *arg){
 if(req==0x707){struct Transfer *t=arg;struct Message *m=t->messages;unsigned c=(m->address-0x30)/2;
  assert(c<2);unsigned reg=((unsigned)m->data[0]<<16)|((unsigned)m->data[1]<<8)|m->data[2];
  if(t->count==2){unsigned char v=hw[c][reg];if(reg==0xf0000){v=0;if(hp_pending[c]&&simulated_us>=hp_pending[c]){v|=1;hp_pending[c]=0;}if(pll_pending[c]&&simulated_us>=pll_pending[c]){v|=4;pll_pending[c]=0;}}*m[1].data=v;return 2;}
  unsigned char v=m->data[4];if(reg==0x20000){
   if((v&0x40)&&!(hw[c][reg]&0x40))assert(!hp_pending[c]);
   /* A closed/unstarted ALSA has no LRCK: HP completion cannot arrive. */
   if((v&0x10)&&!(hw[c][reg]&0x10)&&sent_bytes>=3528*8*8){assert(hw[c][0x10010]==0x99&&hw[c][0x80046]==0x21);hp_pending[c]=simulated_us+4000;}
  }if(reg==0x30001&&v==1)pll_pending[c]=simulated_us+3000;hw[c][reg]=v;return 1;
 }
 assert(fd==27&&req==0x800c4150&&locks);struct Xfer {int result;const void *data;unsigned frames;} *x=arg;
 if(inject_errno){errno_value=inject_errno;inject_errno=0;return -1;}
 unsigned n=x->frames;if(partial&&n>100)n=100;
 assert(sent_bytes+n*8<=sizeof(sent));memcpy(sent+sent_bytes,x->data,n*8);sent_bytes+=n*8;write_count++;
 x->result=(int)n;return 0;
}
/* __errno_location is used by libc too; do not replace the process symbol.
 * Mock ioctl sets the real errno slot when asked to inject EAGAIN/XRUN. */
static int io_adapter(int fd,unsigned req,void *arg){int rc=mock_ioctl(fd,req,arg);if(rc<0)*__errno_location()=errno_value;return rc;}
static void *pcm_open_mock(unsigned c,unsigned d,unsigned flags,const unsigned *cfg){
 assert(c==0&&d==3&&!flags&&cfg[0]==2&&cfg[1]==352800&&cfg[4]==7&&cfg[5]==cfg[2]*cfg[3]);
 pcm_opens++;handle[0]=27;return handle;
}
static int pcm_ok(void *p){assert(p==handle);return 1;}
static int pcm_noop(void *p){assert(p==handle);return 0;}
static int pcm_close_mock(void *p){assert(p==handle);pcm_closes++;return 0;}
static void stock_close(unsigned flags){assert(!flags&&!locks);W(0x200000+6*0x54+0x68)=0;}
static unsigned available(unsigned ring){
 if(ring==0x100000+0x170)return 0;
 assert(ring==0x100000+0x130);if(consume_count>=2)W(0x839a70)=1;return consume_count<2?sizeof(source):0;
}
static unsigned raw_read(unsigned ring,void *out,unsigned n){
 assert(ring==0x100000+0x130&&n==sizeof(source));memcpy(out,source,n);consume_count++;
 return n;
}
static int output_push(unsigned ring,void *p,unsigned n){assert(ring==0x100000+0x170&&p&&n==sizeof(source));native_writes++;return n;}
static int stock_cfg(unsigned hz,unsigned bits,unsigned dsd){assert(!dsd&&hz==352800&&bits==32);return 0;}
static int stock_pcm_open(unsigned hz,unsigned bits,unsigned ch,unsigned flags,unsigned dsd){assert(!dsd&&!flags&&ch==2&&hz==352800&&bits==32);W(0x200000+6*0x54+0x68)=1;return 0;}
static void stock_unmute(unsigned mute){assert(!mute);}
static void *stock_worker(void *p){(void)p;return 0;}
static void *native(unsigned a){switch(a){
 case 0x478354:return stock_close;case 0x4cb6b0:return available;case 0x4cb534:return raw_read;
 case 0x4cb3e4:return output_push;case 0x4e9534:return stock_cfg;case 0x47859c:return stock_pcm_open;
 case 0x4e8b94:return stock_unmute;case 0x4591b0:return stock_worker;
 default:assert(!"Unexpected native DSD function invoked");return 0;}}
static void reset(void){
 memset(memory,0,sizeof(memory));memset(hw,0,sizeof(hw));memset(&dac,0,sizeof(dac));
 simulated_us=sent_bytes=write_count=consume_count=native_writes=pcm_opens=pcm_closes=locks=0;
 hp_pending[0]=hp_pending[1]=pll_pending[0]=pll_pending[1]=0;
 owned=ready=worker_live=rate=packets=frames=failures=xruns=0;bus=-1;pcm=0;levels[0]=levels[1]=255;
 W(0x83dfc0)=0x300000;W(0x300000)=4;W(0x83dfe4)=0x200000;W(0x200000+0x58)=6;B(0x83a75a)=6;
 unsigned p=0x100000;W(p+0xc)=W(p+0x1c)=352800;W(p+0x10)=W(p+0x20)=2;W(p+0x14)=W(p+0x24)=32;
 W(p+0x170+0x24)=200000;W(0x200000+6*0x54+0x68)=1;
 for(unsigned c=0;c<2;c++){
  hw[c][0x10000]=0x43;hw[c][0x10001]=0x13;hw[c][0x10002]=0x10;
  hw[c][0x10006]=reject_clock?5:(board_clock?4:1);hw[c][0x40018]=12;hw[c][0x40019]=10;
  hw[c][0x5000a]=hw[c][0x5000b]=c?15:7;
  hw[c][0x20000]=0xa0;hw[c][0x90004]=c?3:1;hw[c][0x90001]=40;
  hw[c][0xb0000]=0x0b;hw[c][0x80046]=0x20;hw[c][0x90003]=0xac;
 }
 for(unsigned i=0;i<sizeof(source);i+=8){source[i]=source[i+4]=0;source[i+3]=source[i+7]=(i/8&1)?250:5;
  source[i+1]=(unsigned char)i;source[i+2]=(unsigned char)(i/3);source[i+5]=(unsigned char)(i/7);source[i+6]=(unsigned char)(255-i);}
 if(pcm_only)memset(source,0x24,sizeof(source));
 real_ioctl=io_adapter;real_open=pcm_open_mock;real_ready=pcm_ok;real_prepare=real_stop=pcm_noop;real_close=pcm_close_mock;
}
int main(void){
 reset();worker((void *)0x100000);assert(W(0x839a7c)==1&&!owned&&!worker_live&&!locks);
 assert(packets==2&&frames==sizeof(source)/4&&pcm_opens==1&&pcm_closes==1&&!native_writes&&!failures);
 unsigned bootstrap=0;while(bootstrap<sent_bytes&&sent[bootstrap]==0)bootstrap++;bootstrap&=~7u;
 unsigned prefill=bootstrap+3528*8*8;assert(sent_bytes>=prefill+2*sizeof(source));
 for(unsigned i=0;i<bootstrap;i++)assert(sent[i]==0);
 for(unsigned i=bootstrap;i<sent_bytes;i+=8){assert(sent[i+3]==(((i-bootstrap)/8&1)?250:5)&&sent[i+7]==sent[i+3]);
  if(i>=prefill&&i<prefill+2*sizeof(source)){unsigned j=(i-prefill)%sizeof(source);assert(!memcmp(sent+i+1,source+j+1,2)&&!memcmp(sent+i+5,source+j+5,2));}
  else assert(sent[i+1]==0x69&&sent[i+2]==0x69&&sent[i+5]==0x69&&sent[i+6]==0x69);}
 pcm_only=1;reset();worker((void *)0x100000);assert(native_writes==2&&pcm_opens==0&&sent_bytes==0);
 pcm_only=0;partial=1;reset();worker((void *)0x100000);assert(packets==2&&sent_bytes>=prefill+2*sizeof(source)&&write_count>200);
 partial=0;reject_clock=1;reset();worker((void *)0x100000);assert(failures==1&&sent_bytes==0&&!owned&&!locks);
 reject_clock=0;board_clock=1;reset();worker((void *)0x100000);assert(packets==2&&sent_bytes>=prefill+2*sizeof(source)&&!failures);
 assert(hw[0][0x10006]==4&&hw[1][0x10006]==4);board_clock=0;
 reject_clock=0;reset();owned=ready=1;pcm=handle;handle[0]=27;W(0x839a74)=1;inject_errno=11;pthread_mutex_lock(mutex);
 assert(write_output(source,800)&&sent_bytes==800);inject_errno=32;assert(!write_output(source,800));pthread_mutex_unlock(mutex);
 puts("PASS: actual raw worker, independent ALSA prefill/write, PCM forwarding, partial WRITEI, EAGAIN/XRUN, unknown-clock failure, stop cleanup, lock order and zero native DSD calls.");return 0;
}
