#include "stream_core.h"
#include "spectrum_view.h"
extern int socket(int,int,int),bind(int,const void *,unsigned),close(int),unlink(const char *),chmod(const char *,unsigned);
extern int fcntl(int,int,...);
extern long recv(int,void *,unsigned long,int);
extern int poll(void *,unsigned long,int),clock_gettime(int,void *);
extern int pthread_create(unsigned *,const void *,void *(*)(void *),void *),pthread_join(unsigned,void **);
extern int pthread_mutex_lock(void *),pthread_mutex_unlock(void *),pthread_mutex_trylock(void *);
static int worker_fd=-1;static unsigned thread;static volatile unsigned stopping;static int output_lock[6];
static DiscStream stream;static DiscView published;
static unsigned now_ms(void){long t[2];if(clock_gettime(1,t))return 0;return (unsigned)t[0]*1000+(unsigned)t[1]/1000000;}
static void publish(unsigned live){
 pthread_mutex_lock(output_lock);
 published.rate=stream.rate;published.format=stream.format;published.channels=stream.channels;published.dop=stream.dop;published.live=live;
 published.db[0]=live?stream.db[0]:-96;published.db[1]=live?stream.db[1]:-96;
 for(unsigned b=0;b<DISC_BANDS;b++)published.levels[b]=live?stream.levels[b]:0;
 pthread_mutex_unlock(output_lock);
}
static void *worker(void *unused){
 (void)unused;unsigned received=0,last_fft=0;struct {int fd;short events,revents;} pfd={worker_fd,1,0};
 while(!stopping){
  int ready=poll(&pfd,1,20);unsigned time=now_ms();
  if(ready>0&&(pfd.revents&1)){
   for(unsigned i=0;i<128;i++){
    DiscPcmPacket packet;long count=recv(worker_fd,&packet,sizeof(packet),0x40);
    if(count<0)break;
    if(disc_stream_packet(&stream,&packet,(unsigned)count))received=time;
   }
  }
  if(received&&time-received>300){disc_stream_reset(&stream);received=0;publish(0);}
  if(time-last_fft>=40){disc_stream_analyze(&stream);publish(received!=0);last_fft=time;}
 }
 return (void *)0;
}
int disc_audio_start(void){
 struct {unsigned short family;char path[108];} addr={1,DISC_PCM_SOCKET};
#ifdef __mips__
 worker_fd=socket(1,1,0);
#else
 worker_fd=socket(1,2,0);
#endif
 if(worker_fd<0)return 0;
 if(fcntl(worker_fd,2,1)<0){close(worker_fd);worker_fd=-1;return 0;}
 /* This socket belongs to the single enabled stock UI; /tmp is boot-temporary. */
 unlink(DISC_PCM_SOCKET);
 if(bind(worker_fd,&addr,sizeof(addr))){close(worker_fd);worker_fd=-1;return 0;}
 chmod(DISC_PCM_SOCKET,0600);stopping=0;
 if(pthread_create(&thread,(void *)0,worker,(void *)0)){close(worker_fd);worker_fd=-1;unlink(DISC_PCM_SOCKET);return 0;}
 return 1;
}
void disc_audio_snapshot(DiscView *view){
 if(pthread_mutex_trylock(output_lock))return;
 *view=published;pthread_mutex_unlock(output_lock);
}
void disc_audio_stop(void){if(worker_fd>=0){stopping=1;pthread_join(thread,(void *)0);close(worker_fd);worker_fd=-1;unlink(DISC_PCM_SOCKET);}}
