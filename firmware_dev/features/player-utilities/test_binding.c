#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
extern int open(const char *,int,...),close(int),read(int,void *,unsigned),write(int,const void *,unsigned),rename(const char *,const char *),getpid(void);
extern void *dlsym(void *,const char *);
extern int pthread_mutex_lock(void *),pthread_mutex_unlock(void *),pthread_mutex_trylock(void *);
static int model_mode=1;static unsigned model_word,handle_counter=10,read_calls,tag_opens;
static const char *fixture;
static int pc_workmode(void){return model_mode;}
static int pc_enabled(void){return 1;}
static unsigned pc_seconds(void){return 10;}
static int dsp_is_dsd(void){return 0;}
static void *native(unsigned a){(void)a;return 0;}
#define PC_FN(a,t) ((t)native(a))
#define PC_WORD(a) model_word
static int tag_open(const char *path,int flags,...){tag_opens++;return open(strstr(path,"/tmp/sdcard/")?fixture:path,flags);}
#define open tag_open
#include "replaygain_player.h"
#undef open
static int codec_open(const char *p,const char *r,const void *f,unsigned rate){(void)p;(void)r;(void)f;(void)rate;return (int)handle_counter++;}
static int codec_read(int handle,void *data,unsigned count){(void)handle;assert(count>=4096);read_calls++;int *p=data;for(unsigned i=0;i<1024;i++)p[i]=100000000;return 4096;}
static int codec_close(int h){(void)h;return 0;}
int main(int argc,char **argv){
 assert(argc==2);fixture=argv[1];assert(!pthread_key_create(&rg_tls_key,0));rg_real_open=codec_open;rg_real_read=codec_read;rg_real_close=codec_close;rg_config=(DiscRgConfig){1,1,0};rg_active=1;
 int current=rg_open("/tmp/sdcard/current.flac",0,0,0);assert(rg_tag_reads==1&&tag_opens==1);
 unsigned shape[4]={192000,32,2,0};DiscRgWorker worker={0};worker.native=shape;worker.handle=-1;assert(!pthread_setspecific(rg_tls_key,&worker));int pcm[1024];
 unsigned before=tag_opens;rg_read_pcm(current,pcm,sizeof pcm);assert(pcm[0]>=50118600&&pcm[0]<=50118800&&pcm[0]==pcm[1]&&rg_last_source==1&&tag_opens==before);
 int preload=rg_open("/tmp/sdcard/next.flac",0,0,0);pthread_mutex_lock(rg_mutex);for(unsigned i=0;i<4;i++)if(rg_records[i].handle==preload){rg_records[i].tags.track=-20;rg_refresh_record(&rg_records[i]);}pthread_mutex_unlock(rg_mutex);
 before=tag_opens;rg_read_pcm(current,pcm,sizeof pcm);assert(pcm[0]>=50118600&&pcm[0]<=50118800&&tag_opens==before); /* preload never changes current */
 rg_read_pcm(preload,pcm,sizeof pcm);assert(pcm[0]>=9999998&&pcm[0]<=10000002&&rg_last_source==1);
 shape[3]=1;rg_read_pcm(preload,pcm,sizeof pcm);assert(pcm[0]==100000000);shape[3]=0;model_mode=2;rg_read_pcm(preload,pcm,sizeof pcm);assert(pcm[0]==100000000);model_mode=1;
 assert(!pthread_setspecific(rg_tls_key,0));rg_read_pcm(preload,pcm,sizeof pcm);assert(pcm[0]==100000000);pthread_setspecific(rg_tls_key,&worker);
 rg_active=0;for(unsigned i=0;i<9;i++)rg_read_pcm(preload,pcm,sizeof pcm);assert(pcm[0]==100000000); /* smoothly returns to unity */
 assert(rg_open("http://example.invalid/a.flac",0,0,0)>=0&&tag_opens==before); /* no remote tag IO */
 rg_close(current);rg_close(preload);for(unsigned i=0;i<4;i++)assert(!rg_records[i].occupied);assert(read_calls==15);
 puts("PASS actual binding: per-handle metadata/gapless preload, decoder TLS, DSD/other-mode/foreign-thread bypass, disable-to-unity ramp, read has zero added IO, remote excluded, close clears records");return 0;
}

