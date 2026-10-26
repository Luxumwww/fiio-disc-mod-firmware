/* One verified, read-only DMA3 descriptor/buffer snapshot. No writes to RAM
 * used by audio, no FIFO reads, no PCM/DAC device opens or reconfiguration. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),pread(int,void *,unsigned,long),write(int,const void *,unsigned);
extern int printf(const char *,...),snprintf(char *,unsigned,const char *,...),clock_gettime(int,void *),munmap(void *,unsigned);
extern void *mmap(void *,unsigned,int,int,int,long),*dlsym(void *,const char *);
extern char *getenv(const char *);extern unsigned long strtoul(const char *,char **,int);
static unsigned copies[2][56448];
static int descriptor(int fd,unsigned address,unsigned d[4]){
 if(address<0x10000||address>0x07fffff0||(address&15))return 0;
 unsigned page=address&~4095u,offset=address&4095u;
 volatile unsigned *p=mmap(0,4096,1,1,fd,page);if(p==(void *)-1)return 0;
 for(unsigned i=0;i<4;i++)d[i]=p[offset/4+i];
 munmap((void *)p,4096);return 1;
}
static int word(int fd,unsigned a,unsigned *v){return a>=0x800000&&a<0x7ffffffc&&pread(fd,v,4,a)==4;}
static int save(const char *path,const void *p,unsigned n){
 int fd=open(path,1|0x100|0x200,0600);if(fd<0)return 0;unsigned done=0;
 while(done<n){int k=write(fd,(const unsigned char *)p+done,n-done);if(k<=0){close(fd);return 0;}done+=k;}
 close(fd);return 1;
}
static int probe(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;const char *s=getenv("DISC_PROBE_PID");if(!s)return 1;
 unsigned id=strtoul(s,0,10);char path[80];snprintf(path,sizeof path,"/proc/%u/mem",id);
 int proc=open(path,0);unsigned p,m,mode,h[40],end_p,end_h[40];
 if(proc<0||!word(proc,0x83dfc0,&m)||!word(proc,m,&mode)||mode!=4||!word(proc,0x83dfc4,&p)||
  pread(proc,h,160,p)!=160||h[3]!=352800||h[7]!=352800||h[11]!=2||h[22]!=1){if(proc>=0)close(proc);return 2;}
 int fd=open("/dev/mem",0|0x101000);if(fd<0){close(proc);return 3;}
 volatile unsigned *dma=mmap(0,4096,1,1,fd,0x134d0000);
 if(dma==(void *)-1){close(fd);close(proc);return 4;}
 unsigned address=dma[0x5c/4],d[8],normal[2],found=0,steps=0;
 /* DDA retains the initial dummy descriptor. Follow its validated links
  * into the real two-period ring instead of treating it as the active one. */
 while(found<2&&steps<4){
  unsigned entry[4];if(!descriptor(fd,address,entry))break;
  printf("desc_%u_addr=0x%x flags=0x%x DBA=0x%x next=0x%x count=%u\n",steps,address,entry[0],entry[1],entry[2],entry[3]);steps++;
  if(!(entry[0]&4)||((entry[0]>>4)&7)!=4)break;
  if(entry[0]&0x100){
   if(entry[3]!=882||(found&&address==normal[0]))break;
   normal[found]=address;for(unsigned i=0;i<4;i++)d[found*4+i]=entry[i];found++;
  }
  address=entry[2];
 }
 unsigned base=found==2?(d[1]<d[5]?d[1]:d[5]):0,other=found==2?(d[1]>d[5]?d[1]:d[5]):0;
 unsigned live=dma[0x48/4];
 if(found!=2||d[2]!=normal[1]||d[6]!=normal[0]||base<0x10000||base>0x07fc8000||(base&4095)||other-base!=112896||
  live<base||live>=base+225792||!(dma[0x50/4]&1)){
  munmap((void *)dma,4096);close(fd);close(proc);return 5;
 }
 volatile unsigned *ring=mmap(0,229376,1,1,fd,base);close(fd);
 if(ring==(void *)-1){munmap((void *)dma,4096);close(proc);return 6;}
 long begin[2],end[2];unsigned pos_before=dma[0x48/4];clock_gettime(1,begin);
 for(unsigned c=0;c<2;c++)for(unsigned i=0;i<56448;i++)copies[c][i]=ring[i];
 clock_gettime(1,end);unsigned pos_after=dma[0x48/4],different=0;
 for(unsigned i=0;i<56448;i++)if(copies[0][i]!=copies[1][i])different++;
 int same=word(proc,0x83dfc4,&end_p)&&end_p==p&&pread(proc,end_h,160,p)==160&&end_h[3]==h[3]&&end_h[11]==2;
 munmap((void *)ring,229376);munmap((void *)dma,4096);close(proc);
 int saved=save("/tmp/disc-runtime/dma3-buffer-a.bin",copies[0],sizeof copies[0]);
 saved&=save("/tmp/disc-runtime/dma3-buffer-b.bin",copies[1],sizeof copies[1]);
 printf("pid=%u\nbytes=%u\ncopy_begin=%ld.%09ld\ncopy_end=%ld.%09ld\nDBA_before=0x%x\nDBA_after=0x%x\ndifferent_words=%u\nscope_unchanged=%d\nsaved=%d\n",id,(unsigned)sizeof copies[0],begin[0],begin[1],end[0],end[1],pos_before,pos_after,different,same,saved);
 return saved&&same?0:7;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 return next?next(probe,argc,argv,init,fini,rtld_fini,stack):90;
}
