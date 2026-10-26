/* Two-second bounded READ-ONLY DMA3 register probe, outside the player.
 * No PCM/DAC opens, DMA configuration writes, FIFO data reads or live log IO. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),pread(int,void *,unsigned,long),write(int,const void *,unsigned);
extern int ioctl(int,unsigned,...),printf(const char *,...),snprintf(char *,unsigned,const char *,...);
extern int clock_gettime(int,void *),usleep(unsigned),munmap(void *,unsigned);
extern void *mmap(void *,unsigned,int,int,int,long),*dlsym(void *,const char *);
extern char *getenv(const char *);extern unsigned long strtoul(const char *,char **,int);
struct message {unsigned short addr,flags,len;unsigned char *buf;};
struct transfer {struct message *msgs;unsigned count;};
struct sample {unsigned ns,dba,dtc,dcr,dsr,dcm,dda,fcr,ffr,fsr;};
static struct sample rows[3000];
static unsigned header[40],last_header[40],player,mode;
static int read_word(int fd,unsigned a,unsigned *v){return a>=0x800000&&a<0x7ffffffc&&pread(fd,v,4,a)==4;}
static int scope(int fd,unsigned *h){
 unsigned m,p;return read_word(fd,0x83dfc0,&m)&&read_word(fd,m,&mode)&&mode==4&&
  read_word(fd,0x83dfc4,&p)&&p==player&&pread(fd,h,160,p)==160&&h[3]==352800&&h[4]==2&&h[5]==32&&h[6]==0&&h[7]==352800&&h[9]==32&&h[11]==2&&h[22]==1;
}
static int read_reg(int fd,unsigned addr,unsigned reg,unsigned char *value){
 unsigned char a[4]={reg>>16,reg>>8,reg,0};
 struct message msgs[2]={{addr,0,4,a},{addr,1,1,value}};struct transfer t={msgs,2};
 return ioctl(fd,0x707,&t)==2;
}
static int dac_snapshot(int fd,const char *stage,int identity){
 const unsigned regs[]={0x1000b,0x1000c,0x70004,0x40014,0x40016,0x5000a,0x5000b,0x20000,0xf0001,0xf0004};
 for(unsigned addr=0x30;addr<=0x32;addr+=2){
  if(identity){unsigned char id[3];
   for(unsigned i=0;i<3;i++)if(!read_reg(fd,addr,0x10000+i,&id[i]))return 0;
   if(id[0]!=0x43||id[1]!=0x13||id[2]!=0x10)return 0;
  }
  for(unsigned i=identity?0:8;i<10;i++){
   unsigned char v;if(!read_reg(fd,addr,regs[i],&v))return 0;
   printf("%s addr=0x%x reg=0x%x value=0x%x\n",stage,addr,regs[i],v);
  }
 }
 return 1;
}
static int save(const char *path,const void *p,unsigned n){
 int fd=open(path,1|0x100|0x200,0600);if(fd<0)return 0;unsigned done=0;
 while(done<n){int k=write(fd,(const unsigned char *)p+done,n-done);if(k<=0){close(fd);return 0;}done+=k;}
 close(fd);return 1;
}
static int probe(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;const char *s=getenv("DISC_PROBE_PID");if(!s)return 1;
 unsigned id=strtoul(s,0,10);if(!id||id>1000000)return 2;
 char path[80];snprintf(path,sizeof path,"/proc/%u/mem",id);int proc=open(path,0);
 if(proc<0||!read_word(proc,0x83dfc4,&player)||!scope(proc,header)){if(proc>=0)close(proc);return 3;}
 int fd=open("/dev/mem",0|0x101000);if(fd<0){close(proc);return 4;}
 volatile unsigned *mm=mmap(0,8192,1,1,fd,0x134d0000);close(fd);
 if(mm==(void *)-1){close(proc);return 5;}
 /* DMA3 must already be enabled in linked descriptor mode,
  * and its FIFO must be enabled. No probe attempts to start hardware. */
 if(!(mm[0x50/4]&1)||!(mm[0x1034/4]&1)||(mm[0x58/4]&0x80000000)){
  munmap((void *)mm,8192);close(proc);return 6;
 }
 int i2c=open("/dev/i2c-3",2);if(i2c<0||!dac_snapshot(i2c,"before",1)){
  if(i2c>=0)close(i2c);munmap((void *)mm,8192);close(proc);return 7;
 }
 long begin[2],now[2];clock_gettime(1,begin);unsigned count=0,elapsed=0;
 while(count<3000){
  clock_gettime(1,now);elapsed=(unsigned)(now[0]-begin[0])*1000000000u+(unsigned)(now[1]-begin[1]);
  if(elapsed>=2000000000u)break;
  struct sample *r=&rows[count++];r->ns=elapsed;
  r->dba=mm[0x48/4];r->dtc=mm[0x4c/4];r->dcr=mm[0x50/4];r->dsr=mm[0x54/4];r->dcm=mm[0x58/4];r->dda=mm[0x5c/4];
  r->fcr=mm[0x1034/4];r->ffr=mm[0x1038/4];r->fsr=mm[0x103c/4];
  usleep(1000);
 }
 int same=scope(proc,last_header),end_ok=dac_snapshot(i2c,"after",0);
 close(i2c);munmap((void *)mm,8192);close(proc);
 /* Only export after the sampling loop has ended. */
 int ok=save("/tmp/disc-runtime/dma3-readonly.bin",rows,count*sizeof(rows[0]));
 printf("pid=%u\nsamples=%u\nrow_bytes=%u\nrate=%u\nscope_unchanged=%d\nend_DAC_read_ok=%d\nelapsed_ns=%u\nsaved=%d\nDMA_writes=0\nPCM_DAC_open=0\n",id,count,(unsigned)sizeof(rows[0]),header[3],same,end_ok,elapsed,ok);
 return same&&end_ok&&ok&&count?0:8;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 return next?next(probe,argc,argv,init,fini,rtld_fini,stack):90;
}
