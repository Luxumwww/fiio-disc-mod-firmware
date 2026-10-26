/* Shared by the UI editor and the scanner. No audio or database operations. */
#define LS_MAX 64
#define LS_PATH 1024
#define LS_FILE_MAX (LS_MAX*LS_PATH+64)
#ifndef LS_CONFIG
#define LS_DIRECTORY "/usr/data/disc-runtime/module-data/disc-library-scope"
#define LS_CONFIG LS_DIRECTORY "/excluded-folders.conf"
#endif
#define LS_MAGIC "DISC_LIBRARY_EXCLUSIONS_V1\n"
extern int *__errno_location(void);
typedef struct {unsigned count;char path[LS_MAX][LS_PATH];} LibraryScope;
static unsigned ls_len(const char *s){unsigned n=0;while(s&&s[n])n++;return n;}
static int ls_prefix(const char *p,const char *base){unsigned n=ls_len(base);return !memcmp(p,base,n)&&(p[n]==0||p[n]=='/');}
/* Persist only known removable-storage paths; never permit filesystem root. */
static unsigned ls_root_length(const char *p){
 if(ls_len(p)>=11&&ls_prefix(p,"/tmp/sdcard"))return 11;
 if(ls_len(p)<10||memcmp(p,"/tmp/udisk",10))return 0;
 unsigned n=10;while(p[n]>='0'&&p[n]<='9')n++;
 return !p[n]||p[n]=='/'?n:0;
}
static int ls_normalize(const char *p,char out[LS_PATH]){
 unsigned used=0,i=0;if(!p||p[0]!='/')return 0;
 while(p[i]){
  while(p[i]=='/')i++;if(!p[i])break;
  unsigned start=i;while(p[i]&&p[i]!='/'){if((unsigned char)p[i]<32||(unsigned char)p[i]==127)return 0;i++;if(i>=LS_PATH)return 0;}
  unsigned n=i-start;if(n==1&&p[start]=='.')continue;
  if(n==2&&p[start]=='.'&&p[start+1]=='.'){if(!used)return 0;while(used&&out[used-1]!='/')used--;if(used)used--;continue;}
  if(used+n+1>=LS_PATH)return 0;out[used++]='/';for(unsigned k=0;k<n;k++)out[used++]=p[start+k];
 }
 if(!used)out[used++]='/';out[used]=0;return 1;
}
static int ls_valid_path(const char *p){char normalized[LS_PATH];unsigned root=ls_root_length(p);return root&&p[root]=='/'&&p[root+1]&&ls_normalize(p,normalized)&&!strcmp(p,normalized);}
static int ls_excluded(const LibraryScope *c,const char *p){
 unsigned n=ls_len(p);for(unsigned i=0;i<c->count;i++){unsigned b=ls_len(c->path[i]);if(n>=b&&ls_prefix(p,c->path[i]))return 1;}return 0;
}
static int ls_exact(const LibraryScope *c,const char *p){for(unsigned i=0;i<c->count;i++)if(!strcmp(c->path[i],p))return (int)i;return -1;}
/* Missing is the normal initial empty configuration. Malformed files fail open
 * for scanning and remain visible as an error in the editor, never overwritten. */
static int ls_load(LibraryScope *out){
 out->count=0;int fd=open(LS_CONFIG,0);if(fd<0)return *__errno_location()==2;
 char *b=malloc(LS_FILE_MAX+1);if(!b){close(fd);return 0;}unsigned used=0;int n=0,ok=1;
 while(used<LS_FILE_MAX&&(n=read(fd,b+used,LS_FILE_MAX-used))>0)used+=(unsigned)n;
 char extra;if(n<0||read(fd,&extra,1)!=0)ok=0;close(fd);
 unsigned magic=sizeof(LS_MAGIC)-1;if(used<magic||memcmp(b,LS_MAGIC,magic))ok=0;
 unsigned i=magic;while(ok&&i<used){unsigned start=i;while(i<used&&b[i]!='\n'&&b[i])i++;
  unsigned length=i-start;if(i>=used||b[i]!='\n'||!length||length>=LS_PATH||out->count>=LS_MAX){ok=0;break;}
  b[i++]=0;if(!ls_valid_path(b+start)||ls_exact(out,b+start)>=0){ok=0;break;}
  memcpy(out->path[out->count++],b+start,length+1);
 }
 free(b);if(!ok)out->count=0;return ok;
}
static int ls_write_all(int fd,const char *p,unsigned n){while(n){int r=write(fd,p,n);if(r<=0)return 0;p+=r;n-=(unsigned)r;}return 1;}
static int ls_save(const LibraryScope *c){
 if(c->count>LS_MAX)return 0;for(unsigned i=0;i<c->count;i++)if(!ls_valid_path(c->path[i]))return 0;
 mkdir(LS_DIRECTORY,0700);char temp[256];int n=snprintf(temp,sizeof temp,"%s.new.%d",LS_CONFIG,getpid());if(n<0||n>=(int)sizeof temp)return 0;
 int fd=open(temp,LS_O_WRITE_CREATE,0600);if(fd<0)return 0;
 int ok=ls_write_all(fd,LS_MAGIC,sizeof(LS_MAGIC)-1);
 for(unsigned i=0;ok&&i<c->count;i++)ok=ls_write_all(fd,c->path[i],ls_len(c->path[i]))&&ls_write_all(fd,"\n",1);
 if(ok)ok=!fsync(fd);if(close(fd))ok=0;
 if(ok)ok=!rename(temp,LS_CONFIG);if(!ok)unlink(temp);
 if(ok){fd=open(LS_DIRECTORY,0);if(fd>=0){fsync(fd);close(fd);}}
 return ok;
}
