#ifndef DISC_REPLAYGAIN_TAGS_H
#define DISC_REPLAYGAIN_TAGS_H
/* Read existing tags only. No analysis, media writes or decoder allocation.
 * ReplayGain keys/units: https://replaygain.hydrogenaudio.org/ and Xiph
 * VorbisComment. FLAC/Vorbis comments, ID3 TXXX, APEv2 and MP4 freeform tags.
 * File reads are bounded independently of declared metadata/image sizes. */
#include <math.h>
extern float strtof(const char *,char **);
extern int pread64(int,void *,unsigned,long long);
extern long long lseek64(int,long long,int);
typedef struct {float track,album,track_peak,album_peak;unsigned valid;} DiscRgTags;
enum {RG_TRACK=1,RG_ALBUM=2,RG_TRACK_PEAK=4,RG_ALBUM_PEAK=8};
typedef struct {int fd;long long size;unsigned bytes,ops;int failed;} DiscRgReader;
static unsigned rg_le32(const unsigned char *b){return (unsigned)b[0]|(unsigned)b[1]<<8|(unsigned)b[2]<<16|(unsigned)b[3]<<24;}
static unsigned rg_be32(const unsigned char *b){return (unsigned)b[3]|(unsigned)b[2]<<8|(unsigned)b[1]<<16|(unsigned)b[0]<<24;}
static unsigned rg_sync32(const unsigned char *b){return (unsigned)b[0]<<21|(unsigned)b[1]<<14|(unsigned)b[2]<<7|b[3];}
static int rg_sync_valid(const unsigned char *b){return !((b[0]|b[1]|b[2]|b[3])&128);}
static int rg_read(DiscRgReader *r,long long off,void *out,unsigned n){
 if(r->failed||off<0||off>r->size||n>(unsigned long long)(r->size-off)||n>262144u-r->bytes||r->ops>=2048){r->failed=1;return 0;}
 r->ops++;r->bytes+=n;if(pread64(r->fd,out,n,off)!=(int)n){r->failed=1;return 0;}return 1;
}
static int rg_key(const char *a,const char *b){unsigned i=0;for(;a[i]&&b[i];i++){unsigned x=(unsigned char)a[i],y=(unsigned char)b[i];if(x>='a'&&x<='z')x-=32;if(y>='a'&&y<='z')y-=32;if(x!=y)return 0;}return !a[i]&&!b[i];}
static void rg_tag(DiscRgTags *t,const char *key,const char *value){
 unsigned bit=rg_key(key,"REPLAYGAIN_TRACK_GAIN")?RG_TRACK:rg_key(key,"REPLAYGAIN_ALBUM_GAIN")?RG_ALBUM:rg_key(key,"REPLAYGAIN_TRACK_PEAK")?RG_TRACK_PEAK:rg_key(key,"REPLAYGAIN_ALBUM_PEAK")?RG_ALBUM_PEAK:0;
 if(!bit)return;char *end;float v=strtof(value,&end);if(end==value||!isfinite(v))return;
 while(*end==' '||*end=='\t')end++;
 if(bit==RG_TRACK||bit==RG_ALBUM){if((end[0]=='d'||end[0]=='D')&&(end[1]=='b'||end[1]=='B'))end+=2;if(v< -64||v>64)return;}
 else if(v<0||v>32)return;
 while(*end==' '||*end=='\r'||*end=='\n'||*end=='\t')end++;if(*end)return;
 /* Contradictory duplicate values are rejected for that field. */
 float *p=bit==RG_TRACK?&t->track:bit==RG_ALBUM?&t->album:bit==RG_TRACK_PEAK?&t->track_peak:&t->album_peak;
 if((t->valid&bit)&&*p!=v){t->valid&=~bit;t->valid|=bit<<8;return;}if(t->valid&(bit<<8))return;
 *p=v;t->valid|=bit;
}
static void rg_comment(DiscRgTags *t,const unsigned char *text,unsigned n){
 char key[64],value[80];unsigned i=0;while(i<n&&text[i]!='='&&i<sizeof key-1){key[i]=(char)text[i];i++;}
 if(i>=n||text[i]!='='||n-i-1>=sizeof value)return;key[i]=0;i++;unsigned j=0;while(i<n)value[j++]=(char)text[i++];value[j]=0;rg_tag(t,key,value);
}
static int rg_comments(DiscRgReader *r,long long off,unsigned length,DiscRgTags *t){
 unsigned char b[4],text[144];long long end=off+length;
 if(length<8||!rg_read(r,off,b,4))return 0;unsigned vendor=rg_le32(b);off+=4;if(vendor>(unsigned long long)(end-off)||end-off-vendor<4)return 0;off+=vendor;
 if(!rg_read(r,off,b,4))return 0;unsigned count=rg_le32(b);off+=4;if(count>65536)return 0;
 for(unsigned i=0;i<count;i++){
  if(end-off<4||!rg_read(r,off,b,4))return 0;unsigned n=rg_le32(b);off+=4;if(n>(unsigned long long)(end-off))return 0;
  if(n&&n<=sizeof text){if(!rg_read(r,off,text,n))return 0;rg_comment(t,text,n);}off+=n;
 }return !r->failed;
}
static int rg_flac(DiscRgReader *r,long long off,DiscRgTags *t){
 unsigned char b[4];if(!rg_read(r,off,b,4)||memcmp(b,"fLaC",4))return 0;off+=4;
 for(unsigned i=0;i<256;i++){
  if(!rg_read(r,off,b,4))return 0;off+=4;unsigned n=(unsigned)b[1]<<16|(unsigned)b[2]<<8|b[3];
  if(n>(unsigned long long)(r->size-off))return 0;
  if((b[0]&127)==4)return rg_comments(r,off,n,t);
  off+=n;if(b[0]&128)break;
 }return 1;
}
static void rg_id3_text(DiscRgTags *t,unsigned char *p,unsigned n,unsigned unsync){
 if(n<3)return;if(unsync){unsigned out=0;for(unsigned i=0;i<n;i++){p[out++]=p[i];if(p[i]==255&&i+1<n&&p[i+1]==0)i++;}n=out;}
 unsigned enc=*p++,at=0;n--;char key[64],value[80];unsigned k=0,v=0;
 if(enc==0||enc==3){while(at<n&&p[at]&&k<sizeof key-1)key[k++]=(char)p[at++];if(at==n||p[at])return;at++;
  while(at<n&&p[at]&&v<sizeof value-1)value[v++]=(char)p[at++];if(at<n&&p[at])return;
 }else if(enc==1||enc==2){
  int le=0;if(enc==1){if(n<2)return;if(p[0]==255&&p[1]==254)le=1;else if(p[0]!=254||p[1]!=255)return;at=2;}
  int split=0;for(;at+1<n;at+=2){unsigned c=le?(unsigned)p[at]|(unsigned)p[at+1]<<8:(unsigned)p[at]<<8|p[at+1];
   if(!c){if(!split){split=1;continue;}break;}if(c==0xfeff)continue;if(c>127)return;
   if(!split){if(k>=sizeof key-1)return;key[k++]=(char)c;}else{if(v>=sizeof value-1)return;value[v++]=(char)c;}
  }if(!split)return;
 }else return;key[k]=value[v]=0;rg_tag(t,key,value);
}
static long long rg_id3(DiscRgReader *r,DiscRgTags *t){
 unsigned char h[10];if(!rg_read(r,0,h,10)||memcmp(h,"ID3",3))return 0;
 unsigned ver=h[3],flags=h[5];if(ver<2||ver>4||!rg_sync_valid(h+6)||(ver==2&&(flags&64)))return 0;
 long long off=10,end=10+(long long)rg_sync32(h+6);if(end>r->size)return 0;
 if(ver>=3&&(flags&64)){unsigned char x[4];if(!rg_read(r,off,x,4))return 0;unsigned n=ver==4?rg_sync32(x):rg_be32(x);if(ver==4&&(!rg_sync_valid(x)||n<4))return 0;off+=n+(ver==3?4:0);if(off>end)return 0;}
 for(unsigned i=0;i<2048&&off<end;i++){
  unsigned head=ver==2?6:10;if(end-off<head||!rg_read(r,off,h,head))break;if(!h[0])break;
  unsigned n=ver==2?(unsigned)h[3]<<16|(unsigned)h[4]<<8|h[5]:ver==3?rg_be32(h+4):rg_sync32(h+4);
  if(ver==4&&!rg_sync_valid(h+4))break;off+=head;if(n>(unsigned long long)(end-off))break;
  int text=ver==2?!memcmp(h,"TXX",3):!memcmp(h,"TXXX",4);
  unsigned format=ver>=3?h[9]:0,skip=ver==3?(format&32?1:0):(format&64?1:0)+(format&1?4:0);
  unsigned unsupported=ver==3?format&192:format&12;
  if(text&&!unsupported&&n>skip&&n-skip<=512){unsigned char b[512];if(!rg_read(r,off+skip,b,n-skip))break;rg_id3_text(t,b,n-skip,(flags&128)||(ver==4&&(format&2)));}
  off+=n;
 }return end+((ver==4&&(flags&16))?10:0);
}
static void rg_ape(DiscRgReader *r,DiscRgTags *t){
 unsigned char b[144];long long end=r->size;if(end<32)return;
 if(end>=128&&rg_read(r,end-128,b,3)&&!memcmp(b,"TAG",3))end-=128;
 if(end<32||!rg_read(r,end-32,b,32)||memcmp(b,"APETAGEX",8))return;
 unsigned version=rg_le32(b+8),size=rg_le32(b+12),count=rg_le32(b+16),flags=rg_le32(b+20);
 if(version!=2000||size<32||size>16777216u||size>(unsigned long long)end||count>4096||(flags&0x20000000u))return;
 long long off=end-size,limit=end-32;
 for(unsigned i=0;i<count;i++){
  if(limit-off<9||!rg_read(r,off,b,8))return;unsigned n=rg_le32(b),type=rg_le32(b+4);off+=8;
  unsigned keybytes=(unsigned)(limit-off);if(keybytes>64)keybytes=64;if(!rg_read(r,off,b,keybytes))return;
  unsigned k=0;while(k<keybytes&&b[k])k++;if(k==keybytes)return;char key[65];for(unsigned j=0;j<k;j++)key[j]=(char)b[j];key[k]=0;off+=k+1;
  if(n>(unsigned long long)(limit-off))return;if(!(type&6)&&n>0&&n<80){if(!rg_read(r,off,b,n))return;b[n]=0;rg_tag(t,key,(char *)b);}off+=n;
 }
}
static void rg_mp4(DiscRgReader *r,long long off,long long end,unsigned depth,DiscRgTags *t){
 if(depth>6)return;
 for(unsigned iter=0;iter<512&&end-off>=8;iter++){
  unsigned char h[16];if(!rg_read(r,off,h,8))return;unsigned head=8;unsigned long long n=rg_be32(h);
  if(n==1){if(!rg_read(r,off+8,h+8,8))return;n=(unsigned long long)rg_be32(h+8)<<32|rg_be32(h+12);head=16;}
  else if(!n)n=(unsigned long long)(end-off);if(n<head||n>(unsigned long long)(end-off))return;
  long long start=off+head,next=off+(long long)n;
  if(!memcmp(h+4,"moov",4)||!memcmp(h+4,"udta",4)||!memcmp(h+4,"ilst",4))rg_mp4(r,start,next,depth+1,t);
  else if(!memcmp(h+4,"meta",4)&&next-start>=4)rg_mp4(r,start+4,next,depth+1,t);
  else if(!memcmp(h+4,"----",4)){
   char name[64]={0},value[80]={0},mean[80]={0};long long a=start;
   for(unsigned j=0;j<16&&next-a>=8;j++){
    unsigned char x[8];if(!rg_read(r,a,x,8))return;unsigned z=rg_be32(x);if(z<12||z>(unsigned long long)(next-a))break;
    if(!memcmp(x+4,"name",4)&&z-12<sizeof name){if(!rg_read(r,a+12,name,z-12))return;name[z-12]=0;}
    else if(!memcmp(x+4,"mean",4)&&z-12<sizeof mean){if(!rg_read(r,a+12,mean,z-12))return;mean[z-12]=0;}
    else if(!memcmp(x+4,"data",4)&&z>=16&&z-16<sizeof value){unsigned char kind[4];if(!rg_read(r,a+8,kind,4))return;if(rg_be32(kind)==1){if(!rg_read(r,a+16,value,z-16))return;value[z-16]=0;}}
    a+=z;
   }
   if(rg_key(mean,"com.apple.iTunes")||rg_key(mean,"org.hydrogenaudio.replaygain"))rg_tag(t,name,value);
  }off=next;if(r->failed)return;
 }
}
static void rg_ogg(DiscRgReader *r,DiscRgTags *t){
 /* Only identification/comment packets from the first logical stream.
  * Large embedded artwork/comment packets fail closed at 32 KiB. */
 unsigned char packet[32768],h[27],lace[255];unsigned size=0,serial=0,sequence=0,started=0,packets=0;long long off=0;
 for(unsigned pages=0;pages<32&&off<r->size;pages++){
  if(!rg_read(r,off,h,27)||memcmp(h,"OggS",4)||h[4])return;unsigned s=rg_le32(h+14),seq=rg_le32(h+18),segments=h[26];
  if(!started){serial=s;sequence=seq;started=1;if(!(h[5]&2))return;}
  if(s!=serial||seq!=sequence++||((h[5]&1)!=0)!=(size!=0))return;
  if(!rg_read(r,off+27,lace,segments))return;off+=27+segments;
  for(unsigned i=0;i<segments;i++){
   unsigned n=lace[i];if(n>sizeof packet-size||!rg_read(r,off,packet+size,n))return;size+=n;off+=n;
   if(n<255){
    if(!packets){if(!((size>=7&&packet[0]==1&&!memcmp(packet+1,"vorbis",6))||(size>=8&&!memcmp(packet,"OpusHead",8))))return;}
    else if(packets==1){unsigned skip=size>=7&&packet[0]==3&&!memcmp(packet+1,"vorbis",6)?7:size>=8&&!memcmp(packet,"OpusTags",8)?8:0;if(!skip)return;
     /* Parse the bounded packet through the same little-endian layout. */
     unsigned p=skip;if(size-p<4)return;unsigned vendor=rg_le32(packet+p);p+=4;if(vendor>size-p||size-p-vendor<4)return;p+=vendor;unsigned count=rg_le32(packet+p);p+=4;if(count>65536)return;
     for(unsigned j=0;j<count;j++){if(size-p<4)return;unsigned z=rg_le32(packet+p);p+=4;if(z>size-p)return;rg_comment(t,packet+p,z);p+=z;}return;
    }packets++;size=0;
   }
  }
 }
}
static DiscRgTags disc_rg_tags(const char *path){
 DiscRgTags t={0};int fd=open(path,0);if(fd<0)return t;DiscRgReader r={fd,lseek64(fd,0,2),0,0,0};unsigned char h[12];
 if(r.size>=12&&rg_read(&r,0,h,12)){
  if(!memcmp(h,"fLaC",4))rg_flac(&r,0,&t);
  else if(!memcmp(h,"ID3",3)){long long next=rg_id3(&r,&t);if(next>0&&r.size-next>=4)rg_flac(&r,next,&t);}
  else if(!memcmp(h,"OggS",4))rg_ogg(&r,&t);
  else if(!memcmp(h+4,"ftyp",4))rg_mp4(&r,0,r.size,0,&t);
  if(!r.failed)rg_ape(&r,&t);
 }
 close(fd);if(r.failed){DiscRgTags empty={0};return empty;}t.valid&=15;return t;
}
#endif
