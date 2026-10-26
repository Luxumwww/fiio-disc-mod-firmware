/* DISC v2.57 BlueALSA extension. Keep the vendor daemon, its private pipe
 * server, SBC/AAC/LDAC implementations and D-Bus/ALSA contracts intact.
 * Only guarded direct call sites and the spare codec-table terminator change
 * in this process. The immutable executable on flash is never modified. */
#define _GNU_SOURCE
#include "disc_lhdcv5.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#define ID 0x35ffu
#define FN(a,t) ((t)(uintptr_t)(a))
#define WORD(a) (*(uint32_t *)(uintptr_t)(a))
#define U16(p,o) (*(uint16_t *)((uint8_t *)(p)+(o)))
#define U32(p,o) (*(uint32_t *)((uint8_t *)(p)+(o)))
extern int cacheflush(void *,int,int);

/* Layout verified against this exact vendor binary, not inferred from an
 * upstream structure compiled against a different libc. Capabilities are
 * packed at byte 7; selected transport config at 0x1a4; sink PCM at 0x1c0. */
static uint8_t codec[64] __attribute__((aligned(8)));
static const uint8_t caps[11]={0x3a,5,0,0,0x35,0x4c,0x15,0x86,0xd1,0,0};
struct shared_state {
 uint32_t magic,seq,version,active;
 disc_lhdcv5_stats stats;
 uint32_t max_kbps,min_kbps;
 uint8_t configuration[12];
 uint32_t daemon_pid;
 disc_lhdcv5_quality_stats quality;
};
static struct shared_state *shared;
static void publish(disc_lhdcv5 *dec,int active){
 if(!shared)return;
 shared->seq++;__sync_synchronize();
 if(dec)disc_lhdcv5_get_stats(dec,&shared->stats);
 if(dec)disc_lhdcv5_get_quality(&shared->quality);
 shared->active=active;__sync_synchronize();shared->seq++;
}
static const char *name_hook(unsigned id){
 return id==ID?"LHDC-V5":FN(0x4048ec,const char *(*)(unsigned))(id);
}
static unsigned from_name_hook(const char *s){
 if(s&&(!strcasecmp(s,"LHDC-V5")||!strcasecmp(s,"LHDCV5")))return ID;
 return FN(0x404830,unsigned (*)(const char *))(s);
}
static unsigned vendor_hook(const uint8_t *p,size_t n){
 if(n>=6&&!memcmp(p,caps,6))return ID;
 return FN(0x4054c4,unsigned (*)(const void *,size_t))(p,n);
}
static int is_v5(const void *c){return c&&U16(c,4)==ID;}
static unsigned check_hook(const void *c,const uint8_t *p,size_t n){
 if(!is_v5(c))return FN(0x40561c,unsigned (*)(const void *,const void *,size_t))(c,p,n);
 disc_lhdcv5_config cfg;
 if(n!=11)return UINT32_MAX;
 if(disc_lhdcv5_parse_config(p,n,&cfg)||!(p[6]&caps[6]))return 4;
 /* Optional lossless/AR/JAS/DSEE features are not implemented. */
 if((p[7]&1)||p[9]||p[10])return 4;
 return 0;
}
static int filter_hook(const void *c,uint8_t *p,size_t n){
 if(!is_v5(c))return FN(0x4058e4,int (*)(const void *,void *,size_t))(c,p,n);
 if(n!=11||memcmp(p,caps,6)){errno=EINVAL;return -1;}
 if((p[8]&15)!=1){errno=ENOTSUP;return -1;}
 p[6]&=caps[6];p[7]=(p[7]&0xf0)|(p[7]&6);p[8]=0x11;p[9]=p[10]=0;
 if(!(p[6]&0x35)||!(p[7]&6)){errno=ENOTSUP;return -1;}
 return 0;
}
static int select_hook(const void *c,uint8_t *p,size_t n){
 if(!is_v5(c))return FN(0x405cc4,int (*)(const void *,void *,size_t))(c,p,n);
 if(filter_hook(c,p,n))return -1;
 unsigned r=p[6];p[6]=r&1?1:r&4?4:0x10;
 p[7]=(p[7]&0xf0)|(p[7]&2?2:4);
 return check_hook(c,p,n)?-1:0;
}
static const char *path_hook(unsigned type){
 if(type>>16==ID){
  if((type&0xffff)==2)return "/A2DP/LHDCV5/sink";
  return "/A2DP/LHDCV5/source";
 }
 return FN(0x416024,const char *(*)(unsigned))(type);
}
static void set_codec_hook(void *t,unsigned id){
 if(id!=ID){FN(0x40bf18,void (*)(void *,unsigned))(t,id);return;}
 disc_lhdcv5_config cfg;
 if(!(U16(t,0x1c)&2)||disc_lhdcv5_parse_config((uint8_t *)t+0x1a4,11,&cfg)){
  /* SetConfiguration is checked before creating this transport. An invalid
   * internal state must not start an unconfigured decoder. */
  U16(t,0x1e)=0xffff;return;
 }
 U16(t,0x1e)=ID;U16(t,0x1ea)=0x8420;U32(t,0x1ec)=2;U32(t,0x1f0)=cfg.rate;
}
static int codec_init_hook(void){
 int r=FN(0x4052a0,int (*)(void))();
 /* Stock launch specifies only SBC/AAC/LDAC and first clears all enabled
  * flags. Enable the extra sink after the original argument parsing. */
 codec[60]=1;return r;
}
static int emit(void *pcm,const int32_t *samples,size_t frames){
 int32_t scaled[1920];size_t n=frames*2;
 if(n>1920){errno=EOVERFLOW;return -1;}
 memcpy(scaled,samples,n*4);
 FN(0x414198,void (*)(void *,void *,size_t))(pcm,scaled,n);
 return FN(0x4144b4,int (*)(void *,const void *,size_t))(pcm,scaled,n)==(int)n?0:-1;
}
static void close_decoder(void *p){publish(p,0);disc_lhdcv5_close(p);}
static void *decode_thread(void *th){
 pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
 pthread_cleanup_push(FN(0x40d13c,void (*)(void *)),th);
 void *t=*(void **)th;uint8_t io[56] __attribute__((aligned(8)))={0};
 uint8_t packet[8192];U32(io,48)=UINT32_MAX;
 disc_lhdcv5 *dec=disc_lhdcv5_open((uint8_t *)t+0x1a4,11);
 if(dec){
  pthread_cleanup_push(close_decoder,dec);
  void *pcm=(uint8_t *)t+0x1c0;
  if(shared){
   const uint8_t *cfg=(uint8_t *)t+0x1a4;
   unsigned maximum=cfg[7]&0x30,minimum=cfg[7]&0xc0;
   shared->max_kbps=maximum==0x10?390:maximum==0x20?520:maximum==0x30?900:(U32(t,0x1f0)<=48000?900:1000);
   /* For the regular 5ms layout; no extended/lossless bitrate feature. */
   shared->min_kbps=U32(t,0x1f0)>48000?(minimum==0xc0?390:260):(minimum==0x40?160:minimum==0x80?260:minimum==0xc0?390:200);
   memcpy(shared->configuration,cfg,11);shared->daemon_pid=getpid();
  }
  FN(0x40b660,int (*)(void *,unsigned,int))(th,2,0);publish(dec,1);
  unsigned count=0;
  for(;;){
   ssize_t n=FN(0x414660,ssize_t (*)(void *,void *,void *,size_t))(io,th,packet,sizeof(packet));
   if(n<=0)break;
   if(!FN(0x40c14c,int (*)(void *))(pcm)){disc_lhdcv5_flush(dec);continue;}
   if(disc_lhdcv5_packet(dec,packet,(size_t)n,emit,pcm)<0&&errno==EIO)disc_lhdcv5_flush(dec);
   if(++count%64==0)publish(dec,1);
  }
  FN(0x40b660,int (*)(void *,unsigned,int))(th,3,0);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_cleanup_pop(1);
 }
 pthread_cleanup_pop(1);
 return NULL;
}
static int start_hook(void *t){
 if(U16(t,0x1e)!=ID)return FN(0x40cef0,int (*)(void *))(t);
 pthread_t main_thread=(pthread_t)WORD(0x43352c);
 if(!pthread_equal((pthread_t)U32(t,0x88),main_thread)||!pthread_equal((pthread_t)U32(t,0x108),main_thread)){
  errno=EEXIST;return -1;
 }
 if(!(U16(t,0x1c)&2)){errno=ENOTSUP;return -1;}
 return FN(0x40cddc,int (*)(void *,void *(*)(void *),const char *,int))((uint8_t *)t+0xd0,decode_thread,"ba-a2dp-lhdcv5",1);
}
static void *const hooks[]={name_hook,from_name_hook,vendor_hook,check_hook,filter_hook,
 select_hook,path_hook,set_codec_hook,codec_init_hook,start_hook};
