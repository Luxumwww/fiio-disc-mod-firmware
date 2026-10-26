/* Control worker only. Native SOAP owns decoder changes and notifications.
 * Buttons enqueue; no network request executes on the audio/UI callback. */
extern void *malloc(unsigned);extern void free(void *);
extern int connect(int,const void *,unsigned),setsockopt(int,int,int,const void *,unsigned);
extern int send(int,const void *,unsigned,int),recv(int,void *,unsigned,int);
extern unsigned inet_addr(const char *);
#include "track_policy.h"
#include "eof_policy.h"
static unsigned dlna_track_lock[6] __attribute__((aligned(8)));
static unsigned dlna_action_lock[6] __attribute__((aligned(8)));
static DiscDlnaHistory dlna_history;
static unsigned dlna_track_pending,dlna_track_requests,dlna_track_done,dlna_track_unsupported;
static int dlna_track_result;static unsigned dlna_track_observed,dlna_uri_changed,dlna_eof_since,dlna_eof_notified;
static int dlna_set_uri_action(void *event){
 const char *uri=PC_FN(0x5e8f2c,const char *(*)(void *,const char *))(event,"CurrentURI");
 const char *meta=PC_FN(0x5e8f2c,const char *(*)(void *,const char *))(event,"CurrentURIMetaData");
 /* Stock dereferences meta after its own missing-argument path. Reject the
  * malformed request before entering stock, preserving its SOAP error. */
 if(!uri||!meta)return -1;
 int result=PC_FN(0x5e523c,int (*)(void *))(event);
 if(!result){
  dlna_uri_changed=pc_seconds();dlna_eof_since=0;
  /* Stock set_uri clears the backend's prefetch pointer but leaves the
   * AVTransport NextURI variables pointing at the old resource. */
  pthread_mutex_lock((void *)0x89ad7c);unsigned vars=PC_WORD(0x89ad94);
  if(vars&&!PC_WORD(0x89addc)){
   unsigned notifications=PC_WORD(0x83bd2c);if(notifications)PC_FN(0x5e1a4c,void (*)(unsigned))(notifications);
   PC_FN(0x5e1554,int (*)(unsigned,unsigned,const char *))(vars,1,"");
   PC_FN(0x5e1554,int (*)(unsigned,unsigned,const char *))(vars,2,"");
   if(notifications)PC_FN(0x5e1a5c,void (*)(unsigned))(notifications);
  }pthread_mutex_unlock((void *)0x89ad7c);
  pthread_mutex_lock(dlna_track_lock);if(!dlna_history_add(&dlna_history,uri,meta))dlna_history.count=dlna_history.index=0;pthread_mutex_unlock(dlna_track_lock);
 }
 return result;
}
static int dlna_set_uri(void *event){pthread_mutex_lock(dlna_action_lock);int result=dlna_set_uri_action(event);pthread_mutex_unlock(dlna_action_lock);return result;}
static int dlna_set_next(void *event){
 pthread_mutex_lock(dlna_action_lock);
 const char *uri=PC_FN(0x5e8f2c,const char *(*)(void *,const char *))(event,"NextURI");
 const char *meta=PC_FN(0x5e8f2c,const char *(*)(void *,const char *))(event,"NextURIMetaData");
 int result=uri&&meta?PC_FN(0x5e3d7c,int (*)(void *))(event):-1;
 pthread_mutex_unlock(dlna_action_lock);return result;
}
static int dlna_track_click(void *request){
 if(!request||!pc_enabled()||pc_workmode()!=5||PC_WORD(0x89addc))return 0;
 unsigned action=((unsigned *)request)[4];if(action!=1&&action!=2)return 0;
 pthread_mutex_lock(dlna_track_lock);dlna_track_requests++;if(!dlna_track_pending)dlna_track_pending=action;pthread_mutex_unlock(dlna_track_lock);return 1;
}
static int dlna_soap(const char *action,const char *args){
 unsigned port=PC_FN(0x5f25b0,unsigned (*)(void))();if(!port)return -1;
 const char *ip=PC_FN(0x5f261c,const char *(*)(void))();char host[64];if(!dlna_copy_text(host,sizeof(host),ip))return -1;
 unsigned address=inet_addr(host);if(address==0xffffffffu)return -1;
 unsigned cap=64000;char *packet=malloc(cap);if(!packet)return -2;
 char *body=packet+512;int bytes=snprintf(body,cap-512,"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:%s xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID>%s</u:%s></s:Body></s:Envelope>",action,args,action);
 if(bytes<0||(unsigned)bytes>=cap-512){free(packet);return -2;}
 char header[512];int n=snprintf(header,sizeof(header),"POST /upnp/control/rendertransport1 HTTP/1.1\r\nHost: %s:%u\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nSOAPACTION: \"urn:schemas-upnp-org:service:AVTransport:1#%s\"\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",host,port,action,bytes);
 if(n<0||n>=512){free(packet);return -2;}
 /* Move forward into the reserved header gap without overlapping copies. */
 unsigned start=512-(unsigned)n;for(int i=0;i<n;i++)packet[start+(unsigned)i]=header[i];
 /* Linux/MIPS swaps SOCK_STREAM=2 and SOCK_DGRAM=1. */
 int fd=socket(2,2,0),result=-3;if(fd<0){free(packet);return result;}
 long timeout[2]={0,500000};
 if(setsockopt(fd,65535,0x1005,timeout,sizeof(timeout))||setsockopt(fd,65535,0x1006,timeout,sizeof(timeout)))goto finish;
 struct {unsigned short family,port;unsigned address;unsigned char zero[8];} addr={2,(unsigned short)((port>>8)|(port<<8)),address,{0}};
 if(connect(fd,&addr,sizeof(addr)))goto finish;
 unsigned sent=0,total=(unsigned)n+(unsigned)bytes;
 while(sent<total){int k=send(fd,packet+start+sent,total-sent,0x4000);if(k<=0)goto finish;sent+=(unsigned)k;}
 char response[64];unsigned got=0;while(got<12){int k=recv(fd,response+got,sizeof(response)-got,0);if(k<=0)goto finish;got+=(unsigned)k;}
 result=(!memcmp(response,"HTTP/1.",7)&&response[9]=='2'&&response[10]=='0'&&response[11]=='0')?0:-4;
finish:close(fd);free(packet);return result;
}
static int dlna_track_switch(const DiscDlnaTrack *track){
 char *args=malloc(62000);if(!args)return -2;unsigned n=0;
 const char *open="<CurrentURI>";while(*open)args[n++]=*open++;
 int k=dlna_xml_escape(args+n,62000-n,track->uri);if(k<0){free(args);return -2;}n+=(unsigned)k;
 open="</CurrentURI><CurrentURIMetaData>";while(*open)args[n++]=*open++;
 k=dlna_xml_escape(args+n,62000-n,track->meta);if(k<0){free(args);return -2;}n+=(unsigned)k;
 k=snprintf(args+n,62000-n,"</CurrentURIMetaData>");if(k<0||(unsigned)k>=62000-n){free(args);return -2;}
 int result=dlna_soap("SetAVTransportURI",args);free(args);
 if(!result&&pc_workmode()==5)result=dlna_soap("Play","<Speed>1</Speed>");return result;
}
static void dlna_eof_poll(unsigned now){
 if(pthread_mutex_trylock(dlna_action_lock))return;
 unsigned player=PC_WORD(0x83dfc4),dlna=PC_WORD(0x83dfcc),decoder=PC_WORD(0x85f390);
 int candidate=player&&dlna&&decoder&&!PC_WORD(0x89addc)&&PC_WORD(0x89acf4)==0x5e3f2c&&
  dlna_eof_candidate(PC_WORD(player+0x48),PC_WORD(0x89ad98),PC_WORD(dlna+0x10),PC_WORD(decoder+0x2c),dlna_uri_changed,now);
 if(candidate){
  pthread_mutex_lock((void *)0x89ad7c);unsigned vars=PC_WORD(0x89ad94);
  const char *next=vars?PC_FN(0x5e14fc,const char *(*)(unsigned,unsigned,int))(vars,1,0):0;
  candidate=next&&!next[0];pthread_mutex_unlock((void *)0x89ad7c);
 }
 if(!candidate)dlna_eof_since=0;
 else if(!dlna_eof_since)dlna_eof_since=now;
 else if(now-dlna_eof_since>=2){
  /* The stock adapter saves this callback on Play but never forwards the
   * native end state to it. Use its original event/notification path. */
  PC_FN(0x5e3f2c,void (*)(int))(0);dlna_eof_since=0;dlna_eof_notified++;
 }
 pthread_mutex_unlock(dlna_action_lock);
}
static void dlna_transport_poll(void){
 if(pthread_mutex_trylock(dlna_stop_lock))return;
 pthread_mutex_lock(dlna_track_lock);unsigned action=dlna_track_pending;dlna_track_pending=0;
 if(!pc_enabled()||pc_workmode()!=5){dlna_history.count=dlna_history.index=0;dlna_eof_since=0;pthread_mutex_unlock(dlna_track_lock);pthread_mutex_unlock(dlna_stop_lock);return;}
 pthread_mutex_unlock(dlna_track_lock);
 DiscDlnaTrack track;unsigned now=pc_seconds();
 if(action||now!=dlna_track_observed){
  dlna_track_observed=now;int valid=0;
  pthread_mutex_lock((void *)0x89ad7c);unsigned vars=PC_WORD(0x89ad94);
  if(vars&&PC_WORD(0x89aebc)==1&&!PC_WORD(0x89ae64)){
   const char *uri=PC_FN(0x5e14fc,const char *(*)(unsigned,unsigned,int))(vars,0x12,0);
   const char *meta=PC_FN(0x5e14fc,const char *(*)(unsigned,unsigned,int))(vars,0x17,0);
   valid=dlna_copy_text(track.uri,sizeof(track.uri),uri)&&dlna_copy_text(track.meta,sizeof(track.meta),meta);
  }pthread_mutex_unlock((void *)0x89ad7c);
  if(valid&&track.uri[0]){pthread_mutex_lock(dlna_track_lock);dlna_history_add(&dlna_history,track.uri,track.meta);pthread_mutex_unlock(dlna_track_lock);}
  if(!action)dlna_eof_poll(now);
 }
 if(!action){pthread_mutex_unlock(dlna_stop_lock);return;}
 pthread_mutex_lock(dlna_track_lock);int found=dlna_history_target(&dlna_history,action,&track);pthread_mutex_unlock(dlna_track_lock);
 if(!found&&action==1){
  /* Copy while the renderer's mutex protects its variable container. */
  pthread_mutex_lock((void *)0x89ad7c);unsigned vars=PC_WORD(0x89ad94);
  if(vars&&PC_WORD(0x89aebc)==1&&!PC_WORD(0x89ae64)){
   const char *next=PC_FN(0x5e14fc,const char *(*)(unsigned,unsigned,int))(vars,1,0);
   const char *meta=PC_FN(0x5e14fc,const char *(*)(unsigned,unsigned,int))(vars,2,0);
   const char *current=PC_FN(0x5e14fc,const char *(*)(unsigned,unsigned,int))(vars,0x12,0);
   found=next&&next[0]&&current&&strcmp(next,current)&&dlna_copy_text(track.uri,sizeof(track.uri),next)&&dlna_copy_text(track.meta,sizeof(track.meta),meta);
  }pthread_mutex_unlock((void *)0x89ad7c);
 }
 if(found&&pc_workmode()==5){dlna_track_result=dlna_track_switch(&track);if(!dlna_track_result)dlna_track_done++;}
 else{dlna_track_result=-5;dlna_track_unsupported++;}
 char text[160];int n=snprintf(text,sizeof(text),"requests=%u\ncompleted=%u\nunsupported=%u\nresult=%d\n",dlna_track_requests,dlna_track_done,dlna_track_unsupported,dlna_track_result);
 int fd=open("/tmp/disc-runtime/dlna-transport.state",1|0x100|0x200,0600);if(fd>=0){write(fd,text,n);close(fd);}
 pthread_mutex_unlock(dlna_stop_lock);
}
static int dlna_transport_install(void){
 if(PC_WORD(0x83bd80)!=0x5e523c||PC_WORD(0x83bd90)!=0x5e3d7c)return 0;
 PC_WORD(0x83bd80)=(unsigned)(unsigned long)dlna_set_uri;PC_WORD(0x83bd90)=(unsigned)(unsigned long)dlna_set_next;return 1;
}
