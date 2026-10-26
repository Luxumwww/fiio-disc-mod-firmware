#include <assert.h>
#include <stdio.h>
static unsigned thread_id,quit,stops,locked;
static unsigned *word(unsigned a){if(a==0x83e010)return &thread_id;assert(a==0x89ae64);return &quit;}
#define PC_WORD(a) (*word(a))
static void native_stop(void){assert(locked&&thread_id&&quit==0);quit=1;stops++;}
static void *resolve(unsigned a){assert(a==0x5e9eb8);return native_stop;}
#define PC_FN(a,t) ((t)resolve(a))
static int pthread_mutex_lock(void *m){(void)m;assert(!locked);locked=1;return 0;}
static int pthread_mutex_unlock(void *m){(void)m;assert(locked);locked=0;return 0;}
#define DISC_DLNA_LIFECYCLE_TEST
#include "lifecycle.h"
int main(void){
 (void)dlna_lifecycle_ready;
 for(unsigned q=0;q<2;q++){
  thread_id=123;quit=q;unsigned before=stops;
  dlna_stop_complete();assert(stops==before+1&&!thread_id&&quit==1&&!locked);
  dlna_stop_complete();assert(stops==before+1&&!locked);
 }
 thread_id=456;quit=0;dlna_stop_complete();assert(stops==3&&!thread_id);
 puts("PASS: signaled and unsignaled renderers both joined; repeated stop is idempotent; new renderer is reaped independently");
}