#include "build/stock_sites.h"

static void fail(const char *s){write(2,s,strlen(s));_exit(125);}
struct pcm_test {uint8_t pcm[192];int read_fd;uint32_t hash;uint64_t frames;};
static int test_emit(void *user,const int32_t *samples,size_t frames){
 struct pcm_test *s=user;uint8_t actual[7680];size_t size=frames*8;
 if(emit(s->pcm,samples,frames)){printf("PCM emit failure errno=%d\n",errno);return -1;}
 size_t got=0;while(got<size){ssize_t n=read(s->read_fd,actual+got,size-got);if(n<=0)return -1;got+=(size_t)n;}
 for(size_t i=0;i<frames*2;i++)if(((int32_t *)actual)[i]!=samples[i]){
  printf("PCM difference sample=%zu input=%d output=%d\n",i,samples[i],((int32_t *)actual)[i]);return -1;
 }
 for(size_t i=0;i<size;i++)s->hash=(s->hash^actual[i])*16777619u;
 s->frames+=frames;return 0;
}
static int test_pcm_path(const char *dir){
 static const unsigned rates[]={48000,96000,192000};
 /* Candidate0.1.6 corrected output gain; generated by MIPS reference replay. */
 static const unsigned expected[]={3854893515u,1400845837u,2948964985u};
 for(unsigned i=0;i<3;i++){
  char path[512];snprintf(path,sizeof(path),"%s/capture-%u.packets",dir,rates[i]);
  FILE *f=fopen(path,"rb");if(!f)return -1;
  uint8_t cfg[11];memcpy(cfg,caps,11);cfg[6]=i==0?0x10:i==1?4:1;cfg[7]=0x82;cfg[8]=0x11;
  disc_lhdcv5 *d=disc_lhdcv5_open(cfg,11);int fd[2];
  if(!d||pipe2(fd,O_NONBLOCK|O_CLOEXEC)){fclose(f);disc_lhdcv5_close(d);return -1;}
  if(fcntl(fd[1],F_SETPIPE_SZ,16384)<8192){close(fd[0]);close(fd[1]);fclose(f);disc_lhdcv5_close(d);return -1;}
  struct pcm_test s={.read_fd=fd[0],.hash=2166136261u};
  pthread_mutex_init((pthread_mutex_t *)(s.pcm+12),NULL);
  U32(s.pcm,36)=fd[1];U16(s.pcm,42)=0x8420;U32(s.pcm,44)=2;U32(s.pcm,48)=rates[i];
  /* Stock zero scale explicitly mutes even with soft-volume disabled. Set
   * unity gain for both channels, as its real PCM initializer does. */
  *(double *)(s.pcm+72)=1.0;*(double *)(s.pcm+88)=1.0;
  uint8_t head[8],packet[8192];int ok=fread(head,1,8,f)==8&&!memcmp(head,"DLHDC001",8);unsigned count=0;
  while(ok&&fread(head,1,4,f)==4){
   unsigned n=head[0]|head[1]<<8|head[2]<<16|head[3]<<24;
   if(n>sizeof(packet)||fread(packet,1,n,f)!=n||disc_lhdcv5_packet(d,packet,n,test_emit,&s)<0){ok=0;break;}
   count++;
  }
  disc_lhdcv5_stats st;disc_lhdcv5_get_stats(d,&st);
  ok=ok&&count==512&&!st.decode_errors&&!st.pending_bytes&&s.hash==expected[i]&&s.frames==st.pcm_frames;
  printf("PCM path %u: packets=%u frames=%llu fnv=%u result=%s\n",rates[i],count,(unsigned long long)s.frames,s.hash,ok?"PASS":"FAIL");
  pthread_mutex_destroy((pthread_mutex_t *)(s.pcm+12));close(fd[0]);close(fd[1]);fclose(f);disc_lhdcv5_close(d);
  if(!ok)return -1;
 }
 return 0;
}
/* Isolated constructor selftest: execute the actual native codec switch and
 * our trampoline, replacing only its two exits with returning test markers.
 * Restore the exits before main. No D-Bus/transport/UI process is involved. */
