/* Explicit, bounded final pcm_write observation. Callback does RAM copies only. */
extern void *memcpy(void *,const void *,size_t);
struct CaptureEvent {
 unsigned begin_s,begin_ns,copy_s,copy_ns,end_s,end_ns;
 unsigned bytes,offset,copied,prepared_before,underruns_before,prepared_after,underruns_after;
 int result;
};
static struct CaptureEvent capture_events[512];
static unsigned char *capture_data;
static volatile unsigned capture_active,capture_inflight,capture_pending;
static unsigned capture_bytes,capture_count,capture_rate,capture_deadline,capture_error,capture_saved;
static int capture_reset(unsigned rate){
 if(capture_active||capture_inflight||capture_pending||phase==REPLAY||phase==BASELINE||phase==WAIT_DSD)return 0;
 if(capture_data){free(capture_data);capture_data=0;}
 capture_data=malloc(4000000);capture_bytes=capture_count=capture_error=capture_saved=0;
 capture_rate=rate;capture_deadline=seconds()+3;
 if(!capture_data){capture_error=1;return 0;}
 __sync_synchronize();capture_active=1;return 1;
}
static int capture_begin(void *pcm,const void *data,unsigned bytes){
 if(!capture_active)return -1;
 unsigned p=W(0x83dfc4),sink=W(0x83dfe4);
 if(!p||!sink||!scope(capture_rate)||pcm!=current_pcm(sink)||!data||(bytes&7)||!bytes){
  capture_error=2;capture_active=0;capture_pending=1;return -1;
 }
 if(seconds()>=capture_deadline||capture_count>=512){capture_active=0;capture_pending=1;return -1;}
 /* Native writes hold the existing output mutex, serializing this producer. */
 __sync_fetch_and_add(&capture_inflight,1);
 unsigned index=capture_count++,n=bytes;if(n>4000000-capture_bytes)n=4000000-capture_bytes;
 struct CaptureEvent *e=&capture_events[index];long t[2];clock_gettime(1,t);
 e->begin_s=t[0];e->begin_ns=t[1];e->bytes=bytes;e->offset=capture_bytes;e->copied=n;
 e->prepared_before=W((unsigned long)pcm+8);e->underruns_before=W((unsigned long)pcm+12);
 memcpy(capture_data+capture_bytes,data,n);capture_bytes+=n;clock_gettime(1,t);
 e->copy_s=t[0];e->copy_ns=t[1];e->end_s=e->end_ns=0;
 if(capture_bytes==4000000){capture_active=0;capture_pending=1;}
 return (int)index;
}
static void capture_end(int index,void *pcm,int result){
 if(index<0)return;struct CaptureEvent *e=&capture_events[index];long t[2];clock_gettime(1,t);
 e->end_s=t[0];e->end_ns=t[1];e->result=result;
 e->prepared_after=W((unsigned long)pcm+8);e->underruns_after=W((unsigned long)pcm+12);
 __sync_synchronize();__sync_fetch_and_sub(&capture_inflight,1);
}
static int capture_save(const char *path,const void *data,unsigned n){
 int fd=open(path,1|0x100|0x200,0600);if(fd<0)return 0;unsigned done=0;
 while(done<n){int k=write(fd,(const unsigned char *)data+done,n-done);if(k<=0){close(fd);return 0;}done+=(unsigned)k;}
 close(fd);return 1;
}
static void capture_flush(void){
 if(capture_active||capture_inflight||!capture_pending)return;__sync_synchronize();
 int ok=capture_save("/tmp/disc-runtime/dop-final-output.bin",capture_data,capture_bytes);
 ok&=capture_save("/tmp/disc-runtime/dop-final-events.bin",capture_events,capture_count*sizeof(capture_events[0]));
 char text[512];int n=snprintf(text,sizeof text,"version=1.0.3\npid=%d\nrate=%u\nbytes=%u\nevents=%u\nevent_bytes=%u\nerror=%u\ndata_saved=%d\npoint=before_native_pcm_write\nDAC_writes=0\nPCM_reconfigure=0\n",getpid(),capture_rate,capture_bytes,capture_count,(unsigned)sizeof(capture_events[0]),capture_error,ok);
 capture_save("/tmp/disc-runtime/dop-final-capture.state",text,n);
 free(capture_data);capture_data=0;capture_saved=ok;__sync_synchronize();capture_pending=0;
}
static void capture_poll(void){
 if(capture_active&&(!enabled||seconds()>=capture_deadline)){
  if(!enabled)capture_error=3;capture_active=0;capture_pending=1;
 }
 if(enabled&&!capture_active&&!capture_inflight&&!capture_pending&&!access("/tmp/disc-runtime/dop-final-capture.request",0)){
  unlink("/tmp/disc-runtime/dop-final-capture.request");unsigned p=W(0x83dfc4),rate=p?W(p+12):0;
  if((rate==176400||rate==352800)&&scope(rate)&&phase!=REPLAY&&phase!=BASELINE&&phase!=WAIT_DSD)capture_reset(rate);
 }
}
