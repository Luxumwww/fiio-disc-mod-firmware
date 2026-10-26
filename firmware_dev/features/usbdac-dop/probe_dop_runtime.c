/* One-shot read-only RAM metadata. No audio bytes, I2C or open of PCM/DAC. */
extern int open(const char *,int,...),close(int),pread(int,void *,unsigned,long),printf(const char *,...),snprintf(char *,unsigned,const char *,...);
extern char *getenv(const char *);extern unsigned long strtoul(const char *,char **,int);
static int mem;static int get(unsigned a,void *b,unsigned n){return a>=0x800000&&a<0x7fffffff-n&&pread(mem,b,n,a)==(int)n;}
__attribute__((constructor)) static void capture(void){
 const char *s=getenv("DISC_PROBE_PID");if(!s)return;unsigned pid=strtoul(s,0,10);char p[64];snprintf(p,sizeof p,"/proc/%u/mem",pid);mem=open(p,0);if(mem<0)return;
 unsigned player=0,sink=0,route=0,pcm=0,h[40]={0},f[52]={0};
 if(get(0x83dfc4,&player,4)&&get(player,h,sizeof h)){
  printf("input=%u output=%u class=%u state=%u\n",h[3],h[7],h[11],h[18]);
  for(unsigned n=0;n<2;n++){unsigned r[10];if(get(player+0x130+n*0x40,r,sizeof r))printf("ring=%u written=%u read=%u capacity=%u unread=%u\n",n,r[2],r[6],r[9],r[2]-r[6]);}
 }
 if(get(0x83dfe4,&sink,4)&&get(sink+0x58,&route,4)&&route==6&&get(sink+route*0x54+0x74,&pcm,4)&&get(pcm,f,sizeof f)){
  printf("pcm=%u fd=%u flags=%u running_prepared=%u underruns=%u buffer=%u\n",pcm,f[0],f[1],f[2],f[3],f[4]);
  for(unsigned i=38;i<52;i++)printf("pcm_offset=0x%x value=%u\n",i*4,f[i]);
 }
 close(mem);
}
