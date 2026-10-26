#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static unsigned char data[50000];static unsigned bytes,offset,loads,calls,ticks;
static int fake_open(const char *p,int f){(void)p;(void)f;offset=0;return 1;}
static int fake_read(int fd,void *p,unsigned n){assert(fd==1);if(n>bytes-offset)n=bytes-offset;memcpy(p,data+offset,n);offset+=n;return (int)n;}
static int fake_close(int f){assert(f==1);return 0;}
static int fake_access(const char *p,int m){(void)p;(void)m;return -1;}
static void *fake_dlopen(const char *p,int f){(void)p;assert(f==2);loads++;return (void *)1;}
static int install(unsigned abi){assert(abi==1);return ++calls>=2;}
static void tick(void){ticks++;}
static void *fake_dlsym(void *p,const char *n){assert(p==(void *)1);return !strcmp(n,"disc_dsd_backend_install")?(void *)install:(void *)tick;}
#define open fake_open
#define read fake_read
#define close fake_close
#define access fake_access
#define dlopen fake_dlopen
#define dlsym fake_dlsym
#include "binding.h"
int main(void){FILE *f=fopen("build/libdisc-usbdac-dsd-backend.so","rb");assert(f);bytes=fread(data,1,sizeof(data),f);fclose(f);
 dsb_host_install();assert(!dsb_host_installed&&calls==1&&loads==1);
 dsb_host_poll();assert(dsb_host_installed&&calls==2&&loads==1&&ticks==1);
 dsb_host_poll();assert(calls==2&&loads==1&&ticks==2);
 puts("PASS: early uninitialized native callback retries after initialization; one library load, later ticks without repeated hashing.");return 0;
}
