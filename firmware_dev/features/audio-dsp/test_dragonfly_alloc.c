#include "dragonfly_adapter.h"
#include <assert.h>
#include <stdio.h>
void disc_fv_test_fail(unsigned);
unsigned disc_fv_test_outstanding(void);
int main(void){
 unsigned failures=0;
 for(unsigned n=1;n<512;n++){
  disc_fv_test_fail(n);void *d=disc_dragonfly_create(192000);
  if(d){float io[2]={.1f,-.1f};disc_dragonfly_sample(d,io);disc_dragonfly_destroy(d);assert(!disc_fv_test_outstanding());break;}
  failures++;assert(!disc_fv_test_outstanding());
 }
 assert(failures>50&&failures<511);disc_fv_test_fail(0);
 void *d=disc_dragonfly_create(384000);assert(d);disc_dragonfly_destroy(d);assert(!disc_fv_test_outstanding());
 printf("PASS: %u successive allocation failure points safely return NULL without leaked allocations; subsequent create works\n",failures);return 0;
}
