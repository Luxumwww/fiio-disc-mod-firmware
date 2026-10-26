/* One-shot CS43131 frame repair, guarded by live classified stream + BAIC3.
 * No audio processing, logging loop, native binary changes or kernel writes. */
extern int open(const char *,int,...),close(int),read(int,void *,unsigned),pread(int,void *,unsigned,long),ioctl(int,unsigned,...),printf(const char *,...),snprintf(char *,unsigned,const char *,...);
extern char *getenv(const char *);extern unsigned long strtoul(const char *,char **,int);extern void *dlsym(void *,const char *);
extern void *mmap(void *,unsigned,int,int,int,long);extern int munmap(void *,unsigned);
struct message{unsigned short addr,flags,len;unsigned char *buf;};struct transfer{struct message *msgs;unsigned count;};
static int reg(int fd,unsigned addr,unsigned r,unsigned char *v,int write){
 unsigned char b[5]={r>>16,r>>8,r,0,*v};struct message m[2]={{addr,0,write?5:4,b},{addr,1,1,v}};struct transfer t={m,write?1u:2u};return ioctl(fd,0x707,&t)==(write?1:2);
}
static int set(int f,unsigned a,unsigned r,unsigned char v){unsigned char actual=0;return reg(f,a,r,&v,1)&&reg(f,a,r,&actual,0)&&actual==v;}
static int capture(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;char path[64];const char *pid=getenv("DISC_PROBE_PID");if(!pid)return 10;
 unsigned n=strtoul(pid,0,10),p=0,h[40]={0},dsd=0;snprintf(path,sizeof path,"/proc/%u/mem",n);int mem=open(path,0);
 if(mem<0)return 11;
 int valid=pread(mem,&p,4,0x83dfc4)==4&&p>=0x800000&&p<0x7ffff000&&pread(mem,h,sizeof h,p)==sizeof h&&pread(mem,&dsd,4,0x899060)==4;
 close(mem);if(!valid||h[3]!=352800||h[7]!=352800||h[4]!=2||h[5]!=32||h[6]!=0||h[8]!=2||h[9]!=32||h[11]!=2||h[22]!=1||h[38]!=2||(dsd&255)!=1)return 12;
 unsigned cfg=0,div=0;int phys=open("/dev/mem",0);if(phys<0)return 13;
 void *mapped=mmap(0,0x4000,1,1,phys,0x134d5000);valid=mapped!=(void *)-1;
 if(valid){cfg=*(volatile unsigned *)((char *)mapped+0x3014);div=*(volatile unsigned *)((char *)mapped+0x3018);munmap(mapped,0x4000);}close(phys);
 if(!valid||cfg!=0x20711||div!=0x20003)return 14;
 int f=open("/dev/i2c-3",2);if(f<0)return 15;
 const unsigned rr[]={0x10000,0x10001,0x10002,0x1000b,0x70004,0x40018,0x40019,0x20000,0x40014,0x40015,0x40016,0x40017};unsigned char old[2][12]={{0}};
 for(unsigned c=0;c<2;c++)for(unsigned i=0;i<12;i++)if(!reg(f,0x30+2*c,rr[i],&old[c][i],0)){close(f);return 16;}
 for(unsigned c=0;c<2;c++)if(old[c][0]!=0x43||old[c][1]!=0x13||old[c][2]!=0x10||old[c][3]!=7||old[c][4]!=0x54||old[c][5]!=0x0c||old[c][6]!=0x0a||old[c][7]!=0xa2||old[c][8]!=23||old[c][9]||old[c][10]!=47||old[c][11]){close(f);return 17;}
 int dac=open("/dev/cs43131",2);if(dac<0){close(f);return 18;}if(ioctl(dac,0x20004d2d,0)){close(dac);close(f);return 19;}
 int ok=1;for(unsigned c=0;c<2&&ok;c++){
  unsigned a=0x30+2*c;ok=set(f,a,0x20000,0xf2)&&set(f,a,0x40014,31)&&set(f,a,0x40016,63)&&set(f,a,0x20000,0xf2);
 }
 /* Native mute powers ASP down (0xf2). Restore power through native unmute. */
 if(!ok)for(unsigned c=0;c<2;c++){unsigned a=0x30+2*c;set(f,a,0x20000,0xf2);set(f,a,0x40014,old[c][8]);set(f,a,0x40016,old[c][10]);}
 int unmute=ioctl(dac,0x20004d2e,0);printf("frame_repair=%d native_unmute=%d BAIC_CFG=0x%x BAIC_DIV=0x%x\n",ok,unmute,cfg,div);
 for(unsigned c=0;c<2;c++)for(unsigned i=7;i<12;i++){unsigned char v=0;reg(f,0x30+2*c,rr[i],&v,0);printf("addr=0x%x reg=0x%x value=0x%x\n",0x30+2*c,rr[i],v);}
 close(dac);close(f);return ok&&!unmute?0:20;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");return next?next(capture,argc,argv,init,fini,rtld_fini,stack):90;
}
