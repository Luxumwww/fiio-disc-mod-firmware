#define _GNU_SOURCE
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
static pthread_mutex_t native_mutex=PTHREAD_MUTEX_INITIALIZER;
static volatile unsigned capture_state,capture_running=1;
#define UAC_CAPTURE_STATE capture_state
#define UAC_CAPTURE_RUNNING capture_running
#define UAC_NATIVE_CAPTURE_MUTEX (&native_mutex)
#define main legacy_main
#include "test_uac_213.c"
#undef main
static atomic_int allow_worker;
static void *selected_worker(void *unused){
 (void)unused;while(!atomic_load(&allow_worker))sched_yield();
 pthread_mutex_lock(&native_mutex);capture_state=0;pthread_mutex_unlock(&native_mutex);return 0;
}
static void *closer(void *unused){(void)unused;uac_capture_close();return 0;}
int main(void){
 assert(!legacy_main());
 for(unsigned state=3;state<=4;state++){
  capture_state=state;capture_running=1;atomic_store(&allow_worker,0);pthread_t worker_thread,close_thread;int old=closes;
  assert(!pthread_create(&worker_thread,0,selected_worker,0));assert(!pthread_create(&close_thread,0,closer,0));
  while(!uac_tearing_down)sched_yield();usleep(10000);
  uac_capture_set_state(state==3?4:3);uac_capture_reopen(44100,2,32,0);assert(capture_state==state);
  atomic_store(&allow_worker,1);pthread_join(worker_thread,0);pthread_join(close_thread,0);assert(closes==old+1&&!uac_tearing_down);
 }
 capture_state=3;int old=closes;unsigned failed=uac_close_failures;
 assert(uac_unlink_dispatch(0x46dda8,"/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac2.a")==-1);assert(closes==old&&uac_close_failures==failed+1&&!uac_tearing_down);
 capture_state=0;uac_capture_set_state(4);assert(capture_state==4);capture_state=0;
 puts("PASS: wait for selected reopen/pause to finish before native close; no new state3/4 during teardown; timeout keeps UAC link intact");return 0;
}
