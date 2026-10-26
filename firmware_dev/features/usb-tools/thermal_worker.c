/* UVC bulk MJPEG receiver. No vendor firmware/calibration commands.
 * Runs on demand in its own process, never in the LVGL or audio thread. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <dlfcn.h>
#include <linux/usbdevice_fs.h>
#include "thermal_frame.h"
#include "thermal_port_state.h"
/* SDK headers are newer than stock glibc; bind the stable pre-C23 ABI. */
extern unsigned long disc_strtoul(const char *,char **,int) __asm__("strtoul");
extern int disc_sscanf(const char *,const char *,...) __asm__("sscanf");
#define strtoul disc_strtoul
#define sscanf disc_sscanf

#define RAM "/tmp/disc-runtime/"
#define ROLE "/sys/class/usb_role/13500000.otg_new-role-switch/role"
#define TYPEC "/sys/bus/i2c/devices/2-0022/control/typec_state"
static unsigned owner,token,frames;
static int usb=-1,lockfd=-1,hosted,claimed[2],endpoint=0x81,stream_if=1,ctrl_if;
static unsigned payload=16384;
static ThermalFrame *shared;
static volatile sig_atomic_t stopped;
static char lease[96];
static int thermal_palette=-1;
static int thermal_measurement;
static int text(const char *path,char *b,unsigned cap){int fd=open(path,O_RDONLY|O_CLOEXEC);if(fd<0)return 0;int n=read(fd,b,cap-1);close(fd);if(n<0)n=0;b[n]=0;return n;}
static unsigned now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (unsigned)t.tv_sec;}
static int atomic_text(const char *path,const char *data){char tmp[256];snprintf(tmp,sizeof(tmp),"%s.new",path);int fd=open(tmp,O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0600);if(fd<0)return 0;size_t n=strlen(data);int ok=write(fd,data,n)==(ssize_t)n;close(fd);return ok&&!rename(tmp,path);}
static int has(const char *path,const char *needle){char b[1200];return text(path,b,sizeof(b))&&strstr(b,needle);}
static void state(const char *phase,const char *detail){char b[512];snprintf(b,sizeof(b),"version=0.1.20\nui_pid=%u\ntoken=%u\npid=%d\nphase=%s\ndetail=%s\nframes=%u\npalette=%d\nmeasurement=%d\nupdated=%u\n",owner,token,getpid(),phase,detail,frames,thermal_palette,thermal_measurement,now());atomic_text(RAM "thermal.state",b);}
static int alive(void){char b[96];return !stopped&&!kill((pid_t)owner,0)&&!access("/usr/data/disc-runtime/components/disc-usb-tools/enabled",F_OK)&&access("/usr/data/disc-runtime/components/disc-usb-tools/disabled",F_OK)&&text(RAM "toolbox.session",b,sizeof(b))&&!strcmp(b,lease)&&!has(RAM "state","usb_priority=active\n")&&!has(RAM "state","usb_priority=pending\n");}
static void stop_signal(int n){(void)n;stopped=1;}
static int write_role(const char *s){int fd=open(ROLE,O_WRONLY|O_CLOEXEC);if(fd<0)return 0;int ok=write(fd,s,strlen(s))==(ssize_t)strlen(s);close(fd);return ok;}
static int aw(int source){int fd=open("/dev/aw35615",O_RDWR|O_CLOEXEC);if(fd<0)return 0;int r=ioctl(fd,source?0x20004e27u:0x20004e26u,0);close(fd);return r>=0;}
static void close_camera(void){
 if(usb>=0){for(unsigned i=0;i<2;i++)if(claimed[i]){unsigned n=i?stream_if:ctrl_if;ioctl(usb,USBDEVFS_RELEASEINTERFACE,&n);}close(usb);usb=-1;}
 claimed[0]=claimed[1]=0;
}
static void cleanup(void){
 close_camera();
 if(hosted){aw(0);write_role("device");hosted=0;}
 /* Retain port ownership through normal native role cleanup. Supply is
  * managed by the stock Type-C driver; do not access the charger chip. */
 char b[120];if(text(RAM "usb-tool.request",b,sizeof(b))){unsigned p,c,w;if(sscanf(b,"%u %u %u",&p,&c,&w)==3&&p==owner&&c==token&&w==(unsigned)getpid())unlink(RAM "usb-tool.request");}
 if(shared){munmap(shared,sizeof(*shared));shared=0;}unlink(RAM "thermal.frame");
 if(lockfd>=0){flock(lockfd,LOCK_UN);close(lockfd);lockfd=-1;}
}
static unsigned u16(const unsigned char *p){return p[0]|(unsigned)p[1]<<8;}
static unsigned u32(const unsigned char *p){return u16(p)|u16(p+2)<<16;}
static void put32(unsigned char *p,unsigned v){for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));}
static int control(unsigned type,unsigned request,unsigned value,unsigned index,void *data,unsigned length){struct usbdevfs_ctrltransfer c={.bRequestType=type,.bRequest=request,.wValue=value,.wIndex=index,.wLength=length,.timeout=600,.data=data};return ioctl(usb,USBDEVFS_CONTROL,&c);}
static int find_camera(void){
 DIR *d=opendir("/sys/bus/usb/devices");if(!d)return 0;struct dirent *e;char path[280],v[64],p[64],bus[32],dev[32];int result=0;
 while((e=readdir(d))){if(e->d_name[0]=='.'||strchr(e->d_name,':'))continue;
  snprintf(path,sizeof(path),"/sys/bus/usb/devices/%s/idVendor",e->d_name);if(!text(path,v,sizeof(v))||strtoul(v,0,16)!=0x2bdf)continue;
  snprintf(path,sizeof(path),"/sys/bus/usb/devices/%s/idProduct",e->d_name);if(!text(path,p,sizeof(p))||strtoul(p,0,16)!=0x0101)continue;
  snprintf(path,sizeof(path),"/sys/bus/usb/devices/%s/busnum",e->d_name);if(!text(path,bus,sizeof(bus)))continue;
  snprintf(path,sizeof(path),"/sys/bus/usb/devices/%s/devnum",e->d_name);if(!text(path,dev,sizeof(dev)))continue;
  snprintf(path,sizeof(path),"/dev/bus/usb/%03u/%03u",(unsigned)strtoul(bus,0,10),(unsigned)strtoul(dev,0,10));usb=open(path,O_RDWR|O_CLOEXEC);result=usb>=0;break;
 }closedir(d);return result;
}
static int configure(void){
 unsigned char config[4096];int n=control(0x80,6,0x200,0,config,9);if(n<9)return 0;unsigned len=u16(config+2);if(len>sizeof(config)||len<9)return 0;
 n=control(0x80,6,0x200,0,config,len);if(n!=(int)len)return 0;
 unsigned iface=99,alt=0,sub=0,fmt=0,format=0,frame=0,interval=400000,max_frame=1024*1024;
 for(unsigned i=0;i+2<=len&&config[i]>=2;){unsigned l=config[i];if(i+l>len)return 0;unsigned char *p=config+i;
  if(p[1]==4&&l>=9){iface=p[2];alt=p[3];sub=p[5]==14?p[6]:0;if(sub==1)ctrl_if=(int)iface;}
  if(sub==2&&alt==0&&p[1]==36&&l>=4){if(p[2]==6)fmt=p[3];else if(p[2]==4||p[2]==16)fmt=0;
   if(p[2]==7&&fmt&&l>=26&&!format){format=fmt;frame=p[3];interval=u32(p+21);max_frame=u32(p+17);stream_if=(int)iface;}}
  if(sub==2&&iface==(unsigned)stream_if&&alt==0&&p[1]==5&&l>=7&&(p[2]&128)&&(p[3]&3)==2)endpoint=p[2];
  i+=l;
 }
 if(!format||!frame||endpoint!=0x81)return 0;
 for(unsigned i=0;i<2;i++){unsigned index=i?stream_if:ctrl_if;if(ioctl(usb,USBDEVFS_CLAIMINTERFACE,&index)<0)return 0;claimed[i]=1;}
 unsigned char probe[48]={0},sz[2]={0};int plen=control(0xa1,0x85,0x100,stream_if,sz,2)==2?(int)u16(sz):34;if(plen!=26&&plen!=34&&plen!=48)plen=34;
 probe[0]=1;probe[2]=format;probe[3]=frame;put32(probe+4,interval);put32(probe+18,max_frame);put32(probe+22,16384);
 if(control(0x21,1,0x100,stream_if,probe,plen)<0)return 0;
 if(control(0xa1,0x81,0x100,stream_if,probe,plen)<26)return 0;
 payload=u32(probe+22);if(payload<512||payload>65536)return 0;
 if(control(0x21,1,0x200,stream_if,probe,plen)<0)return 0;
 return 1;
}
/* JPEG-only stb decoder; stock FFmpeg on this firmware is audio-only. */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCALS
#define STBI_MAX_DIMENSIONS 2048
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "third_party/stb_image.h"
#pragma clang diagnostic pop
static int decoder_init(void){return 1;}
static int render_jpeg(const unsigned char *jpeg,unsigned length){
 if(length<4||length>2*1024*1024)return 0;
 int w=0,h=0,channels=0;if(!stbi_info_from_memory(jpeg,(int)length,&w,&h,&channels)||w<1||h<1||w>2048||h>2048)return 0;
 unsigned char *rgb=stbi_load_from_memory(jpeg,(int)length,&w,&h,&channels,3);if(!rgb)return 0;
 shared->sequence++;__sync_synchronize();
 for(unsigned y=0;y<THERMAL_H;y++){unsigned yy=y*(unsigned)h/THERMAL_H;
  for(unsigned x=0;x<THERMAL_W;x++){unsigned xx=x*(unsigned)w/THERMAL_W,k=(y*THERMAL_W+x)*4,src=(yy*(unsigned)w+xx)*3;
   shared->pixels[k]=rgb[src+2];shared->pixels[k+1]=rgb[src+1];shared->pixels[k+2]=rgb[src];shared->pixels[k+3]=255;
  }
 }stbi_image_free(rgb);shared->width=THERMAL_W;shared->height=THERMAL_H;shared->frames=++frames;__sync_synchronize();shared->sequence++;return 1;
}
#include "thermal_palette.h"
#include "thermal_measurement.h"
#include "thermal_center.h"
static void capture(void){
 unsigned char *buf=malloc(payload),*jpeg=malloc(2*1024*1024);if(!buf||!jpeg){state("error","内存不足");free(buf);free(jpeg);return;}
 unsigned length=0,fid=2,last_good=now(),last_state=0,measurement_at=0,measurement_tries=0,start_frames=frames,stream_started=now();int bad=0;
 while(alive()){
  center_command();
  measurement_command();
  /* Like the manufacturer's preview startup, wait for real video before
   * changing thermometry. Retry at most three times, two seconds apart. */
  if(thermal_measurement!=1&&(frames-start_frames>=2||now()-stream_started>=3)&&measurement_tries<3&&now()-measurement_at>=2){measurement_at=now();measurement_tries++;measurement_apply();}
  palette_command();
  if(thermal_palette<0&&now()-palette_checked>=3){palette_checked=now();unsigned char p[512];palette_read(p);}
  if(!has(RAM "state","usb_active=0\n")){state("error","USB 端口被其他功能接管");break;}
  struct usbdevfs_bulktransfer x={.ep=(unsigned)endpoint,.len=payload,.timeout=500,.data=buf};int n=ioctl(usb,USBDEVFS_BULK,&x);
  if(n<0){if(errno==ENODEV||errno==ESHUTDOWN)break;if(now()-last_good>(frames?8u:120u)){state("waiting","正在等待视频恢复");break;}continue;}
  if(n<2||buf[0]<2||buf[0]>(unsigned)n)continue;unsigned head=buf[0],flags=buf[1];
  if((flags&1)!=fid){if(!bad&&length&&render_jpeg(jpeg,length))last_good=now();length=0;bad=0;fid=flags&1;}
  if(flags&64)bad=1;
  if(!bad){unsigned take=(unsigned)n-head;if(length+take<=2*1024*1024){memcpy(jpeg+length,buf+head,take);length+=take;}else{bad=1;length=0;}}
  if(flags&2){if(!bad&&length&&render_jpeg(jpeg,length))last_good=now();length=0;bad=0;}
  if(now()!=last_state){last_state=now();state(frames?"streaming":"starting",frames?"已连接":"正在接收画面");}
  if(now()-last_good>(frames?8u:120u)){state("waiting","正在等待视频恢复");break;}
 }free(buf);free(jpeg);
}
__attribute__((constructor)) static void run(void){
 const char *o=getenv("DISC_THERMAL_OWNER"),*c=getenv("DISC_THERMAL_TOKEN");if(!o||!c)return;
 owner=(unsigned)strtoul(o,0,10);token=(unsigned)strtoul(c,0,10);if(!owner||!token)return;snprintf(lease,sizeof(lease),"%u %u\n",owner,token);
 const char *test=getenv("DISC_THERMAL_TEST_JPEG");
 if(test){shared=calloc(1,sizeof(*shared));unsigned char *b=malloc(2*1024*1024);int f=open(test,O_RDONLY);int n=f>=0?read(f,b,2*1024*1024):0;if(f>=0)close(f);int ok=shared&&b&&decoder_init()&&n>0&&render_jpeg(b,(unsigned)n);printf("THERMAL_DECODE_TEST %s %ux%u frames=%u\n",ok?"PASS":"FAIL",shared?shared->width:0,shared?shared->height:0,frames);fflush(stdout);free(b);free(shared);_exit(ok?0:1);}
 lockfd=open(RAM "thermal.lock",O_CREAT|O_RDWR|O_CLOEXEC,0600);if(lockfd<0||flock(lockfd,LOCK_EX|LOCK_NB)<0)_exit(2);
 signal(SIGTERM,stop_signal);signal(SIGINT,stop_signal);nice(8);state("preparing","正在准备 USB");
 if(!alive())goto done;
 /* Do not take the data role from a connected computer/charger. */
 {char b[96];if(!text(TYPEC,b,sizeof(b))||!strstr(b,"Unattached")){state("error","请断开电脑 USB，再连接热成像");goto done;}}
 if(!has(ROLE,"device")){state("error","USB 输出正在使用，请先关闭");goto done;}
 {char b[96];snprintf(b,sizeof(b),"%u %u %d\n",owner,token,getpid());if(!atomic_text(RAM "usb-tool.request",b)){state("error","USB 请求失败");goto done;}}
 {unsigned at=now();while(alive()&&!has(RAM "state","pause_reason=usb_tool\n")){if(now()-at>=12){state("error","USB 端口释放超时");goto done;}usleep(100000);}}
 {unsigned at=now();char b[1200];while(alive()){
   if(text(RAM "state",b,sizeof(b))&&thermal_port_state_ready(b)&&access("/dev/usb-ffs/adb/ep0",F_OK))break;
   if(now()-at>=35){state("error","USB 调试尚未释放端口");goto done;}usleep(100000);
  }}
 if(!alive())goto done;
 {char b[96];if(!text(TYPEC,b,sizeof(b))||!strstr(b,"Unattached")){state("error","检测到外部 USB 供电，请先拔下");goto done;}}
 if(!aw(1)){state("error","USB 主机切换失败");goto done;}hosted=1;
 if(!write_role("host")){state("error","USB 主机切换失败");goto done;}state("waiting","请连接热成像，等待设备启动");
 {int f=open(RAM "thermal.frame",O_CREAT|O_TRUNC|O_RDWR|O_CLOEXEC,0600);if(f<0||ftruncate(f,sizeof(ThermalFrame))){if(f>=0)close(f);state("error","无法建立画面缓冲");goto done;}shared=mmap(0,sizeof(*shared),PROT_READ|PROT_WRITE,MAP_SHARED,f,0);close(f);if(shared==MAP_FAILED){shared=0;goto done;}memset(shared,0,sizeof(*shared));shared->magic=THERMAL_MAGIC;shared->ui_pid=owner;shared->token=token;}
 /* Reconnect the video receiver while the page is open. Native Type-C
  * management alone decides when to supply power. */
 while(alive()){
  unsigned published=0;
  while(alive()&&!find_camera()){if(now()-published>=5){published=now();state("waiting","正在等待热成像启动");}usleep(250000);}
  if(!alive())break;
  if(!configure()){close_camera();state("waiting","设备正在启动，等待视频就绪");usleep(500000);continue;}
  thermal_protocol=0;thermal_palette=-1;
  {unsigned char data[512];if(palette_read(data)){char saved[24];if(text("/usr/data/disc-runtime/module-data/disc-usb-tools/palette.conf",saved,sizeof(saved))){unsigned preferred=(unsigned)strtoul(saved,0,10);if(palette_valid(preferred)&&preferred!=(unsigned)thermal_palette)palette_set(preferred);}}}
  thermal_measurement=0;
  state("starting","正在接收画面");capture();close_camera();
  if(alive()){state("waiting","等待热成像重新连接");usleep(500000);}
 }
done:
 cleanup();if(stopped||!alive())state("stopped","已关闭 USB 热成像");_exit(0);
}
