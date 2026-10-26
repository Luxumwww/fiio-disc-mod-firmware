/* WPA config reader/editor. Never sends a passphrase to the normal list. */
#ifdef HOST_TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#else
typedef unsigned size_t;
extern int printf(const char *,...),snprintf(char *,size_t,const char *,...);
extern void *fopen(const char *,const char *);
extern size_t fread(void *,size_t,size_t,void *),fwrite(const void *,size_t,size_t,void *);
extern int fclose(void *),fflush(void *),fileno(void *),fsync(int),atoi(const char *);
extern int strcmp(const char *,const char *),memcmp(const void *,const void *,size_t);
extern void *dlsym(void *,const char *);
#endif
extern int PKCS5_PBKDF2_HMAC_SHA1(const char *,int,const unsigned char *,int,int,int,unsigned char *);
static char buf[131073];
static struct {int a,b,ssid,ssid_end,psk,psk_end,key,key_end;} nets[100];
static int count;
static int space(char c){return c==' '||c=='\t'||c=='\r';}
static int hex(char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;}
static int decode(int a,int b,unsigned char *out,int max){
 int n=0;if(a<=0||b<=a)return 0;
 if(buf[a]=='"'){
  if(buf[b-1]!='"')return -1;
  for(int i=a+1;i<b-1;i++){
   int c=(unsigned char)buf[i];
   if(c=='\\'){if(++i>=b-1)return -1;c=(unsigned char)buf[i];
    if(c=='n')c='\n';else if(c=='r')c='\r';else if(c=='t')c='\t';
    else if(c=='e')c=27;else if(c=='x'){if(i+2>=b-1||hex(buf[i+1])<0||hex(buf[i+2])<0)return -1;c=hex(buf[i+1])*16+hex(buf[i+2]);i+=2;}
   }
   if(n>=max)return -1;out[n++]=c;
  }
 }else{
  if((b-a)&1)return -1;
  for(int i=a;i<b;i+=2){if(hex(buf[i])<0||hex(buf[i+1])<0||n>=max)return -1;out[n++]=hex(buf[i])*16+hex(buf[i+1]);}
 }
 return n;
}
static int parse(int len){
 int current=-1;count=0;
 for(int start=0;start<len;){
  int end=start;while(end<len&&buf[end]!='\n')end++;
  int a=start;while(a<end&&space(buf[a]))a++;
  int b=end,quote=0,escape=0;
  for(int j=a;j<end;j++){char c=buf[j];if(escape){escape=0;continue;}if(c=='\\'&&quote){escape=1;continue;}if(c=='"')quote=!quote;else if(c=='#'&&!quote){b=j;break;}}
  if(quote)return -1;while(b>a&&space(buf[b-1]))b--;
  if(current<0&&b-a>=7&&!memcmp(buf+a,"network",7)){
   int j=a+7;while(j<b&&space(buf[j]))j++;if(j>=b||buf[j++]!='=')return -1;
   while(j<b&&space(buf[j]))j++;if(j+1!=b||buf[j]!='{'||count==100)return -1;
   current=count++;nets[current].a=start;
   nets[current].ssid=nets[current].psk=nets[current].key=0;
  }else if(current>=0){
   if(b-a==1&&buf[a]=='}'){nets[current].b=end<len?end+1:end;current=-1;}
   else if(b>a){
    int j=a;while(j<b&&buf[j]!='='&&!space(buf[j]))j++;int name_end=j;
    while(j<b&&space(buf[j]))j++;if(j>=b||buf[j++]!='=')return -1;while(j<b&&space(buf[j]))j++;
    if(name_end-a==4&&!memcmp(buf+a,"ssid",4)){nets[current].ssid=j;nets[current].ssid_end=b;}
    else if(name_end-a==3&&!memcmp(buf+a,"psk",3)){nets[current].psk=j;nets[current].psk_end=b;}
    else if(name_end-a==8&&!memcmp(buf+a,"key_mgmt",8)){nets[current].key=j;nets[current].key_end=b;}
   }
  }
  start=end<len?end+1:end;
 }
 return current<0?0:-1;
}
static const char *kind(int k){
 if(nets[k].psk)return buf[nets[k].psk]=='"'?"pass":"hash";
 int a=nets[k].key,b=nets[k].key_end;
 return a&&b-a==4&&!memcmp(buf+a,"NONE",4)?"open":"other";
}
/* Independent catalog: the stock player clears its active list on every
 * connection. Keep at most 20 personal/open profiles without modifying that
 * native policy or its UI object indexes. */
