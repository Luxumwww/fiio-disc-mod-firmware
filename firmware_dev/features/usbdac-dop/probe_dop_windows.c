/* Bounded read-only process-memory snapshot; runs in /bin/true, not player. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),read(int,void *,unsigned),write(int,const void *,unsigned),pread(int,void *,unsigned,long);
extern int snprintf(char *,unsigned,const char *,...);
extern char *getenv(const char *);
extern unsigned long strtoul(const char *,char **,int);
static unsigned char data[8192];
static unsigned player,header[40];
static int mem;
static int get(unsigned address,void *dest,unsigned size){return address>=0x800000&&address<=0x7fffffff-size&&pread(mem,dest,size,address)==(int)size;}
static int save(const char *path,const void *b,unsigned n){int f=open(path,1|0x100|0x200,0600);if(f<0)return 0;int ok=write(f,b,n)==(int)n;close(f);return ok;}
static void ring_snapshot(unsigned offset,const char *name){
 unsigned r[10],after[10];char text[768],path[100];
 if(!get(player+offset,r,sizeof r)||r[9]<sizeof data||r[9]>8*1024*1024||r[4]>=r[9])return;
 unsigned start=(r[4]+r[9]-sizeof data)%r[9],first=sizeof data;if(first>r[9]-start)first=r[9]-start;
 if(!get(r[0]+start,data,first)||(first<sizeof data&&!get(r[0],data+first,sizeof data-first))||!get(player+offset,after,sizeof after))return;
 unsigned long long a=(unsigned long long)r[3]<<32|r[2],b=(unsigned long long)after[3]<<32|after[2];
 unsigned invalid=0,phase=0,stereo=0,first_bad=sizeof data;
 for(unsigned i=0;i<sizeof data;i+=8){unsigned marker=data[i+3];if(marker!=5&&marker!=250){invalid++;if(first_bad==sizeof data)first_bad=i;}if(marker!=data[i+7])stereo++;if(i&&marker!=(unsigned)(data[i-5]^255))phase++;}
 int stable=after[0]==r[0]&&after[9]==r[9]&&b>=a&&b-a<r[9]-sizeof data;
 snprintf(path,sizeof path,"/tmp/disc-runtime/dop-window-%s.bin",name);save(path,data,sizeof data);
 int n=snprintf(text,sizeof text,"stable=%d\nrate=%u\nclass=%u\noutput_rate=%u\nstart=%u\nsize=%u\nwrite_low=%u\nwrite_high=%u\nread_low=%u\nread_high=%u\ncapacity=%u\ninvalid_marker=%u\nphase_jump=%u\nstereo_mismatch=%u\nfirst_bad=%u\n",stable,header[3],header[11],header[7],start,(unsigned)sizeof data,r[2],r[3],r[6],r[7],r[9],invalid,phase,stereo,first_bad);
 snprintf(path,sizeof path,"/tmp/disc-runtime/dop-window-%s.state",name);save(path,text,n);
}
__attribute__((constructor)) static void probe(void){
 const char *pid=getenv("DISC_PROBE_PID");if(!pid)return;unsigned id=strtoul(pid,0,10);if(!id||id>1000000)return;
 char path[80];snprintf(path,sizeof path,"/proc/%u/mem",id);mem=open(path,0);if(mem<0)return;
 if(get(0x83dfc4,&player,4)&&get(player,header,sizeof header)&&header[4]==2&&header[5]==32&&header[6]==0&&(header[3]==352800||header[3]==176400)){
  ring_snapshot(0x130,"input");ring_snapshot(0x170,"output");
 }
 unsigned sink=0,route=0,pcm=0,fields[8]={0};char status[192];
 if(get(0x83dfe4,&sink,4)&&get(sink+0x58,&route,4)&&route<8&&get(sink+route*0x54+0x74,&pcm,4)&&get(pcm,fields,sizeof fields)){
  int n=snprintf(status,sizeof status,"route=%u\nhandle=%u\nfd=%u\nflags=%u\nrunning_prepared=%u\nunderruns=%u\n",route,pcm,fields[0],fields[1],fields[2],fields[3]);save("/tmp/disc-runtime/dop-pcm-runtime.state",status,n);
 }
 close(mem);
}