extern int query_switch_probe(void *);
__asm__(".text\n.set push\n.set noreorder\n"
 ".hidden query_switch_probe\n.type query_switch_probe,@function\n"
 "query_switch_probe:\naddiu $sp,$sp,-32\nsw $ra,28($sp)\n"
 "sw $s3,24($sp)\nmove $s3,$a0\nlui $t9,0x40\nori $t9,$t9,0xdb34\njalr $t9\nnop\n"
 "lw $s3,24($sp)\nlw $ra,28($sp)\njr $ra\naddiu $sp,$sp,32\n"
 ".size query_switch_probe,.-query_switch_probe\n.set pop\n");
static int test_query_switch(void){
 static const unsigned ids[]={0,2,0x2dff,ID,0x4fff,0x24ff,0xffff};
 uint32_t saved[]={WORD(0x40db60),WORD(0x40db64),WORD(0x40db74),WORD(0x40db78)};
 if(mprotect((void *)0x40d000,4096,PROT_READ|PROT_WRITE|PROT_EXEC))return -1;
 WORD(0x40db60)=0x03e00008;WORD(0x40db64)=0x24020000;
 WORD(0x40db74)=0x03e00008;WORD(0x40db78)=0x24020001;
 if(cacheflush((void *)0x40d000,4096,3))return -1;
 int ok=1;uint8_t object[64]={0};
 for(unsigned i=0;i<sizeof(ids)/sizeof(ids[0]);i++){
  U16(object,4)=ids[i];int actual=query_switch_probe(object);
  printf("GetCodecs native switch id=%04x accepted=%d\n",ids[i],actual);
  if(actual!=(i<4))ok=0;
 }
 WORD(0x40db60)=saved[0];WORD(0x40db64)=saved[1];
 WORD(0x40db74)=saved[2];WORD(0x40db78)=saved[3];
 if(cacheflush((void *)0x40d000,4096,3)||mprotect((void *)0x40d000,4096,PROT_READ|PROT_EXEC))return -1;
 return ok?0:-1;
}
static int selftest(void){
 if(test_query_switch())return -1;
 uint8_t c[11];memcpy(c,caps,11);
 if(select_hook(codec,c,11)||check_hook(codec,c,11)||c[6]!=1||c[7]!=0x82)return -1;
 if(vendor_hook(c,11)!=ID||from_name_hook("LHDC-V5")!=ID||strcmp(name_hook(ID),"LHDC-V5"))return -1;
 if(strcmp(path_hook(ID<<16|2),"/A2DP/LHDCV5/sink"))return -1;
 if(strcmp(name_hook(0),"SBC")||strcmp(name_hook(2),"AAC")||strcmp(name_hook(0x2dff),"LDAC"))return -1;
 uint8_t transport[1024] __attribute__((aligned(8)))={0};
 U16(transport,0x1c)=2;memcpy(transport+0x1a4,c,11);set_codec_hook(transport,ID);
 if(U16(transport,0x1ea)!=0x8420||U32(transport,0x1ec)!=2||U32(transport,0x1f0)!=192000)return -1;
 /* Exercise original SBC/AAC/LDAC validation, selection and config formats
  * through patched executable call sites, not just the hook's custom branch. */
 for(unsigned i=0;i<6;i++){
  void *old=(void *)(uintptr_t)WORD(0x41acc8+i*4);
  size_t size=U32(old,24);memcpy(c,(uint8_t *)old+7,size);
  if(select_hook(old,c,size)||check_hook(old,c,size))return -1;
 }
 return 0;
}
__attribute__((constructor)) static void install(void){
 /* Check the ELF header before touching fixed virtual addresses. The launch
  * wrapper also verifies the complete SHA-256 of this immutable stock copy. */
 uint8_t header[64];int fd=open("/proc/self/exe",O_RDONLY);
 if(fd<0||read(fd,header,64)!=64||memcmp(header,stock_header,64))fail("LHDCV5: wrong executable\n");
 close(fd);
 for(size_t i=0;i<sizeof(guards)/sizeof(guards[0]);i++)if(WORD(guards[i].address)!=guards[i].word)fail("LHDCV5: ABI guard failed\n");
 for(size_t i=0;i<sizeof(sites)/sizeof(sites[0]);i++)if(WORD(sites[i].address)!=sites[i].word)fail("LHDCV5: call-site guard failed\n");
 /* GetCodecs inlines ba_variant_populate_sep. Its stock switch accepts only
  * SBC/AAC/LDAC after building Capabilities, and asserts on any extra codec.
  * Preserve that switch; recognize V5 at its default path before asserting. */
 if(WORD(0x40db58)!=0x3c050042||WORD(0x40db5c)!=0xafa00010||
    WORD(0x40db60)!=0x24e7cef0||WORD(0x40db74)!=0x0c10123b)
  fail("LHDCV5: GetCodecs guard failed\n");
 U32(codec,0)=1;U16(codec,4)=ID;memcpy(codec+7,caps,11);U32(codec,24)=11;codec[60]=1;
 /* One new pointer replaces the original null, whose adjacent alignment
  * word is also zero and has no code/data references. That next word remains
  * unchanged as the new terminator; all six original pointers remain intact. */
 uint32_t *stub=mmap((void *)0x07600000,4096,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
 if(stub==MAP_FAILED||(uintptr_t)stub>=0x10000000u)fail("LHDCV5: trampoline allocation failed\n");
 for(unsigned i=0;i<sizeof(hooks)/sizeof(hooks[0]);i++){
  uintptr_t p=(uintptr_t)hooks[i];stub[i*4]=0x3c190000|(p>>16);stub[i*4+1]=0x37390000|(p&65535);stub[i*4+2]=0x03200008;stub[i*4+3]=0;
 }
 uint32_t *query=stub+192;
 query[0]=0x240535ff; /* li a1,LHDC-V5; v0 is the original codec ID */
 query[1]=0x10450003; /* beq v0,a1,query[5] */
 query[2]=0x3c050042; /* delay: original lui a1,0x42 for default assertion */
 query[3]=0x081036d8; /* j 0x40db60: original unsupported-codec assertion */
 query[4]=0;
 query[5]=0x081036dd; /* j 0x40db74: original add/clear capabilities builder */
 query[6]=0;
 if(cacheflush(stub,4096,3)||mprotect(stub,4096,PROT_READ|PROT_EXEC))fail("LHDCV5: trampoline protection failed\n");
 /* All patching happens in the constructor, before main creates any threads.
  * Make every needed page writable before applying the first mutation. */
 for(size_t i=0;i<sizeof(pages)/sizeof(pages[0]);i++)if(mprotect((void *)(uintptr_t)pages[i],4096,PROT_READ|PROT_WRITE|PROT_EXEC))fail("LHDCV5: text protection failed\n");
 for(size_t i=0;i<sizeof(sites)/sizeof(sites[0]);i++)WORD(sites[i].address)=(sites[i].word&0xfc000000)|(((uintptr_t)stub+sites[i].hook*16)>>2);
 WORD(0x40db58)=0x08000000|((uintptr_t)query>>2);
 WORD(0x41ace0)=(uintptr_t)codec;__sync_synchronize();
 for(size_t i=0;i<sizeof(pages)/sizeof(pages[0]);i++)if(cacheflush((void *)(uintptr_t)pages[i],4096,3)||mprotect((void *)(uintptr_t)pages[i],4096,PROT_READ|PROT_EXEC))fail("LHDCV5: text restore failed\n");
 const char *test=getenv("DISC_LHDCV5_SELFTEST");int testing=test&&*test=='1';
 /* Never inject this fixed-address extension into a helper executed by the
  * stock service. Dynamic linking has already loaded both owned libraries. */
 unsetenv("LD_PRELOAD");unsetenv("DISC_LHDCV5_SELFTEST");
 if(testing){
  if(selftest())fail("LHDCV5: selftest failed\n");
  const char *packet_dir=getenv("DISC_LHDCV5_PACKETSET");
  if(packet_dir&&test_pcm_path(packet_dir))fail("LHDCV5: PCM path test failed\n");
  unsetenv("DISC_LHDCV5_PACKETSET");
  puts("PASS stock extension: LHDC V5 registration, configuration, PCM layout; original SBC/AAC/LDAC configuration retained");return;
 }
 fd=open("/tmp/disc-runtime/lhdcv5.state",O_RDWR|O_CREAT|O_TRUNC,0600);
 if(fd>=0){
  if(!ftruncate(fd,sizeof(*shared))){
   void *p=mmap(NULL,sizeof(*shared),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
   if(p!=MAP_FAILED){shared=p;memset(shared,0,sizeof(*shared));shared->magic=0x3543484c;shared->version=3;}
  }
  close(fd);
 }
}


