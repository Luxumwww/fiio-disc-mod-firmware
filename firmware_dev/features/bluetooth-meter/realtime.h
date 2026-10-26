/* Passive kernel HCI monitor. Copies of RX packets only: no HCI commands,
 * no audio consumer, no writer, no thread. Executed by the existing UI loop.
 * Count encoded A2DP payload bytes after HCI/L2CAP/RTP/media headers. */
extern int socket(int,int,int),bind(int,const void *,unsigned),fcntl(int,int,...);
extern int recv(int,void *,unsigned,int);
#include "lhdcv5_bitrate.h"
static int monitorfd=-1;
static unsigned monitor_opened,monitor_last,monitor_retry,realtime_kbps;
static unsigned monitor_packets,monitor_invalid,monitor_capped;
static struct {unsigned handle,used,expected,cid;unsigned char data[4096];} fragments[4];
static struct {unsigned at,bytes;} arrivals[96];
static unsigned arrival_write;
static unsigned le16(const unsigned char *p){return p[0]|((unsigned)p[1]<<8);}
static unsigned be16(const unsigned char *p){return ((unsigned)p[0]<<8)|p[1];}
static void monitor_close(void){
 if(monitorfd>=0)close(monitorfd);monitorfd=-1;realtime_kbps=0;monitor_opened=monitor_last=0;arrival_write=0;
 v5_bitrate_reset();
 for(unsigned i=0;i<96;i++)arrivals[i].bytes=arrivals[i].at=0;
 for(unsigned i=0;i<4;i++)fragments[i].used=fragments[i].expected=0;
}
static unsigned media_bytes(const unsigned char *p,unsigned n,const char *codec){
 if(n<13||(p[0]>>6)!=2||(p[1]&127)!=96)return 0;
 unsigned offset=12+(p[0]&15)*4;if(offset>n)return 0;
 if(p[0]&16){if(offset+4>n)return 0;unsigned ext=be16(p+offset+2)*4;if(ext>n-offset-4)return 0;offset+=4+ext;}
 if(p[0]&32){unsigned pad=p[n-1];if(!pad||pad>n-offset)return 0;n-=pad;}
 if(!strcmp(codec,"LDAC")){
  if(n-offset<4||!(p[offset]&15)||p[offset]&0xf0||p[offset+1]!=0xaa)return 0;
  return n-offset-1;
 }
 if(!strcmp(codec,"SBC")){
  if(n-offset<5||!(p[offset]&15)||p[offset]&0xf0||p[offset+1]!=0x9c)return 0;
  return n-offset-1;
 }
 /* BlueALSA 4.0.0 AAC carries LATM directly after RTP, including when
  * fragmented across RTP packets. Count the encoded transport payload;
  * excludes HCI, L2CAP and RTP, includes AAC's small LATM framing. */
 if(!strcmp(codec,"AAC"))return n-offset;
 /* Verified V5 captures: two media-header bytes then encoded frame data.
  * Count encoded transport bytes, including an incomplete carried frame. */
 if(!strcmp(codec,"LHDC V5"))return n-offset>=2?n-offset-2:0;
 /* Other codecs require a verified media-payload format before counting. */
 return 0;
}
static unsigned monitor_packet(const unsigned char *p,unsigned n,const char *codec){
 if(n<10||le16(p)!=5||le16(p+2)!=0||le16(p+4)!=n-6)return 0;
 p+=6;n-=6;unsigned handle=le16(p),len=le16(p+2),pb=(handle>>12)&3;handle&=0xfff;
 if(len!=n-4){monitor_invalid++;return 0;}p+=4;n-=4;
 unsigned slot=4;for(unsigned i=0;i<4;i++)if(fragments[i].expected&&fragments[i].handle==handle){slot=i;break;}
 if(pb==0||pb==2){
  if(n<4)return 0;if(slot==4)for(unsigned i=0;i<4;i++)if(!fragments[i].expected){slot=i;break;}
  if(slot==4){monitor_invalid++;return 0;}
  fragments[slot].handle=handle;fragments[slot].expected=le16(p);fragments[slot].cid=le16(p+2);fragments[slot].used=0;p+=4;n-=4;
  if(fragments[slot].cid<0x40||!fragments[slot].expected||fragments[slot].expected>4096){fragments[slot].expected=0;return 0;}
 }else if(pb!=1||slot==4)return 0;
 unsigned used=fragments[slot].used,expected=fragments[slot].expected;
 if(n>expected-used){fragments[slot].expected=0;monitor_invalid++;return 0;}
 for(unsigned i=0;i<n;i++)fragments[slot].data[used+i]=p[i];used+=n;fragments[slot].used=used;
 if(used!=expected)return 0;
 unsigned bytes=media_bytes(fragments[slot].data,used,codec);fragments[slot].expected=0;
 if(bytes&&!strcmp(codec,"LHDC V5")){
  const unsigned char *rtp=fragments[slot].data;unsigned end=used;
  if(rtp[0]&32)end-=rtp[used-1];
  v5_bitrate_packet(rtp,rtp+end-bytes,bytes,monitor_last);
 }
 if(bytes)monitor_packets++;return bytes;
}
static int realtime_supported(const char *codec){return !strcmp(codec,"LDAC")||!strcmp(codec,"SBC")||!strcmp(codec,"AAC")||!strcmp(codec,"LHDC V5");}
static void monitor_poll(unsigned t,int yes,const char *codec){
 if(!yes||!realtime_supported(codec)){monitor_close();return;}
 if(monitorfd<0){
  if(t-monitor_retry<1000)return;monitor_retry=t;
  int fd=socket(31,3,1);if(fd<0)return;
  const unsigned short address[3]={31,0xffff,2};
  if(bind(fd,address,6)||fcntl(fd,2,1)<0){close(fd);return;}
  monitorfd=fd;monitor_opened=t;
 }
 if(t-monitor_last<16)return;monitor_last=t;
 unsigned bytes=0;unsigned char packet[4106];int i;
 for(i=0;i<64;i++){int n=recv(monitorfd,packet,sizeof(packet),0x40);if(n<0)break;if(n<6)continue;bytes+=monitor_packet(packet,(unsigned)n,codec);}
 if(i==64)monitor_capped++;
 if(bytes){unsigned slot=arrival_write++%96;arrivals[slot].at=t;arrivals[slot].bytes=bytes;}
 unsigned total=0;for(unsigned j=0;j<96;j++)if(arrivals[j].bytes&&t-arrivals[j].at<1000)total+=arrivals[j].bytes;
 unsigned interval=t-monitor_opened;if(interval>1000)interval=1000;
 realtime_kbps=interval>=250?(total*8+interval/2)/interval:0;
}
