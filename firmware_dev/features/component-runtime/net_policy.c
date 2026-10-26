/* A per-adbd policy: TCP/5555 accepts only the explicitly selected computer.
 * Loaded only in our daemon; the constructor removes preload from child env. */
#include "syscalls.h"
extern int *__errno_location(void);
extern int unsetenv(const char *);
struct sa4 { unsigned short family,port; unsigned char ip[4],pad[8]; };
__attribute__((constructor)) static void policy_init(void) { unsetenv("LD_PRELOAD"); }
static int approved(const unsigned char ip[4]) {
 if(ip[0]==127) return 1;
 char data[40];
 long fd=disc_sys3(4005,(long)"/usr/data/disc-runtime/allowed-host-ipv4",0,0);
 if(fd<0) return 0;
 long n=disc_sys3(4003,fd,(long)data,39);disc_sys1(4006,fd);
 if(n<7 || n>16) return 0;
 int pos=0;
 for(int i=0;i<4;i++) {
  unsigned v=0,digits=0;
  while(pos<n && data[pos]>='0' && data[pos]<='9') {v=v*10+(unsigned)(data[pos++]-'0');digits++;if(digits>3)return 0;}
  if(!digits || v>255 || v!=ip[i])return 0;
  if(i<3 && (pos>=n || data[pos++]!='.')) return 0;
 }
 while(pos<n && (data[pos]=='\n' || data[pos]=='\r'))pos++;
 return pos==n;
}
int accept(int fd,void *addr,unsigned *length) {
 struct sa4 local;unsigned local_size=sizeof(local);
 long info=disc_sys3(4172,fd,(long)&local,(long)&local_size);
 int protected_port=info==0 && local.family==2 && local.port==0xb315;
 for(;;) {
  struct sa4 peer;unsigned size=sizeof(peer);
  long client=disc_sys3(4168,fd,(long)&peer,(long)&size);
  if(client<0) {*__errno_location()=(int)-client;return -1;}
  if(protected_port && (peer.family!=2 || !approved(peer.ip))) {disc_sys1(4006,client);continue;}
  if(addr && length) {
   unsigned copied=*length<size?*length:size;
   if(copied>sizeof(peer))copied=sizeof(peer);
   for(unsigned i=0;i<copied;i++)((unsigned char *)addr)[i]=((unsigned char *)&peer)[i];
   *length=size;
  }
  return (int)client;
 }
}
