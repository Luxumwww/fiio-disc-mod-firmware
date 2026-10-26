#include <assert.h>
#include <stdio.h>
#include "lease.h"
int main(void){DiscLease l={0};unsigned page=lease_claim(&l);assert(page);assert(!lease_claim(&l));assert(!lease_release(&l,0));assert(!lease_release(&l,page+1));assert(lease_valid(&l,page));assert(lease_release(&l,page));assert(!lease_valid(&l,page));unsigned next=lease_claim(&l);assert(next&&next!=page);assert(!lease_release(&l,page));assert(lease_valid(&l,next));assert(lease_release(&l,next));l.sequence=0xffffffffu;assert(lease_claim(&l)==1);puts("PASS: exclusive page lease, busy rejection, stale release, no preemption, sequence wrap skips zero");return 0;}
