#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
static int cacheflush(void *p,int n,int flags){(void)p;(void)n;(void)flags;return 0;}
static long readlink(const char *p,char *out,unsigned long n){(void)p;(void)out;(void)n;return -1;}
static int unlink(const char *p){(void)p;return 0;}
static int pc_workmode(void){return 1;}
static void (*pc_cover_tick)(void);
static unsigned test_word;
static unsigned long mock_function(unsigned);
#define PC_FN(a,t) ((t)mock_function(a))
#define PC_WORD(a) test_word
static int enabled=1;
static int pc_enabled(void){return enabled;}
static int pthread_mutex_lock(void *p){(void)p;return 0;}
static int pthread_mutex_unlock(void *p){(void)p;return 0;}
/* Diagnostic files are intentionally discarded in this host-only test. */
static int test_open(const char *p,int flags,...){(void)p;(void)flags;return -1;}
static int test_read(int fd,void *b,unsigned n){(void)fd;(void)b;(void)n;return -1;}
static int test_write(int fd,const void *b,unsigned n){(void)fd;(void)b;return (int)n;}
static int test_close(int fd){(void)fd;return 0;}
static int test_rename(const char *a,const char *b){(void)a;(void)b;return 0;}
static int getpid(void){return 17;}
#define open test_open
#define read test_read
#define write test_write
#define close test_close
#define rename test_rename
#include "player_binding.h"
static void *key_value;
static int key_fail,worker_calls,frees,get_calls;
static unsigned get_limit,last_hint;
static void *(*scheduled_start)(void *);
static void *scheduled_arg;
static unsigned char png[32]={137,80,78,71,13,10,26,10,0,0,0,13,'I','H','D','R',0,0,5,160,0,0,5,160};
static int extract_size=3691554;
static void release_data(void **p){assert(p&&*p);free(*p);*p=0;frees++;}
static int get_image(const char *path,void **out,unsigned limit){
 assert(path);get_calls++;get_limit=limit;
 if(extract_size<0){*out=0;return extract_size;}
 if((unsigned)extract_size>limit){*out=0;return -2;}
 *out=calloc(1,(unsigned)extract_size);assert(*out);memcpy(*out,png,32);return extract_size;
}
static void *native_worker(void *arg){
 worker_calls++;last_hint=*(unsigned *)((char *)arg+0x400);
 void *data=0;int n=cover_get(arg,&data,DISC_COVER_STOCK_LIMIT);
 if(data)release_data(&data);
 free(arg);return (void *)(long)n;
}
static int native_sync(const char *path,const char *directory,const char *artist,unsigned hint){
 (void)directory;(void)artist;worker_calls++;last_hint=hint;
 if(!hint||hint>=DISC_COVER_STOCK_LIMIT)return 0;
 void *data=0;int n=cover_get(path,&data,DISC_COVER_STOCK_LIMIT);if(data)release_data(&data);return n;
}
static unsigned long mock_function(unsigned a){if(a==0x44ea10)return (unsigned long)native_worker;if(a==0x462440)return (unsigned long)native_sync;if(a==0x4ce8c0)return (unsigned long)release_data;abort();}
static void *get_key(unsigned k){assert(k==7);return key_value;}
static int set_key(unsigned k,const void *p){assert(k==7);if(key_fail&&p)return 1;key_value=(void *)p;return 0;}
static int create(void *thread,const void *attr,void *(*start)(void *),void *arg){assert(thread==(void *)17&&attr==(void *)19);scheduled_start=start;scheduled_arg=arg;return 23;}
static void *test_malloc(unsigned n){return malloc(n);}
static void *record(unsigned hint){char *p=calloc(1,0x408);assert(p);strcpy(p,"/tmp/sdcard/test.flac");*(unsigned *)(p+0x400)=hint;*(unsigned *)(p+0x404)=88;return p;}
static void *other_worker(void *p){return p;}
int main(int argc,char **argv){
 cover_key=7;cover_key_get=get_key;cover_key_set=set_key;cover_real_get=get_image;cover_real_create=create;
 cover_real_malloc=test_malloc;
 assert(cover_jpeg_dimensions_ok(8000,8000)&&!cover_jpeg_dimensions_ok(8001,1)&&!cover_jpeg_dimensions_ok(0,100));
 assert(cover_jpeg_scale(6000,6000)==8&&cover_jpeg_scale(8000,8000)==8&&cover_jpeg_scale(3000,3000)==8&&cover_jpeg_scale(1440,1440)==2);
 DiscCoverJob budget={0};budget.jpeg_active=1;budget.jpeg_heap=DISC_COVER_JPEG_HEAP_LIMIT-8;key_value=&budget;
 assert(!cover_malloc(9)&&budget.jpeg_heap_rejected==1&&budget.jpeg_heap==DISC_COVER_JPEG_HEAP_LIMIT-8);
 void *small=cover_malloc(8);assert(small&&budget.jpeg_heap==DISC_COVER_JPEG_HEAP_LIMIT);free(small);key_value=0;
 assert(cover_sync("test.flac",0,0,3691554)==3691554&&last_hint==1499999&&get_limit==8388608&&!key_value);
 assert(cover_sync("test.flac",0,0,8388609)==0&&last_hint==8388609&&!key_value);
 enabled=0;assert(cover_sync("test.flac",0,0,3691554)==0&&last_hint==3691554);enabled=1;
 key_fail=1;assert(cover_sync("test.flac",0,0,3691554)==0&&last_hint==3691554&&!key_value);key_fail=0;
 frees=0;
 assert(!cover_large_hint(1499999)&&cover_large_hint(1500000)&&cover_large_hint(8388608)&&!cover_large_hint(8388609));
 void *p=record(3691554);
 assert(cover_create((void *)17,(void *)19,native_worker,p)==23&&scheduled_start==cover_native_worker&&scheduled_arg==p);
 assert((long)scheduled_start(scheduled_arg)==3691554&&last_hint==1499999&&get_limit==8388608&&!key_value&&frees==1);
 p=record(1499999);cover_create((void *)17,(void *)19,native_worker,p);assert(scheduled_start==cover_native_worker);assert((long)scheduled_start(p)==-2&&last_hint==1499999&&get_limit==1500000);
 p=record(8388609);cover_create((void *)17,(void *)19,native_worker,p);assert(scheduled_start==cover_native_worker);assert((long)scheduled_start(p)==-2&&last_hint==8388609&&get_limit==1500000);
 p=record(3691554);enabled=0;cover_create((void *)17,(void *)19,native_worker,p);assert(scheduled_start==native_worker);free(p);enabled=1;
 p=record(3691554);cover_create((void *)17,(void *)19,other_worker,p);assert(scheduled_start==other_worker);free(p);
 p=record(3691554);key_fail=1;assert((long)cover_native_worker(p)==-2&&last_hint==3691554&&get_limit==1500000&&!key_value);key_fail=0;
 /* Oversized actual pixel dimensions are rejected/freed before native decode. */
 png[16]=127;p=record(3691554);int old=frees;assert((long)cover_native_worker(p)==-2&&frees==old+1&&!key_value);png[16]=0;
 extract_size=-35;p=record(3691554);assert((long)cover_native_worker(p)==-35&&!key_value);extract_size=3691554;
 unsigned w=0,h=0;assert(cover_large_image_ok(png,32,&w,&h)&&w==1440&&h==1440);
 assert(!cover_large_image_ok(png,23,&w,&h));png[20]=1;assert(!cover_large_image_ok(png,32,&w,&h));png[20]=0;
 unsigned char jpeg[32]={255,216,255,224,0,4,0,0,255,194,0,8,8,5,160,5,160,1};
 assert(cover_large_image_ok(jpeg,sizeof jpeg,&w,&h)&&w==1440&&h==1440);
 jpeg[10]=255;assert(!cover_large_image_ok(jpeg,sizeof jpeg,&w,&h));
 for(unsigned n=0;n<32;n++){cover_large_image_ok(png,n,&w,&h);cover_large_image_ok(jpeg,n,&w,&h);}
 /* The file is a private test fixture, never packaged with the component. */
 if(argc>1){FILE *f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);unsigned char *b=malloc((unsigned)n);assert(fread(b,1,(unsigned)n,f)==(unsigned)n);fclose(f);assert(cover_large_image_ok(b,(unsigned)n,&w,&h)&&w==1440&&h==1440);free(b);}
 printf("PASS: production worker/import hooks, ordinary/oversize/disabled/other-thread pass-through, TLS failure/cleanup, extraction errors/native frees, PNG/JPEG bounds, malformed lengths and exact actual cover (%d workers, %d getters)\n",worker_calls,get_calls);
}
