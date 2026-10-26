#include <assert.h>
#include <stdio.h>
#include "startup_gate.h"
int main(void){
 DiscStartupGate s={0};
 /* Even a reentrant sleep while the dynamic loader runs constructors cannot
  * load a consumer before native initialization is available. */
 assert(!startup_claim(&s,0x40397c,1,1));
 s.constructor_ready=1;
 assert(!startup_claim(&s,0x40397c,0,1));
 assert(!startup_claim(&s,0x40397c,1,0));
 assert(!startup_claim(&s,0x403978,1,1));
 assert(startup_claim(&s,0x40397c,1,1));
 assert(!startup_claim(&s,0x40397c,1,1));
 startup_complete(&s);
 for(unsigned i=0;i<1000;i++)assert(!startup_claim(&s,0x40397c,1,1));
 s=(DiscStartupGate){.constructor_ready=1};
 assert(startup_claim(&s,0x40397c,1,1));
 puts("PASS: no constructor/foreign/uninitialized/unlocked/recursive loading; one native-loop initialization per UI process.");
 return 0;
}
