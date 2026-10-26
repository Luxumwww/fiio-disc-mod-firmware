#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
static int r(int f,unsigned char reg){unsigned char v=0;struct i2c_msg m[2]={{.addr=0x1a,.len=1,.buf=&reg},{.addr=0x1a,.flags=I2C_M_RD,.len=1,.buf=&v}};struct i2c_rdwr_ioctl_data x={.msgs=m,.nmsgs=2};return ioctl(f,I2C_RDWR,&x)==2?v:-1;}
__attribute__((constructor))static void run(void){int f=open("/dev/i2c-2",O_RDWR|O_CLOEXEC);if(f<0)return;int last[4]={-9,-9,-9,-9};for(int i=0;i<240;i++){int v[4]={r(f,1),r(f,5),r(f,8),r(f,9)};if(v[0]!=last[0]||v[1]!=last[1]||v[2]!=last[2]||v[3]!=last[3]||i%20==0){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);printf("%ld.%03ld config=%02x timer=%02x status=%02x fault=%02x\n",t.tv_sec,t.tv_nsec/1000000,v[0],v[1],v[2],v[3]);fflush(stdout);}for(int j=0;j<4;j++)last[j]=v[j];usleep(250000);}close(f);}
