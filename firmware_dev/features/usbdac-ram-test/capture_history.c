/* One bounded historical snapshot, outside player. No audio/DAC device opens. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),pread(int,void *,unsigned,long),write(int,const void *,unsigned);
extern int snprintf(char *,unsigned,const char *,...),clock_gettime(int,void *);
extern char *getenv(const char *);
extern unsigned long strtoul(const char *,char **,int);
static unsigned char data[3500000];
static unsigned player,header[40];static int mem;
static int get(unsigned a,void *b,unsigned n){return a>=0x800000&&a<=0x7fffffff-n&&pread(mem,b,n,a)==(int)n;}
static int save(const char *path,const void *b,unsigned n){
 int f=open(path,1|0x100|0x200,0600);if(f<0)return 0;unsigned done=0;
 while(done<n){int w=write(f,(const unsigned char *)b+done,n-done);if(w<=0){close(f);return 0;}done+=(unsigned)w;}
 close(f);return 1;
}
static void snap(unsigned offset,const char *name,unsigned size){
 unsigned r[10],after[10],end_header[40];char text[1200],path[100];long begin[2],end[2];
 if(!get(player+offset,r,40)||r[9]<=size||r[9]>8*1024*1024||r[4]>=r[9]||(r[4]&7))return;
 unsigned start=(r[4]+r[9]-size)%r[9],first=size;if(first>r[9]-start)first=r[9]-start;
 clock_gettime(1,begin);
 if(!get(r[0]+start,data,first)||(first<size&&!get(r[0],data+first,size-first))||!get(player+offset,after,40)||!get(player,end_header,160))return;
 clock_gettime(1,end);
 unsigned long long a=(unsigned long long)r[3]<<32|r[2],b=(unsigned long long)after[3]<<32|after[2];
 int stable=after[0]==r[0]&&after[9]==r[9]&&b>=a&&b-a<r[9]-size&&end_header[3]==header[3]&&end_header[11]==header[11];
 snprintf(path,sizeof path,"/tmp/disc-runtime/sine-history-%s.bin",name);int saved=save(path,data,size);
 int n=snprintf(text,sizeof text,"stable=%d\nsaved=%d\nrate=%u\nclass=%u\noutput_rate=%u\nheader_chunk_bytes=%u\noutput_format=%u\nDSD_strategy=%u\nstart=%u\nsize=%u\ncapacity=%u\nwrite_before=%llu\nwrite_after=%llu\nread_before=%llu\ncopy_begin=%ld.%09ld\ncopy_end=%ld.%09ld\n",stable,saved,header[3],header[11],header[7],header[1],header[10],header[38],start,size,r[9],a,b,(unsigned long long)r[7]<<32|r[6],begin[0],begin[1],end[0],end[1]);
 snprintf(path,sizeof path,"/tmp/disc-runtime/sine-history-%s.state",name);save(path,text,n);
}
__attribute__((constructor)) static void capture(void){
 const char *s=getenv("DISC_PROBE_PID");if(!s)return;unsigned id=strtoul(s,0,10);if(!id||id>1000000)return;
 char path[80];snprintf(path,sizeof path,"/proc/%u/mem",id);mem=open(path,0);if(mem<0)return;
 if(get(0x83dfc4,&player,4)&&get(player,header,160)&&header[3]==352800&&header[4]==2&&header[5]==32&&header[6]==0&&header[11]==2&&header[22]==1){
  save("/tmp/disc-runtime/sine-history-header.bin",header,160);
  snap(0x130,"input",1700000);snap(0x170,"output",3500000);
 }
 close(mem);
}
