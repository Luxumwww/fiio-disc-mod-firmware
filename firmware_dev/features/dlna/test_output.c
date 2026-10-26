#include <assert.h>
#include <stdio.h>
static unsigned mode,input,output,native_calls,restored;static int enabled=1;
static unsigned char *byte(unsigned a){static unsigned char b[2];b[0]=input;b[1]=output;assert(a==0x83a759||a==0x83a75a);return b+(a==0x83a75a);}
#define PC_BYTE(a) (*byte(a))
static int pc_enabled(void){return enabled;}
static int pc_workmode(void){return mode;}
static int native_output(void *request){output=((unsigned *)request)[4];native_calls++;mode=0;return 0;}
static void restore(unsigned m){assert(m==5);mode=m;restored++;}
static void *resolve(unsigned a){if(a==0x4f1294)return native_output;assert(a==0x453674);return restore;}
#define PC_FN(a,t) ((t)resolve(a))
#include "output.h"
int main(void){unsigned request[5]={0,0,0,0,6};int result=-1;mode=5;input=12;output=6;
 assert(dlna_output_request(request,&result)&&result==0&&!native_calls&&mode==5);
 request[4]=4;assert(dlna_output_request(request,&result)&&result==0&&native_calls==1&&restored==1&&mode==5);
 /* Input8 can now observe mode5 and run the normal DLNA teardown. */
 mode=1;assert(!dlna_output_request(request,&result));mode=5;enabled=0;assert(!dlna_output_request(request,&result));assert(!dlna_output_request(0,&result));
 puts("PASS: redundant DLNA output preserves renderer; real output change restores mode for subsequent teardown; other modes untouched");}
