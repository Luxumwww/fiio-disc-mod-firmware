#include <assert.h>
#include <stdio.h>
#include <string.h>
static int worker,pcm_fd,closes,opens,step,close_step,open_step,expected_stop;
static void native_close(void){
 close_step=++step;closes++;
 /* Native 4614bc requests exit and joins; its capture callback closes PCM. */
 worker=0;pcm_fd=0;
}
#define UAC_CLOSE_CAPTURE() native_close()
#define UAC_TEST_DISPATCH
#include "../bluetooth-meter/uac_capture_guard.h"
static void *native_open(const char *path,const char *mode){
 (void)path;(void)mode;open_step=++step;opens++;
 if(expected_stop){assert(!worker&&!pcm_fd);assert(close_step<open_step);}
 return (void *)&opens;
}
int main(void){
 const char *udc="/sys/kernel/config/usb_gadget/uac_demo/UDC";
 uac_original_fopen=native_open;
 const unsigned sites[]={0x46dd8c,0x46de2c};
 for(unsigned cycle=0;cycle<10;cycle++)for(unsigned i=0;i<2;i++){
  worker=pcm_fd=expected_stop=1;
  assert(uac_open_dispatch(sites[i],udc,"w")==&opens);
  assert(!worker&&!pcm_fd&&uac_ordered_closes==(unsigned)closes);
 }
 /* A repeated stop is idempotent through the stock capture closer. */
 uac_open_dispatch(sites[0],udc,"w");assert(!worker&&!pcm_fd&&closes==21);
 expected_stop=0;worker=pcm_fd=1;
 assert(uac_open_dispatch(0x46dd90,udc,"w")==&opens);
 uac_open_dispatch(sites[0],udc,"r");uac_open_dispatch(sites[1],udc,"wb");
 uac_open_dispatch(sites[0],"/sys/kernel/config/usb_gadget/disc_adb/UDC","w");
 uac_open_dispatch(sites[0],"/tmp/user-file","w");
 uac_open_dispatch(sites[0],0,"w");uac_open_dispatch(sites[0],udc,0);
 assert(worker&&pcm_fd&&closes==21&&opens==28);
 puts("PASS production UAC guard: close/join before configfs open, both native UAC stop sites, repeated stop, unrelated caller/path/mode pass-through");
 fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
