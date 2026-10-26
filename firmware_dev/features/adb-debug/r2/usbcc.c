/* Match V427 mq_player set_usb_device/host: /dev/usbcc_ioctl + _IO(0xef,3/2).
 * No register writes, no guessed AW35615 requests, no libc dependency. */
#include "syscalls.h"
#ifndef CC_PATH
#define CC_PATH "/dev/usbcc_ioctl"
#endif
#ifdef CC_HOST
#define CC_REQUEST 0x2000ef02L
#define CC_MESSAGE "USBCC host request succeeded\n"
#else
#define CC_REQUEST 0x2000ef03L
#define CC_MESSAGE "USBCC device request succeeded\n"
#endif
void _start(void) {
 long fd=disc_sys3(DISC_NR_OPEN,(long)CC_PATH,2,0);
 if(fd<0) {
  disc_write_all(2,"USBCC open failed\n",18);
  disc_sys1(DISC_NR_EXIT,2);for(;;){}
 }
 long rc=disc_sys3(DISC_NR_IOCTL,fd,CC_REQUEST,0);
 disc_sys1(DISC_NR_CLOSE,fd);
 if(rc<0) {
  disc_write_all(2,"USBCC ioctl failed\n",19);
  disc_sys1(DISC_NR_EXIT,3);for(;;){}
 }
 disc_write_all(1,CC_MESSAGE,sizeof(CC_MESSAGE)-1);
 disc_sys1(DISC_NR_EXIT,0);for(;;){}
}
