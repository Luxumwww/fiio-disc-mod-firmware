/* Bounded metadata-only observation, outside player. No PCM/DAC opens,
 * audio copies, SYNC_PTR ioctl, control/configuration writes or TF logging. */
extern int open(const char *,int,...),close(int),pread(int,void *,unsigned,long),write(int,const void *,unsigned);
extern int printf(const char *,...),snprintf(char *,unsigned,const char *,...),clock_gettime(int,void *),usleep(unsigned),munmap(void *,unsigned);
extern void *mmap(void *,unsigned,int,int,int,long),*dlsym(void *,const char *);
extern char *getenv(const char *);extern unsigned long strtoul(const char *,char **,int);
struct row {unsigned ns,in[10],out[10],pcm[6],status[12],control[2],dba,dtc,dcs,consistent;};
static struct row rows[2400];
static int get(int fd,unsigned address,void *p,unsigned n){
 return address>=0x800000&&address<0x80000000u-n&&pread(fd,p,n,address)==(int)n;
}
static int save(const void *p,unsigned size){
 int fd=open("/tmp/disc-runtime/queue-progress.bin",1|0x100|0x200,0600);if(fd<0)return 0;
 unsigned done=0;while(done<size){int n=write(fd,(const char *)p+done,size-done);if(n<=0){close(fd);return 0;}done+=(unsigned)n;}
 close(fd);return 1;
}
static int probe(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;const char *s=getenv("DISC_PROBE_PID");if(!s)return 1;
 unsigned pid=strtoul(s,0,10);char path[80];snprintf(path,sizeof path,"/proc/%u/mem",pid);
 int fd=open(path,0);unsigned player,modeptr,mode,sink,pcm,words[50],header[40];
 if(fd<0)return 2;
 if(!get(fd,0x83dfc0,&modeptr,4)||!get(fd,modeptr,&mode,4)||mode!=4||
    !get(fd,0x83dfc4,&player,4)||!get(fd,player,header,160)||header[3]!=352800||header[11]!=2||header[22]!=1||
    !get(fd,0x83dfe4,&sink,4)||!get(fd,sink+0x58,&mode,4)||mode!=6||
    !get(fd,sink+6*0x54+0x74,&pcm,4)||!get(fd,pcm,words,200)||words[38]!=2||words[39]!=352800||
    words[40]!=14112||words[41]!=2||words[42]!=7){close(fd);return 3;}
 int mem=open("/dev/mem",0|0x101000);if(mem<0){close(fd);return 4;}
 volatile unsigned *dma=mmap(0,4096,1,1,mem,0x134d0000);close(mem);
 if(dma==(void *)-1){close(fd);return 5;}
 long begin[2],now[2];clock_gettime(1,begin);unsigned count=0,scope_ok=1;
 while(count<2400){
  clock_gettime(1,now);unsigned ns=(unsigned)(now[0]-begin[0])*1000000000u+(unsigned)(now[1]-begin[1]);
  if(ns>=4000000000u)break;
  struct row *r=&rows[count];unsigned check_in[10],check_out[10],pcheck,modecheck;
  if(!get(fd,0x83dfc4,&pcheck,4)||pcheck!=player||!get(fd,modeptr,&modecheck,4)||modecheck!=4||
     !get(fd,player+0x130,r->in,40)||!get(fd,player+0x170,r->out,40)||
     !get(fd,pcm,r->pcm,24)||!get(fd,words[46],r->status,48)||!get(fd,words[47],r->control,8)||
     !get(fd,player+0x130,check_in,40)||!get(fd,player+0x170,check_out,40)){
   scope_ok=0;break;
  }
  r->ns=ns;r->dba=dma[0x48/4];r->dtc=dma[0x4c/4];r->dcs=dma[0x50/4];
  /* Counter reads are not atomic. Reject differences and transient mixed
   * snapshots before interpreting queue occupancy or underflow. */
  r->consistent=1;
  for(unsigned i=0;i<10;i++)if(r->in[i]!=check_in[i]||r->out[i]!=check_out[i])r->consistent=0;
  count++;usleep(2000);
 }
 unsigned pcheck,endheader[40];
 scope_ok=scope_ok&&get(fd,0x83dfc4,&pcheck,4)&&pcheck==player&&get(fd,player,endheader,160)&&endheader[3]==352800&&endheader[11]==2;
 munmap((void *)dma,4096);close(fd);
 int saved=save(rows,count*sizeof(rows[0]));
 printf("pid=%u\nplayer=0x%x\npcm=0x%x\nsamples=%u\nrow_bytes=%u\nscope_unchanged=%u\nsaved=%u\n",pid,player,pcm,count,(unsigned)sizeof(rows[0]),scope_ok,saved);
 return saved&&scope_ok&&count>100?0:6;
}
int __libc_start_main(int (*mainfn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack){
 (void)mainfn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 return next?next(probe,argc,argv,init,fini,rtld_fini,stack):90;
}
