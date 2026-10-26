/* Exercise the production polar renderer's idle/wake lifecycle on both ABIs. */
#define size_t renderer_size_t
#include "polar.c"
#undef size_t
#include <assert.h>
#include <stdio.h>
static short input[960][2];
int main(void){
 reset();tick(1000);assert(idle);unsigned g=generation;
 for(unsigned t=1016;t<2000;t+=16)tick(t);assert(generation==g);
 feed(input,DISC_SCOPE_MAX_SAMPLES+1,48000,2000);assert(idle&&generation==g);
 feed(input,960,48000,2016);tick(2016);assert(!idle&&generation>g); /* Digital silence still draws the time/radius ring. */
 unsigned lit=0;for(unsigned i=0;i<129600;i++)lit|=left_glow[i]|right_glow[i];assert(lit);
 for(unsigned t=2200;t<5000;t+=16)tick(t);assert(idle);g=generation;
 for(unsigned t=5016;t<6000;t+=16)tick(t);assert(generation==g);
 for(unsigned i=0;i<960;i++){input[i][0]=(short)(i*137);input[i][1]=(short)(i*257);}
 feed(input,960,48000,6016);tick(6016);assert(generation>g&&!idle);
 reset();assert(!idle);feed(input,960,48000,7000);tick(7000);assert(!idle);
 puts("PASS production polar: idle revision stable, no stale trace, digital silence ring, invalid feed, resume and reset");fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
