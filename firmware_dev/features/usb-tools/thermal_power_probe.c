#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
static int r(int f,unsigned char reg){unsigned char v=0;struct i2c_msg m[2]={{.addr=0x1a,.len=1,.buf=&reg},{.addr=0x1a,.flags=I2C_M_RD,.len=1,.buf=&v}};struct i2c_rdwr_ioctl_data x={.msgs=m,.nmsgs=2};int n=ioctl(f,I2C_RDWR,&x);printf("reg_%02x=%02x result=%d\n",reg,v,n);return n==2?v:-1;}
__attribute__((constructor))static void run(void){int f=open("/dev/i2c-2",O_RDWR|O_CLOEXEC);if(f>=0){r(f,1);r(f,2);r(f,5);r(f,8);r(f,9);r(f,9);close(f);}fflush(stdout);}
