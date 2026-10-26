/* Development-only standard USB reset, not an OTG/5V control. */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
__attribute__((constructor))static void run(void){int f=open("/dev/bus/usb/001/005",O_RDWR);if(f<0){perror("camera node");return;}unsigned char data[18]={0};struct usbdevfs_ctrltransfer x={.bRequestType=0x80,.bRequest=6,.wValue=0x100,.wLength=18,.timeout=500,.data=data};int r=ioctl(f,USBDEVFS_CONTROL,&x);if(r!=18||data[8]!=0xdf||data[9]!=0x2b||data[10]!=1||data[11]!=1){puts("Camera VID/PID mismatch; reset skipped");close(f);return;}r=ioctl(f,USBDEVFS_RESET,0);printf("USB reset result=%d\n",r);if(r<0)perror("USB reset");close(f);}
