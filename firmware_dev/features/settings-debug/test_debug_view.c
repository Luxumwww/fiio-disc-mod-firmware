#include <assert.h>
#include <stdio.h>
#include "debug_view.h"
int main(void){
 DiscDebugView v={0};
 assert(disc_debug_value(&v,1,0,1,0,1,1,0,0)==2); /* DAC owns USB: pending intent is paused. */
 assert(disc_debug_value(&v,100,0,1,0,0,1,0,0)==2);
 assert(disc_debug_value(&v,101,0,1,0,1,0,0,0)==3);
 assert(disc_debug_value(&v,131,0,1,0,1,0,0,0)==5); /* Recovery failed, never spinner forever. */
 assert(disc_debug_value(&v,132,0,1,1,1,0,0,0)==1);
 assert(disc_debug_value(&v,133,1,1,0,1,0,0,1)==3);
 assert(disc_debug_value(&v,163,1,1,0,1,0,0,1)==5);
 assert(disc_debug_value(&v,164,1,1,1,1,0,0,0)==1);
 assert(disc_debug_value(&v,165,1,1,0,0,0,0,0)==3);
 assert(disc_debug_value(&v,175,1,1,0,0,0,0,0)==5);
 assert(disc_debug_value(&v,176,1,0,0,1,0,0,0)==0);
 assert(disc_debug_value(&v,177,0,1,1,1,1,1,0)==4);
 assert(disc_debug_value(&v,178,1,1,0,1,0,1,0)==0);
 puts("PASS: USB ownership pauses intent, success observed, finite switch timeout, stale manager detected, changed request resets pending, rescue priority");
}
