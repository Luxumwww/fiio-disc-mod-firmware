#include <stdio.h>
#include <string.h>
#include <assert.h>
extern int open(const char *,int,...),read(int,void *,unsigned),close(int);
#include "cache_sync.h"
int main(void){
 DiscMappedSpan s;const char *owner="/usr/data/module.so";
 assert(cache_owned_span("12345000-12346000 r-xp 00000000 00:14 1 /usr/data/module.so",owner,&s));
 assert(s.low==0x12345000&&s.high==0x12346000);
 assert(!cache_owned_span("12345000-12346000 r--p 00000000 00:14 1 /usr/data/module.so",owner,&s));
 assert(!cache_owned_span("12345000-12346000 ---p 00000000 00:14 1 /usr/data/module.so",owner,&s));
 assert(!cache_owned_span("12345000-12346000 r-xp 00000000 00:14 1 /evil/usr/data/module.so",owner,&s));
 assert(!cache_owned_span("12345000-12346000 r-xp 00000000 00:14 1 /usr/data/module.so.other",owner,&s));
 assert(!cache_owned_span("12345000-12346000 r-xp 00000000 00:14 1 /usr/data/another.so",owner,&s));
 assert(cache_owned_span("12345000-12346000 r-xp 00000000 00:14 1 /usr/data/module.so (deleted)",owner,&s));
 puts("PASS: only owned readable executable mappings selected; guards, data, other files and prefix collisions rejected.");return 0;
}
