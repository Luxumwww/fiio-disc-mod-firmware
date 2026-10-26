/* Scoped to mq_player: forward real tinyalsa reads unchanged, send bounded copies
 * from the FIIO_UAC2 capture card. No second PCM reader, FFT, file logging or wait. */
#include "pcm_packet.h"
extern void *dlsym(void *,const char *);
extern int *__errno_location(void);
extern int open(const char *,int,...),close(int),socket(int,int,int),fcntl(int,int,...);
extern long read(int,void *,unsigned long),sendto(int,const void *,unsigned long,int,const void *,unsigned);
static void *(*real_open)(unsigned,unsigned,unsigned,const void *);
static int (*real_close)(void *),(*real_read)(void *,void *,unsigned);
static unsigned (*get_channels)(const void *),(*get_rate)(const void *),(*frames_bytes)(const void *,unsigned);
static int (*get_format)(const void *),(*format_bits)(int),(*is_ready)(const void *);
typedef struct {void *pcm;unsigned ch,rate,format,bytes,position,epoch,gap;} Capture;
static Capture captures[4];static volatile unsigned busy,gap_generation;static unsigned epoch_counter;
static int transport=-1;
static struct {unsigned short family;char path[108];} destination={1,DISC_PCM_SOCKET};
#ifndef DISC_CARDS_FILE
#define DISC_CARDS_FILE "/proc/asound/cards"
#endif
static int card_is_uac(unsigned card){
 char cards[4096];int fd=open(DISC_CARDS_FILE,0);
 if(fd<0)return 0;
 long n=read(fd,cards,sizeof(cards)-1);close(fd);if(n<=0)return 0;cards[n]=0;
 for(unsigned i=0;i<(unsigned)n;){
  unsigned start=i;while(i<(unsigned)n&&cards[i]!='\n')i++;
  unsigned end=i; i++;
  while(start<end&&cards[start]==' ')start++;
  if(start==end||cards[start]<'0'||cards[start]>'9')continue;
  unsigned value=0;while(start<end&&cards[start]>='0'&&cards[start]<='9')value=value*10+cards[start++]-'0';
  if(value!=card)continue;
  const char token[]="FIIO_UAC2_Gadge - FIIO_UAC2_Gadget";
  for(unsigned j=start;j+sizeof(token)-1<=end;j++){
   unsigned k=0;while(k<sizeof(token)-1&&cards[j+k]==token[k])k++;
   if(k==sizeof(token)-1)return 1;
  }
 }
 return 0;
}
__attribute__((constructor)) static void init_tap(void){
 void *next=(void *)-1;
 real_open=dlsym(next,"pcm_open");real_close=dlsym(next,"pcm_close");real_read=dlsym(next,"pcm_read");
 get_channels=dlsym(next,"pcm_get_channels");get_rate=dlsym(next,"pcm_get_rate");get_format=dlsym(next,"pcm_get_format");
 frames_bytes=dlsym(next,"pcm_frames_to_bytes");format_bits=dlsym(next,"pcm_format_to_bits");is_ready=dlsym(next,"pcm_is_ready");
#ifdef __mips__
 transport=socket(1,1,0); /* Linux MIPS SOCK_DGRAM differs from the host ABI. */
#else
 transport=socket(1,2,0);
#endif
 if(transport>=0){
#ifdef __mips__
  const int nonblock=0x80;
#else
  const int nonblock=0x800;
#endif
  if(fcntl(transport,4,nonblock)<0||fcntl(transport,2,1)<0){close(transport);transport=-1;}
 }
}
void *pcm_open(unsigned card,unsigned device,unsigned flags,const void *config){
 if(!real_open){*__errno_location()=89;return (void *)0;}
 void *pcm=real_open(card,device,flags,config);int saved=*__errno_location();
 if(pcm&&(flags&0x10000000U)&&get_channels&&get_rate&&get_format&&frames_bytes&&format_bits&&is_ready&&is_ready(pcm)&&card_is_uac(card)){
  unsigned ch=get_channels(pcm),rate=get_rate(pcm),stride=frames_bytes(pcm,1);
  int bits=format_bits(get_format(pcm));unsigned bytes=ch?stride/ch:0;
  unsigned format=bits==16&&bytes==2?2:bits==24&&bytes==3?32:bits==24&&bytes==4?6:bits==32&&bytes==4?10:0;
  if(ch>=1&&ch<=2&&stride==bytes*ch&&rate>=8000&&rate<=768000&&format){
   while(__sync_lock_test_and_set(&busy,1)){}
   for(unsigned i=0;i<4;i++)if(!captures[i].pcm){captures[i]=(Capture){pcm,ch,rate,format,bytes,0,++epoch_counter,gap_generation};break;}
   __sync_lock_release(&busy);
  }
 }
 *__errno_location()=saved;return pcm;
}
int pcm_close(void *pcm){
 if(!real_close){*__errno_location()=89;return -1;}
 /* Close is a control operation. Clear the slot after the real close; concurrent
  * reads of an already closing handle are the caller's synchronization contract. */
 int result=real_close(pcm),saved=*__errno_location();
 while(__sync_lock_test_and_set(&busy,1)){}
 for(unsigned i=0;i<4;i++)if(captures[i].pcm==pcm)captures[i].pcm=0;
 __sync_lock_release(&busy);
 *__errno_location()=saved;return result;
}
int pcm_read(void *pcm,void *data,unsigned count){
 if(!real_read){*__errno_location()=89;return -1;}
 int result=real_read(pcm,data,count),saved=*__errno_location();
 if(result==0&&data&&count&&transport>=0){
  if(__sync_lock_test_and_set(&busy,1)){__sync_fetch_and_add(&gap_generation,1);*__errno_location()=saved;return result;}
  for(unsigned i=0;i<4;i++)if(captures[i].pcm==pcm){
   Capture *c=&captures[i];unsigned stride=c->bytes*c->ch;
   if(c->gap!=gap_generation){c->epoch=++epoch_counter;c->position=0;c->gap=gap_generation;}
   if(count%stride)break;
   unsigned frames=count/stride,offset=0;
   /* Bound work even for an unexpected huge caller buffer: keep the tail,
    * publishing the true frame position so the consumer can reset across gaps. */
   if(frames>4096)offset=frames-4096;
   while(offset<frames){
    unsigned n=frames-offset;if(n>DISC_PCM_FRAMES)n=DISC_PCM_FRAMES;
    DiscPcmPacket packet={DISC_PCM_MAGIC,1,c->format,c->rate,c->ch,n,c->position+offset,c->epoch,{0}};
    const unsigned char *src=(const unsigned char *)data+offset*stride;
    for(unsigned j=0;j<n*stride;j++)packet.pcm[j]=src[j];
    long sent=sendto(transport,&packet,32+n*stride,0x40,&destination,sizeof(destination));
    offset+=n;
    if(sent<0)break; /* Full/missing receiver: drop immediately, preserve playback. */
   }
   c->position+=frames;break;
  }
  __sync_lock_release(&busy);
 }
 *__errno_location()=saved;return result;
}
