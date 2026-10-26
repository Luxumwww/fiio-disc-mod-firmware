#include <assert.h>
#include <stdio.h>
#include "system_metrics.h"
int main(void){
 DiscCpuSample s={0};unsigned a[8]={100,0,100,600,200,0,0,0},b[8]={200,0,100,700,200,0,0,0};
 assert(disc_cpu_percent(&s,a)==-1);assert(disc_cpu_percent(&s,b)==50);assert(disc_cpu_percent(&s,b)==-1);
 assert(disc_memory_used(120000,50000)==70000&&disc_memory_used(100,200)==0);
 unsigned pll=49u<<20;assert(!disc_cpu_mhz(0x90000000,pll)); /* Reject a clock outside the reviewed range. */
 assert(!disc_cpu_mhz(0x10000000,pll));
 unsigned stock_pll=24u<<20;assert(disc_cpu_mhz(0x90000000,stock_pll)==1200);assert(disc_cpu_mhz(0x90000001,stock_pll)==600);
 puts("PASS: all-core aggregate percentage; no duplicate/stale delta; reclaim-aware memory; validated APLL source and CPU divider");
}
