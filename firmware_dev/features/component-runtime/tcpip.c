/* Device-side tcpip callback bridge for the pinned DISC stock adbd.
 * Replaces only a writable snprintf import, never daemon instructions.
 * The existing native tcpip service writes/closes its response normally.
 * Runtime consumes a RAM request after the response has had time to drain. */
#include "syscalls.h"
#include "tcpip-guard.h"
typedef unsigned size_t;
typedef __builtin_va_list va_list;
extern int vsnprintf(char *,size_t,const char *,va_list);
extern int snprintf(char *,size_t,const char *,...);
extern int strncmp(const char *,const char *,size_t);
extern int rename(const char *,const char *);
extern int mkdir(const char *,unsigned);
extern int rmdir(const char *);
extern int unlink(const char *);
static int active;
static int tcpip_strncmp(const char *a,const char *b,size_t n) {
 /* Stock exit_on_close can terminate adbd before the reply drains.
  * Let runtime perform the bounded restart instead, only for this service. */
 if(active && (unsigned)b==TCPIP_NAME && n==6 &&
    (unsigned)__builtin_return_address(0)==TCPIP_EXIT_RETURN)return 1;
 return strncmp(a,b,n);
}
static const char *submit(int port) {
 if(disc_sys3(4033,(long)"/usr/data/disc-runtime/components/disc-adb-tcpip/enabled",0,0)<0 ||
    disc_sys3(4033,(long)"/usr/data/disc-runtime/components/disc-adb-tcpip/disabled",0,0)==0)
  return "error: DISC tcpip module is disabled\n";
 if(disc_sys3(4033,(long)"/tmp/sdcard/DISC_MODULES_SAFE_MODE",0,0)==0)
  return "error: DISC modules safe mode is active\n";
 if(port!=5555)return "error: DISC tcpip currently supports port 5555 only\n";
 if(disc_sys3(4033,(long)"/usr/data/disc-runtime/components/disc-adb-tcpip/current/tcpip-control.sh",0,0)<0 ||
    disc_sys3(4033,(long)"/usr/data/disc-runtime/service/runtime-modules.sh",0,0)<0)
  return "error: DISC tcpip runtime unavailable\n";
 if(disc_sys3(4033,(long)"/tmp/sdcard/DISC_DEBUG_USB_ENABLE",0,0)==0)
  return "error: TF USB rescue overrides TCP mode; remove rescue marker first\n";
 const char *lock="/tmp/disc-runtime/tcpip-submit.lock";
 if(mkdir(lock,0700))return "error: DISC tcpip request busy\n";
 const char *temp="/tmp/disc-runtime/tcpip.request.new";
 long fd=disc_sys3(4005,(long)temp,0x301,0600);
 int ok=fd>=0;
 if(ok){ok=disc_write_all(fd,"5555\n",5)==5;disc_sys1(4006,fd);}
 if(ok)ok=rename(temp,"/tmp/disc-runtime/tcpip.request")==0;
 if(!ok)unlink(temp);
 rmdir(lock);
 return ok ? (const char *)0 : "error: DISC tcpip request could not be saved\n";
}
static int tcpip_snprintf(char *buf,size_t size,const char *fmt,...) {
 va_list ap;__builtin_va_start(ap,fmt);
 int result;
 if(active && (unsigned)fmt==TCPIP_FORMAT &&
    (unsigned)__builtin_return_address(0)==TCPIP_RETURN) {
  int port=__builtin_va_arg(ap,int);
  const char *error=submit(port);
  result=error ? snprintf(buf,size,"%s",error) : snprintf(buf,size,"restarting in TCP mode port: %d\n",port);
 }else result=vsnprintf(buf,size,fmt,ap);
 __builtin_va_end(ap);return result;
}
__attribute__((constructor)) static void tcpip_init(void) {
 /* Entire executable fingerprint plus mapped call-site/format guards.
  * Host deployment separately checks SHA256; this is a version guard. */
 long fd=disc_open_read("/proc/self/exe");if(fd<0)return;
 unsigned hash=2166136261u,total=0;char block[1024];long n;
 while((n=disc_read(fd,block,sizeof(block)))>0){
  total+=(unsigned)n;
  for(long i=0;i<n;i++)hash=(hash^(unsigned char)block[i])*16777619u;
 }
 disc_close(fd);
 if(n<0 || total!=ADBD_SIZE || (hash!=ADBD_STOCK_FNV && hash!=ADBD_BOTH_FNV))return;
 const unsigned char *mapped=(const unsigned char *)TCPIP_GUARD_ADDRESS;
 for(unsigned i=0;i<sizeof(tcpip_guard);i++)if(mapped[i]!=tcpip_guard[i])return;
 mapped=(const unsigned char *)TCPIP_EXIT_GUARD_ADDRESS;
 for(unsigned i=0;i<sizeof(tcpip_exit_guard);i++)if(mapped[i]!=tcpip_exit_guard[i])return;
 active=1;
 *(volatile unsigned *)SNPRINTF_GOT=(unsigned)tcpip_snprintf;
 *(volatile unsigned *)STRNCMP_GOT=(unsigned)tcpip_strncmp;
 disc_write("DISC tcpip module 1.1.0 loaded (5555; USB request preserved)\n");
}
