/* Reuse the stock sink parser's ONE CLI query. Only its native initialization
 * worker calls this guarded parser; observe its final codec comparison and
 * map V5 to family 8 after the original validation. No second query/process,
 * early pipe close, polling or UI-thread work. */
static int lhdcv5_parse_v5;
extern int system(const char *);
static int lhdcv5_launch(const char *command){
 /* This exact native call starts BlueALSA on its sink worker. The immutable
  * system manager starts 20 seconds after boot; it must not gate codec setup. */
 if(pc_enabled()&&!access("/usr/data/disc-runtime/components/disc-bt-lhdcv5/enabled",0)&&access("/usr/data/disc-runtime/components/disc-bt-lhdcv5/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0))
  system("/bin/sh /usr/data/disc-runtime/components/disc-bt-lhdcv5/current/entry.sh --prepare >/tmp/disc-lhdcv5-preflight.log 2>&1");
 return system(command);
}
static int lhdcv5_codec_compare(const char *selected,const char *expected){
 lhdcv5_parse_v5=pc_enabled()&&selected&&expected&&!strcmp(expected,"LDAC")&&!strcmp(selected,"LHDC-V5");
 return lhdcv5_parse_v5?0:strcmp(selected,expected);
}
static int lhdcv5_sink_param(void *ctx,const char *path){
 lhdcv5_parse_v5=0;
 int ret=PC_FN(0x4e1254,int (*)(void *,const char *))(ctx,path);
 if(!ret&&ctx&&lhdcv5_parse_v5){
  unsigned rate=PC_WORD((unsigned)ctx+4);
  int valid=pc_enabled()&&PC_WORD((unsigned)ctx+16)==9&&PC_WORD((unsigned)ctx+12)==2&&PC_WORD((unsigned)ctx+8)==32&&(rate==48000||rate==96000||rate==192000);
  PC_WORD((unsigned)ctx+16)=valid?8:0;
 }
 /* One RAM-only initialization record, never per audio frame. */
 char state[192];int n=snprintf(state,sizeof(state),"installed=1\nsingle_query=1\nresult=%d\nv5=%d\ncodec=%u\nrate=%u\nbits=%u\nchannels=%u\n",ret,lhdcv5_parse_v5,ctx?PC_WORD((unsigned)ctx+16):0,ctx?PC_WORD((unsigned)ctx+4):0,ctx?PC_WORD((unsigned)ctx+8):0,ctx?PC_WORD((unsigned)ctx+12):0);
 int fd=open("/tmp/disc-runtime/lhdcv5-player.state",1|0x100|0x200,0600);if(fd>=0){write(fd,state,n);close(fd);}
 return ret;
}
static int lhdcv5_sink_install(void){
 if(PC_WORD(0x4e00cc)!=0x0c138495||PC_WORD(0x4e157c)!=0x0c209b58||PC_WORD(0x4e1580)!=0x27a401b8||PC_WORD(0x4e1584)!=0x24030009||PC_WORD(0x4df928)!=0x0c209e30||PC_WORD(0x4df92c)!=0xafa2026c)return 0;
 unsigned *stub=(void *)-1;
 for(unsigned hint=0x07900000;hint<0x07904000;hint+=4096){
  stub=mmap((void *)hint,4096,3,0x802,-1,0);
  if(stub!=(void *)-1&&(unsigned)stub<0x10000000u)break;
  if(stub!=(void *)-1)munmap(stub,4096);stub=(void *)-1;
 }
 if(stub==(void *)-1)return 0;
 unsigned target[3]={(unsigned)lhdcv5_sink_param,(unsigned)lhdcv5_codec_compare,(unsigned)lhdcv5_launch};
 for(unsigned i=0;i<3;i++){stub[i*4]=0x3c190000|(target[i]>>16);stub[i*4+1]=0x37390000|(target[i]&65535);stub[i*4+2]=0x03200008;stub[i*4+3]=0;}
 if(cacheflush(stub,48,3)||mprotect(stub,4096,5)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x4e0000,4096,7)){munmap(stub,4096);return 0;}
 if(mprotect((void *)0x4e1000,4096,7)){mprotect((void *)0x4e0000,4096,5);munmap(stub,4096);return 0;}
 if(mprotect((void *)0x4df000,4096,7)){mprotect((void *)0x4e0000,4096,5);mprotect((void *)0x4e1000,4096,5);munmap(stub,4096);return 0;}
 PC_WORD(0x4e00cc)=0x0c000000|((unsigned)stub>>2);
 PC_WORD(0x4e157c)=0x0c000000|(((unsigned)stub+16)>>2);__sync_synchronize();
 PC_WORD(0x4df928)=0x0c000000|(((unsigned)stub+32)>>2);__sync_synchronize();
 int ok=!cacheflush((void *)0x4df000,12288,3);
 if(mprotect((void *)0x4df000,4096,5)||mprotect((void *)0x4e0000,4096,5)||mprotect((void *)0x4e1000,4096,5))ok=0;
 return ok;
}
