/* Read-only one-shot evidence; no GUI or PLAYER writes. */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/mman.h>
#include "../bluetooth-meter/system_metrics.h"
static unsigned word(int f,unsigned a){unsigned v=0;pread(f,&v,4,a);return v;}
static int probe(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;int physical=open("/dev/mem",O_RDONLY);if(physical>=0){volatile unsigned *r=mmap(0,4096,PROT_READ,MAP_SHARED,physical,0x10000000);if(r!=(void *)-1){printf("CPCCR=%08x CPAPCR=%08x CPU_MHz=%u\n",r[0],r[4],disc_cpu_mhz(r[0],r[4]));munmap((void *)r,4096);}close(physical);}char p[64];snprintf(p,sizeof(p),"/proc/%s/mem",getenv("UI_PID"));int ui=open(p,O_RDONLY);
 unsigned label=word(ui,0x8de20c),text=word(ui,label+0x2c);char b[128]={0};if(text)pread(ui,b,127,text);printf("UI_rate=%s label=%x text=%x\n",b,label,text);close(ui);
 snprintf(p,sizeof(p),"/proc/%s/mem",getenv("PLAYER_PID"));int f=open(p,O_RDONLY);unsigned player=word(f,0x83dfc4),mp=word(f,0x83dfc0);printf("player=%x workmode=%u digital=%u dac_dsd=%u\n",player,word(f,mp),word(f,0x839a74),word(f,0x899060)&255);
 unsigned h[40];pread(f,h,sizeof(h),player);for(unsigned j=0;j<40;j++)printf("h[%u]=%u%s",j,h[j],j%4==3?"\n":" ");unsigned a=word(f,player+0x138),ah=word(f,player+0x13c);usleep(500000);printf("ring_before=%u:%u ring_after=%u:%u\n",ah,a,word(f,player+0x13c),word(f,player+0x138));close(f);return 0;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym(RTLD_NEXT,"__libc_start_main");return next?next(probe,argc,argv,init,fini,rtld_fini,stack):90;
}
