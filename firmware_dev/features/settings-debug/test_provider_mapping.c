#include <assert.h>
#include <stdio.h>
#include "mapped_span.h"
#include "../hardware-accel/runtime-provider/code_guard.h"
int main(void){
 DiscMappedSpan s;
 assert(mapped_parse("69f1b000-69f29000 r-xp 00000000 00:14 1 /provider.so",&s));
 assert(mapped_contains(&s,0x69f1b004,4,1));
 assert(!mapped_contains(&s,0x69f29000,4,1));
 assert(!mapped_contains(&s,0x69f28ffc,8,1));
 assert(!mapped_contains(&s,0x69f1b004,0xfffffff0u,1));
 assert(!mapped_contains(&s,0x69f1b004,0,1));
 assert(mapped_parse("69f1b000-69f29000 r--p 00000000 00:14 1 /provider.so",&s));
 assert(mapped_contains(&s,0x69f1b004,60,0)&&!mapped_contains(&s,0x69f1b004,4,1));
 assert(mapped_parse("69f1b000-69f29000 ---p 00000000 00:14 1 /provider.so",&s));
 assert(!mapped_contains(&s,0x69f1b004,4,0));
 assert(!mapped_parse("69f1b000-69f1b000 r-xp",&s));
 assert(!mapped_parse("garbage",&s));
 unsigned char expected[16]={0},live[16]={0};assert(provider_code_equal(live,expected,16));
 for(unsigned i=0;i<16;i++){live[i]=1;assert(!provider_code_equal(live,expected,16));live[i]=0;}
 puts("PASS: valid code/data spans, no zero/overflow/cross-boundary/guard-page calls; exact byte guard catches each mismatch.");
 return 0;
}
