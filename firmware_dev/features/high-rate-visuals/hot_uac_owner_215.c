#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdint.h>
#define PC_WORD(a) (*(volatile unsigned *)(uintptr_t)(a))
#include "../bluetooth-meter/uac_capture_guard.h"
extern int mprotect(void *,unsigned,int),cacheflush(void *,int,int);
__attribute__((constructor)) static void install_owner(void){
 const unsigned addresses[4]={0x461188,0x461644,0x4614bc,0x461620};
 const unsigned originals[4][2]={{0x27bdffb8,0xafb3003c},{0x27bdffc0,0xafb40038},{0x27bdffc8,0xafb1002c},{0x3c020084,0x03e00008}};
 for(unsigned i=0;i<4;i++){unsigned instruction=PC_WORD(addresses[i]);if(instruction!=originals[i][0]&&(instruction>>26!=2||((instruction&0x3ffffff)<<2)<0x08100000u||((instruction&0x3ffffff)<<2)>=0x08101000u))return;}
 if(!uac_capture_guard_install()||mprotect((void *)0x461000,4096,7))return;
 for(unsigned i=0;i<4;i++){PC_WORD(addresses[i])=originals[i][0];PC_WORD(addresses[i]+4)=originals[i][1];}
 __sync_synchronize();cacheflush((void *)0x461000,4096,3);mprotect((void *)0x461000,4096,5);
}
