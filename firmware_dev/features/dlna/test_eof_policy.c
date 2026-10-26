#include <assert.h>
#include <stdio.h>
#include "eof_policy.h"
int main(void){
 assert(dlna_eof_candidate(3,1,8,7000,1,10));
 assert(!dlna_eof_candidate(1,1,8,7000,1,10));
 assert(!dlna_eof_candidate(3,3,8,7000,1,10));
 assert(!dlna_eof_candidate(3,0,8,7000,1,10));
 assert(!dlna_eof_candidate(3,1,0,7000,1,10));
 assert(!dlna_eof_candidate(3,1,8,2000,1,10));
 assert(!dlna_eof_candidate(3,1,8,7000,9,10));
 assert(!dlna_eof_candidate(3,1,86401,7000,1,10));
 puts("PASS: known-length EOF; pause, active playback, early stop, unknown duration and recent format switch are excluded");
}
