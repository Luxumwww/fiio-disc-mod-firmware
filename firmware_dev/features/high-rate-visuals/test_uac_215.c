#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
enum {SINK=0x10000000,PCM=0x20000000,ENTRY=6*0x54};
static unsigned root=SINK,sink[512],pcm[64];static int fd_open=1,closed,unlinked,expect_owner=1;
static pthread_mutex_t input_lock=PTHREAD_MUTEX_INITIALIZER;
static unsigned *word(unsigned a){if(a==0x83dfe4)return &root;if(a>=SINK&&a<SINK+sizeof sink)return sink+(a-SINK)/4;if(a>=PCM&&a<PCM+sizeof pcm)return pcm+(a-PCM)/4;assert(0);return 0;}
#define PC_WORD(a) (*word(a))
#define UAC_OWNER_LOCK(p) pthread_mutex_lock(&input_lock)
#define UAC_OWNER_UNLOCK(p) pthread_mutex_unlock(&input_lock)
static const char *target="/dev/snd/pcmC1D0c (deleted)";
static long test_link(const char *p,char *b,unsigned n){assert(!strcmp(p,"/proc/self/fd/25"));unsigned z=(unsigned)strlen(target);assert(z<=n);memcpy(b,target,z);return z;}
#define readlink test_link
#define UAC_TEST_DISPATCH
#include "../bluetooth-meter/uac_capture_guard.h"
static int close_pcm(void *p){assert((unsigned long)p==PCM&&fd_open);assert(pthread_mutex_trylock(&input_lock)!=0);assert(!sink[(ENTRY+0x308)/4]&&!sink[(ENTRY+0x314)/4]);fd_open=0;closed++;return 0;}
static int unlink_uac(const char *p){(void)p;if(expect_owner){assert(!fd_open&&!sink[(ENTRY+0x308)/4]&&!sink[(ENTRY+0x314)/4]);assert(pthread_mutex_trylock(&input_lock)!=0);}unlinked++;return -7;}
static void setup(void){memset(sink,0,sizeof sink);memset(pcm,0,sizeof pcm);sink[0x5c/4]=2;sink[(ENTRY+0x300)/4]=1;sink[(ENTRY+0x308)/4]=1;sink[(ENTRY+0x314)/4]=PCM;pcm[0]=25;pcm[1]=0x10000000;fd_open=1;target="/dev/snd/pcmC1D0c (deleted)";}
static atomic_int reader_entered,reader_release;
static void *reader(void *p){(void)p;pthread_mutex_lock(&input_lock);atomic_store(&reader_entered,1);while(!atomic_load(&reader_release))usleep(1000);pthread_mutex_unlock(&input_lock);return 0;}
static void *teardown(void *p){(void)p;assert(uac_unlink_dispatch(0x46dda8,"/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac2.a")==-7);return 0;}
int main(void){
 uac_pcm_close=close_pcm;uac_original_unlink=unlink_uac;setup();
 /* The real failure: the native worker has already exited but PCM survives.
  * Selected input is now another device; the fixed UAC bank must still close. */
 assert(uac_unlink_dispatch(0x46dda8,"/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac2.a")==-7);assert(closed==1&&!fd_open);
 assert(uac_unlink_dispatch(0x46dda8,"/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac2.a")==-7&&closed==1);
 setup();target="/dev/snd/pcmC1D0c";assert(uac_unlink_dispatch(0x46de48,"/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac1.a")==-7&&closed==2);
 setup();target="/dev/snd/pcmC0D0p";int n=unlinked;assert(uac_unlink_dispatch(0x46dda8,"/sys/kernel/config/usb_gadget/uac_demo/configs/c.1/uac2.a")==-1);assert(fd_open&&closed==2&&unlinked==n);
 expect_owner=0;assert(uac_unlink_dispatch(0x46dda0,"/tmp/unrelated")==-7&&fd_open);expect_owner=1;
 setup();pthread_t a,b;pthread_create(&a,0,reader,0);while(!atomic_load(&reader_entered))usleep(1000);pthread_create(&b,0,teardown,0);usleep(10000);assert(fd_open);atomic_store(&reader_release,1);pthread_join(a,0);pthread_join(b,0);assert(!fd_open&&closed==3);
 puts("PASS actual-owner regression: stopped worker with live PCM, changed selected input, UAC1/UAC2, idempotence, wrong descriptor rejection, in-flight read serialized through unlink");return 0;
}
