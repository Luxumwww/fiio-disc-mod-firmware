/* Stock mount_sdcard ignores umount failure before rm -rf /tmp/sdcard.
 * Refuse that exact cleanup while the filesystem is still mounted. */
static int tf_root_cleanup(const char *s){
 const char *prefix="rm -rf /tmp/sdcard";unsigned i=0;
 if(!s)return 0;while(prefix[i]){if(s[i]!=prefix[i])return 0;i++;}
 if(s[i]=='/')i++;while(s[i]==' '||s[i]=='\t'||s[i]=='\r'||s[i]=='\n')i++;return !s[i];
}
static int tf_mounted_text(const char *s){
 while(*s){while(*s&&*s!=' '&&*s!='\t'&&*s!='\n')s++;
  while(*s==' '||*s=='\t')s++;const char *p=s;unsigned n=0;while(*s&&*s!=' '&&*s!='\t'&&*s!='\n'){s++;n++;}
  if(n==11&&!memcmp(p,"/tmp/sdcard",11))return 1;
  while(*s&&*s!='\n')s++;if(*s)s++;
 }return 0;
}
static int tf_root_mounted(void){
 char b[8192];int f=open("/proc/mounts",0);if(f<0)return 1;
 int n=read(f,b,sizeof(b)-1);close(f);if(n<0||n==(int)sizeof(b)-1)return 1;b[n]=0;return tf_mounted_text(b);
}
static void *(*tf_original_popen)(const char *,const char *);
static unsigned tf_cleanup_blocked;
static void *tf_safe_popen(const char *command,const char *mode){
 if(tf_root_cleanup(command)&&tf_root_mounted()){
  tf_cleanup_blocked++;char b[120];int n=snprintf(b,sizeof(b),"mounted_root_cleanup_blocked=%u\n",tf_cleanup_blocked);
  int f=open("/tmp/disc-runtime/tf-protection.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}return 0;
 }
 return tf_original_popen?tf_original_popen(command,mode):0;
}
static int tf_guard_install(void){
 const unsigned expected[4]={0x3c0f0084,0x8df9a2d8,0x03200008,0x25f8a2d8};
 if(memcmp((const void *)0x8298a0,expected,sizeof(expected)))return 0;
 tf_original_popen=dlsym((void *)0,"popen");if(!tf_original_popen)return 0;
 *(volatile unsigned *)0x83a2d8=(unsigned)tf_safe_popen;return 1;
}
