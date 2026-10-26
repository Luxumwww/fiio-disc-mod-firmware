#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
static int worker,pcm_fd,closes,opens,reopens,expect_stop,slow_open;
static atomic_int entered,release_open;
static void native_close(void){worker=pcm_fd=0;closes++;}
#define UAC_CLOSE_CAPTURE() native_close()
#define UAC_TEST_DISPATCH
#include "../bluetooth-meter/uac_capture_guard.h"
static int native_open(int device,void *api){
 (void)device;(void)api;opens++;
 if(slow_open){atomic_store(&entered,1);while(!atomic_load(&release_open))sched_yield();}
 worker=pcm_fd=1;return 0;
}
static void native_reopen(unsigned a,unsigned b,unsigned c,unsigned d){(void)a;(void)b;(void)c;(void)d;reopens++;}
static int native_unlink(const char *path){
 (void)path;
 if(expect_stop){assert(!worker&&!pcm_fd);int old=opens,previous=reopens;assert(uac_capture_open(4,0)==-1);uac_capture_reopen(192000,2,32,0);assert(opens==old&&reopens==previous);}
 return -17;
}
static void *opener(void *unused){(void)unused;assert(!uac_capture_open(4,0));return 0;}
static void *stopper(void *unused){(void)unused;assert(uac_unlink_dispatch(0x46dda8,"/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac2.a")==-17);return 0;}
int main(void){
 uac_original_unlink=native_unlink;uac_original_capture_open=native_open;uac_original_capture_reopen=native_reopen;
 const char *paths[]={"/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac2.a","/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac1.a"};
 const unsigned callers[]={0x46dda8,0x46de48};
 for(unsigned i=0;i<2;i++){assert(!uac_capture_open(4,0));expect_stop=1;assert(uac_unlink_dispatch(callers[i],paths[i])==-17);assert(!worker&&!pcm_fd&&!uac_tearing_down);expect_stop=0;}
 /* Unrelated deletion must preserve the native return and capture state. */
 assert(!uac_capture_open(4,0));int before=closes;assert(uac_unlink_dispatch(0x46dda0,paths[0])==-17);assert(uac_unlink_dispatch(callers[0],"/tmp/unrelated")==-17);assert(closes==before&&worker&&pcm_fd);
 native_close();slow_open=1;expect_stop=1;pthread_t a,b;assert(!pthread_create(&a,0,opener,0));while(!atomic_load(&entered))sched_yield();assert(!pthread_create(&b,0,stopper,0));while(!uac_tearing_down)sched_yield();assert(uac_capture_open(4,0)==-1);uac_capture_reopen(48000,2,32,0);atomic_store(&release_open,1);pthread_join(a,0);pthread_join(b,0);assert(!worker&&!pcm_fd&&!uac_tearing_down);
 slow_open=expect_stop=0;assert(!uac_capture_open(4,0));uac_capture_reopen(48000,2,32,0);assert(worker&&pcm_fd&&reopens==1);native_close();
 puts("PASS: UAC1/UAC2 close before actual unlink; no reentry during deletion; in-flight open state published before join; return/pass-through preserved; subsequent capture works");return 0;
}
