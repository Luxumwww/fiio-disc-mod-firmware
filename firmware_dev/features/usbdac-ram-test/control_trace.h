/* On-demand five-second observation of existing player DAC ioctls only.
 * Forward every call unchanged, including positive driver success results.
 * No device opens, I2C access, payload copies or file IO in the callback. */
extern int *__errno_location(void);
struct ControlEvent {
 unsigned begin_s,begin_ns,end_s,end_ns,request,caller;
 int fd,result,error;
 unsigned dac,mode,rate,dsd;
};
static struct ControlEvent control_events[256];
static volatile unsigned control_active,control_inflight,control_pending,control_count;
static unsigned control_deadline,control_overflow,control_saved,control_rejected;
static int observed_ioctl(int fd,unsigned request,void *arg){
 unsigned slot=256;
 if(control_active&&(fd==(int)W(0x83a7d0)||fd==(int)W(0x83a7cc))){
  __sync_fetch_and_add(&control_inflight,1);
  if(control_active){
   slot=__sync_fetch_and_add(&control_count,1);
   if(slot>=256){__sync_fetch_and_add(&control_overflow,1);slot=256;}
  }
  if(slot==256)__sync_fetch_and_sub(&control_inflight,1);
 }
 if(slot==256)return native_ioctl(fd,request,arg);
 struct ControlEvent *e=&control_events[slot];
 int prior=*__errno_location();long t[2]={0};clock_gettime(1,t);
 e->begin_s=t[0];e->begin_ns=t[1];e->fd=fd;e->request=request;
 e->caller=(unsigned)(unsigned long)__builtin_return_address(0);
 e->dac=(fd==(int)W(0x83a7d0)?1:0)|(fd==(int)W(0x83a7cc)?2:0);
 unsigned m=W(0x83dfc0),p=W(0x83dfc4);
 e->mode=m?W(m):0;e->rate=p?W(p+0xc):0;e->dsd=p?W(p+0x2c):0;
 *__errno_location()=prior;
 int rc=native_ioctl(fd,request,arg),error=*__errno_location();
 e->result=rc;e->error=error;t[0]=t[1]=0;clock_gettime(1,t);
 e->end_s=t[0];e->end_ns=t[1];
 __sync_synchronize();__sync_fetch_and_sub(&control_inflight,1);
 *__errno_location()=error;return rc;
}
static int control_start(void){
 if(control_active||control_pending||control_inflight)return 0;
 if(!enabled||phase==REPLAY||phase==BASELINE||phase==WAIT_DSD||
    (!scope(176400)&&!scope(352800))){control_rejected++;return 0;}
 control_count=control_overflow=control_saved=0;control_deadline=seconds()+5;
 __sync_synchronize();control_active=1;return 1;
}
static void control_poll(void){
 if(!access("/tmp/disc-runtime/dac-control-trace-start",0)){
  unlink("/tmp/disc-runtime/dac-control-trace-start");control_start();
 }
 if(control_active&&(!enabled||seconds()>=control_deadline)){
  control_active=0;__sync_synchronize();control_pending=1;
 }
}
static void control_flush(void){
 if(control_active||control_inflight||!control_pending)return;
 __sync_synchronize();unsigned n=control_count;if(n>256)n=256;
 int ok=capture_save("/tmp/disc-runtime/dac-control-events.bin",control_events,n*sizeof(control_events[0]));
 char b[512];int len=snprintf(b,sizeof b,"version=1.0.5\npid=%d\nevents=%u\nevent_bytes=%u\noverflow=%u\nsaved=%d\npoint=player_DAC_ioctl\nforward_unchanged=1\nnew_DAC_calls=0\n",getpid(),n,(unsigned)sizeof(control_events[0]),control_overflow,ok);
 ok&=capture_save("/tmp/disc-runtime/dac-control-trace.state",b,(unsigned)len);
 control_saved=ok;__sync_synchronize();control_pending=0;
}
