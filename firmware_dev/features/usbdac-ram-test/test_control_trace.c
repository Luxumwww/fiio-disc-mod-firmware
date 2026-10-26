#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#define DISC_RAM_TEST
#include "replay.c"
int cacheflush(void *p,size_t n,int f){(void)p;(void)n;(void)f;return 0;}
static int expect_fd,result,err;static unsigned expect_request,calls;static void *expect_arg;
static int forward(int fd,unsigned req,void *arg){
 assert(fd==expect_fd&&req==expect_request&&arg==expect_arg);calls++;errno=err;return result;
}
int main(void){
 assert(mmap((void *)0x800000,0x200000,3,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)==(void *)0x800000);
 W(0x83a7d0)=10;W(0x83a7cc)=11;W(0x83dfc0)=0x900100;W(0x900100)=4;
 unsigned p=0x900200,sink=0x900500;W(0x83dfc4)=p;W(0x83dfe4)=sink;W(sink+0x58)=6;
 B(0x83a75a)=6;W(0x839a74)=1;W(p+0xc)=W(p+0x1c)=352800;
 W(p+0x10)=W(p+0x20)=2;W(p+0x14)=W(p+0x24)=32;W(p+0x2c)=2;W(p+0x58)=1;
 B(0x899060)=1;W(0x899084)=352800;native_ioctl=forward;enabled=1;
 expect_fd=10;expect_request=0x20004d28;expect_arg=(void *)0x123;result=1;err=EBUSY;
 assert(observed_ioctl(expect_fd,expect_request,expect_arg)==1&&errno==EBUSY&&!control_count);
 assert(control_start()&&!control_start());
 assert(observed_ioctl(expect_fd,expect_request,expect_arg)==1&&errno==EBUSY);
 assert(control_count==1&&control_events[0].fd==10&&control_events[0].dac==1&&control_events[0].result==1&&control_events[0].request==expect_request);
 assert(control_events[0].mode==4&&control_events[0].rate==352800&&control_events[0].dsd==2);
 expect_fd=11;expect_request=0x20004d2e;result=-1;err=EINVAL;
 assert(observed_ioctl(expect_fd,expect_request,expect_arg)==-1&&errno==EINVAL&&control_count==2);
 assert(control_events[1].result==-1&&control_events[1].error==EINVAL&&control_events[1].dac==2);
 expect_fd=12;assert(observed_ioctl(expect_fd,expect_request,expect_arg)==-1&&control_count==2);
 expect_fd=10;for(unsigned i=0;i<300;i++)assert(observed_ioctl(expect_fd,expect_request,expect_arg)==-1);
 assert(control_count==302&&control_overflow==46&&!control_inflight);
 control_active=0;assert(observed_ioctl(expect_fd,expect_request,expect_arg)==-1&&control_count==302);
 enabled=0;assert(!control_start());enabled=1;phase=REPLAY;assert(!control_start());
 phase=IDLE;W(0x900100)=2;assert(!control_start()&&control_rejected==3);
 puts("PASS: DAC observer preserves fd/request/argument, positive success, failure and errno; excludes other fds and inactive scope, bounded overflow, no dropped calls");
}
