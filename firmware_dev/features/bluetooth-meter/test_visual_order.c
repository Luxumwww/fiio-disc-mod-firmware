#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "visual_toolbar_order.h"
static unsigned cases;
static void permutation(unsigned *o,unsigned at){
 if(at==6){char b[64];unsigned parsed[6];snprintf(b,sizeof b,"%u %u %u %u %u %u\n",o[0],o[1],o[2],o[3],o[4],o[5]);assert(vt_order_parse(b,parsed)&&!memcmp(o,parsed,sizeof parsed));for(unsigned id=0;id<6;id++)for(unsigned slot=0;slot<6;slot++){unsigned moved[6];memcpy(moved,o,sizeof moved);vt_order_move(moved,id,slot);assert(moved[slot]==id);unsigned seen=0;for(unsigned i=0;i<6;i++){assert(moved[i]<6);seen|=1u<<moved[i];}assert(seen==63);}cases++;return;}
 for(unsigned i=at;i<6;i++){unsigned v=o[at];o[at]=o[i];o[i]=v;permutation(o,at+1);v=o[at];o[at]=o[i];o[i]=v;}
}
int main(void){unsigned o[6];vt_order_default(o);permutation(o,0);assert(cases==720);const char *bad[]={"","0 1 2 3 4","0 1 2 3 4 4","0 1 2 3 4 6","-1 1 2 3 4 5","4294967296 1 2 3 4 5","0 1 2 3 4 5 trailing","0 1 2 3 4 5 0"};for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++){unsigned before[6];memcpy(before,o,sizeof o);assert(!vt_order_parse(bad[i],o)&&!memcmp(before,o,sizeof o));}for(unsigned i=0;i<6;i++)assert(vt_nearest_slot(vt_slot_x(i),vt_slot_y(i))==i);puts("PASS 720 permutations / 25920 moves, malformed/overflow no-change, six-slot geometry");}
