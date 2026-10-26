#include <assert.h>
#include <stdint.h>
#include <stdio.h>
static void *fast_timer;static unsigned normal_period;static int fast_ui;
typedef struct {unsigned period;} Timer;
static _Alignas(16) unsigned char display_data[32];
static unsigned calls;
static void set_display(Timer *p){*(Timer **)(void *)(display_data+8)=p;}
static void *get_display(void){return display_data+4;}
static void set_period(void *p,unsigned n){((Timer *)p)->period=n;calls++;}
#define lv_disp_default get_display
#define FN(address,type) set_period
/* Place fixture base at+4 so its timer at+8 is pointer-aligned on both ABIs. */
#include "../bluetooth-meter/visual_refresh.h"
int main(void){
 Timer a={17},b={25};set_display(&a);
 frameperiod(33);assert(a.period==33&&normal_period==17&&!fast_ui&&calls==1);
 frameperiod(33);assert(calls==1);
 frameperiod(16);assert(a.period==16&&normal_period==17&&fast_ui&&calls==2);
 frameperiod(33);assert(a.period==33&&!fast_ui&&calls==3);
 frameperiod(0);assert(a.period==17&&!fast_ui&&!fast_timer&&calls==4);
 frameperiod(16);a.period=42;frameperiod(33);assert(a.period==42&&!fast_ui);
 frameperiod(0);assert(a.period==42&&!fast_timer);
 a.period=17;frameperiod(33);set_display(&b);frameperiod(16);assert(b.period==16&&normal_period==25&&fast_ui);
 frameperiod(0);assert(b.period==25&&!fast_ui&&!fast_timer);
 set_display(0);frameperiod(33);assert(!fast_ui&&!fast_timer);
 puts("PASS home/scopes/shade period handoff, original restore, external owner, replacement/missing timer");fflush(stdout);return 0;
}
#ifdef __mips__
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");
#endif
