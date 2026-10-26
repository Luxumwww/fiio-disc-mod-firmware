/* Development-only readback of the APK SDK's measurement commands. */
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
static int fd;
static int ctl(unsigned type,unsigned req,unsigned selector,void *data,unsigned n){struct usbdevfs_ctrltransfer x={.bRequestType=type,.bRequest=req,.wValue=selector<<8,.wIndex=10<<8,.wLength=n,.timeout=600,.data=data};return ioctl(fd,USBDEVFS_CONTROL,&x);}
__attribute__((constructor))static void run(void){
 fd=open("/dev/bus/usb/001/002",O_RDWR);if(fd<0){perror("open");return;}
 for(unsigned cmd=1;cmd<=2;cmd++){unsigned char select[2]={3,cmd},size[4]={0},data[512]={0};int s=ctl(0x21,1,5,select,2);int l=s==2?ctl(0xa1,0x85,3,size,4):-1;unsigned n=size[0]|(unsigned)size[1]<<8;if(l==4)n|=(unsigned)size[2]<<16|(unsigned)size[3]<<24;int got=(l==2||l==4)&&n>0&&n<=512?ctl(0xa1,0x81,3,data,n):-1;printf("command=3,%u select=%d len_reply=%d length=%u received=%d data=",cmd,s,l,n,got);for(int i=0;i<got;i++)printf("%02x",data[i]);puts("");}close(fd);
}