typedef struct {unsigned char ssid[32],key[64];int n,key_n,type;} Profile;
static Profile catalog[20],incoming[100];
static int cat_n;
static int load(const char *path,int optional){
 void *f=fopen(path,"rb");if(!f){count=0;buf[0]=0;return optional?0:-1;}
 int len=(int)fread(buf,1,sizeof(buf),f);fclose(f);if(len>131072||parse(len))return -1;return len;
}
static int profile(int k,Profile *p){
 p->n=decode(nets[k].ssid,nets[k].ssid_end,p->ssid,32);if(p->n<1)return -1;
 const char *t=kind(k);p->type=!strcmp(t,"pass")?1:!strcmp(t,"hash")?2:!strcmp(t,"open")?3:0;
 if(!p->type)return -1;
 p->key_n=p->type==3?0:decode(nets[k].psk,nets[k].psk_end,p->key,64);
 if((p->type==1&&(p->key_n<8||p->key_n>63))||(p->type==2&&p->key_n!=32))return -1;
 if(p->type==1)for(int i=0;i<p->key_n;i++)if(p->key[i]<32||p->key[i]==127)return -1;
 return 0;
}
static int read_catalog(const char *path){
 if(load(path,1)<0||count>20)return -1;cat_n=count;
 for(int k=0;k<cat_n;k++)if(profile(k,catalog+k))return -1;return 0;
}
static int same_ssid(const Profile *a,const Profile *b){return a->n==b->n&&!memcmp(a->ssid,b->ssid,a->n);}
static int matches(const Profile *p,const unsigned char *pass,int n){
 if(p->type==1)return n==p->key_n&&!memcmp(pass,p->key,n);
 unsigned char hash[32];
 return p->type==2&&PKCS5_PBKDF2_HMAC_SHA1((const char *)pass,n,p->ssid,p->n,4096,32,hash)==1&&!memcmp(hash,p->key,32);
}
static int put(void *f,const void *p,int n){return fwrite(p,1,n,f)==(size_t)n?0:-1;}
static int puthex(void *f,const unsigned char *p,int n){
 const char *digits="0123456789abcdef";char s[128];for(int i=0;i<n;i++){s[i*2]=digits[p[i]>>4];s[i*2+1]=digits[p[i]&15];}return put(f,s,n*2);
}
static int emit(void *f,const Profile *p){
 int r=put(f,"network={\n ssid=",16);r|=puthex(f,p->ssid,p->n);r|=put(f,"\n",1);
 if(p->type==3)r|=put(f," key_mgmt=NONE\n",15);
 else {
  r|=put(f," psk=",5);
  if(p->type==2)r|=puthex(f,p->key,32);
  else {r|=put(f,"\"",1);for(int i=0;i<p->key_n;i++){if(p->key[i]=='"'||p->key[i]=='\\')r|=put(f,"\\",1);r|=put(f,p->key+i,1);}r|=put(f,"\"",1);}
  r|=put(f,"\n",1);
 }
 return r|put(f,"}\n",2);
}
static int finish(void *f,int r){if(fflush(f)||fsync(fileno(f)))r=-1;if(fclose(f))r=-1;return r?6:0;}
static int save_catalog(const char *path){
 void *f=fopen(path,"wb");if(!f)return 3;
 const char header[]="# DISC saved Wi-Fi catalog v1\n";int r=put(f,header,sizeof(header)-1);
 for(int k=0;k<cat_n;k++)r|=emit(f,catalog+k);return finish(f,r);
}
static int pending(const char *path,Profile *p){
 char raw[200];void *f=fopen(path,"rb");if(!f)return -1;int n=fread(raw,1,sizeof(raw),f);fclose(f);
 int pos=0;p->n=p->key_n=0;p->type=1;
 for(int field=0;field<2;field++){
  unsigned char *out=field?p->key:p->ssid;int max=field?63:32,k=0;
  while(pos+1<n&&raw[pos]!='\n'){int a=hex(raw[pos++]),b=hex(raw[pos++]);if(a<0||b<0||k>=max)return -1;out[k++]=a*16+b;}
  if(pos>=n||raw[pos++]!='\n')return -1;if(field)p->key_n=k;else p->n=k;
 }
 return pos==n&&p->n>0&&p->key_n>=8?0:-1;
}
static int catalog_command(int argc,char **argv){
 if(!strcmp(argv[1],"capture")&&argc==5){
  Profile p;int n=0;while(argv[3][n]&&n<33)n++;if(n<1||n>32)return 4;p.n=n;
  for(int i=0;i<n;i++)p.ssid[i]=argv[3][i];n=0;while(argv[4][n]&&n<64)n++;if(n<8||n>63)return 4;p.key_n=n;
  for(int i=0;i<n;i++){p.key[i]=argv[4][i];if(p.key[i]<32||p.key[i]==127)return 4;}
  void *f=fopen(argv[2],"wb");if(!f)return 3;int r=puthex(f,p.ssid,p.n);r|=put(f,"\n",1);r|=puthex(f,p.key,p.key_n);r|=put(f,"\n",1);return finish(f,r);
 }
 if(!strcmp(argv[1],"merge")&&argc==6){
  if(read_catalog(argv[2]))return 4;
  if(load(argv[3],0)<0)return 4;int n=count;
  for(int k=0;k<n;k++)if(profile(k,incoming+k))return 4;
  Profile secret;int has_secret=!pending(argv[4],&secret);
  for(int k=0;k<n;k++){
   Profile *p=incoming+k;int found=-1;for(int j=0;j<cat_n;j++)if(same_ssid(p,catalog+j)){found=j;break;}
   if(has_secret&&same_ssid(p,&secret)&&matches(p,secret.key,secret.key_n))*p=secret;
   else if(found>=0&&catalog[found].type==1&&matches(p,catalog[found].key,catalog[found].key_n))*p=catalog[found];
   if(found<0){if(cat_n==20)return 7;found=cat_n++;}catalog[found]=*p;
  }
  return save_catalog(argv[5]);
 }
 if(!strcmp(argv[1],"single")&&argc==6){
  if(read_catalog(argv[2]))return 4;int k=atoi(argv[3]);if(k<0||k>=cat_n)return 5;
  int len=load(argv[4],0);if(len<0)return 4;int header=count?nets[0].a:len;
  void *f=fopen(argv[5],"wb");if(!f)return 3;int r=put(f,buf,header);r|=emit(f,catalog+k);return finish(f,r);
 }
 if(!strcmp(argv[1],"verify")&&argc==4){
  Profile secret;if(pending(argv[3],&secret)||load(argv[2],0)<0)return 4;
  for(int k=0;k<count;k++){Profile p;if(!profile(k,&p)&&same_ssid(&p,&secret)&&matches(&p,secret.key,secret.key_n))return 0;}return 5;
 }
 if(!strcmp(argv[1],"prune")&&argc==6){
  if(read_catalog(argv[2]))return 4;int k=atoi(argv[3]);if(k<0||k>=cat_n)return 5;
  int len=load(argv[4],0);if(len<0)return 4;void *f=fopen(argv[5],"wb");if(!f)return 3;
  int r=0,pos=0;for(int j=0;j<count;j++){Profile p;if(!profile(j,&p)&&same_ssid(&p,catalog+k)){r|=put(f,buf+pos,nets[j].a-pos);pos=nets[j].b;}}
  r|=put(f,buf+pos,len-pos);return finish(f,r);
 }
 return 2;
}
static int run(int argc,char **argv,char **env){
 (void)env;if(argc<3)return 2;
 if(!strcmp(argv[1],"capture")||!strcmp(argv[1],"merge")||!strcmp(argv[1],"single")||!strcmp(argv[1],"verify")||!strcmp(argv[1],"prune"))return catalog_command(argc,argv);
 void *f=fopen(argv[2],"rb");if(!f)return 3;
 int len=(int)fread(buf,1,sizeof(buf),f);fclose(f);if(len>131072||parse(len))return 4;
 if(!strcmp(argv[1],"list")){
  for(int k=0;k<count;k++){unsigned char ssid[32];int n=decode(nets[k].ssid,nets[k].ssid_end,ssid,32);if(n<1)return 4;
   printf("%d|",k);for(int j=0;j<n;j++)printf("%02x",ssid[j]);printf("|%s\n",kind(k));}
  return 0;
 }
 if(argc<4)return 2;int k=atoi(argv[3]);if(k<0||k>=count)return 5;
 if(!strcmp(argv[1],"secret")){
  const char *type=kind(k);
  if(!strcmp(type,"hash"))printf("已保存的是密钥摘要，无法还原原密码");
  else if(!strcmp(type,"open"))printf("此网络无需密码");
  else if(!strcmp(type,"other"))printf("此网络的认证方式暂不支持显示密码");
  else {unsigned char p[64];int n=decode(nets[k].psk,nets[k].psk_end,p,64);if(n<0)return 4;
   for(int j=0;j<n;j++)if(p[j]<32||p[j]==127)return 4;
   for(int j=0;j<n;j++)printf("%c",p[j]);}
  printf("\n");return 0;
 }
 if(!strcmp(argv[1],"delete")&&argc==5){
  f=fopen(argv[4],"wb");if(!f)return 3;
  int a=nets[k].a,b=nets[k].b;
  int ok=fwrite(buf,1,a,f)==(size_t)a&&fwrite(buf+b,1,len-b,f)==(size_t)(len-b);
  if(fflush(f)||fsync(fileno(f)))ok=0;if(fclose(f))ok=0;return ok?0:6;
 }
 return 2;
}
#ifdef HOST_TEST
int main(int argc,char **argv){return run(argc,argv,0);}
#else
int __libc_start_main(int (*main_fn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld_fini)(void),void *stack_end){
 (void)main_fn;int (*real_start)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");
 if(!real_start)return 90;
 return real_start(run,argc,argv,init,fini,rtld_fini,stack_end);
}
#endif
