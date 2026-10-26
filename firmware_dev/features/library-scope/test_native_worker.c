/* Run only the original traversal worker in an isolated executable copy.
 * Logical /tmp/sdcard is redirected to RAM fixtures for opendir/access. The
 * original full scan entry, parser, database, PCM and player main never run. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),read(int,void *,unsigned),write(int,const void *,unsigned),access(const char *,int),rename(const char *,const char *),unlink(const char *),fsync(int),getpid(void),snprintf(char *,unsigned,const char *,...),printf(const char *,...),memcmp(const void *,const void *,unsigned),strcmp(const char *,const char *),pthread_mutex_init(void *,void *),pthread_mutex_destroy(void *);
extern void *dlsym(void *,const char *),*malloc(unsigned),*memcpy(void *,const void *,unsigned),*calloc(unsigned,unsigned);
extern void free(void *);
static char directory[200],config[256];
#define LS_DIRECTORY directory
#define LS_CONFIG config
#define LS_STATUS "/tmp/disc-library-worker-probe.state"
#include "scanner_hook.h"
#include "test_worker_guard.h"
#define CHECK(x) do{if(!(x)){printf("FAIL line %d: %s\n",__LINE__,#x);return 2;}}while(0)
static void *(*native_opendir)(const char *);
static int (*native_access)(const char *,int);
static const char *map_path(const char *p,char b[2048]){if(ls_len(p)>=11&&ls_prefix(p,"/tmp/sdcard")){snprintf(b,2048,"%s/tree%s",directory,p+11);return b;}return p;}
static void *mapped_opendir(const char *p){char b[2048];return native_opendir(map_path(p,b));}
static int mapped_access(const char *p,int mode){char b[2048];return native_access(map_path(p,b),mode);}
static int fixture(const char *p,int isdir){char logical[1024],b[2048];snprintf(logical,sizeof logical,"/tmp/sdcard%s",p);map_path(logical,b);if(isdir)return mkdir(b,0700);int fd=open(b,1|0x100|0x200,0600);if(fd<0)return -1;return close(fd);}
static unsigned count_worker(void){
 unsigned *c=calloc(1,0xa0);if(!c)return 999;c[0]=(unsigned)"/tmp/sdcard";
 for(unsigned i=0x1c;i<=0x24;i+=2)c[i+1]=(unsigned)(c+i);
 pthread_mutex_init(c+8,0);pthread_mutex_init(c+0xe,0);pthread_mutex_init(c+0x14,0);
 ((void *(*)(void *))0x46830c)(c);unsigned count=c[4];
 for(unsigned i=0x1c;i<=0x24;i+=2){unsigned *p=(unsigned *)c[i];while(p){unsigned *next=(unsigned *)p[14];if(i==0x1e)printf("queued: %s\n",(char *)p[1]);free((void *)p[1]);free(p);p=next;}}
 pthread_mutex_destroy(c+8);pthread_mutex_destroy(c+0xe);pthread_mutex_destroy(c+0x14);free(c);return count;
}
static int run(int argc,char **argv,char **env){
 (void)argc;(void)argv;(void)env;
 snprintf(directory,sizeof directory,"/tmp/disc-library-worker-%d",getpid());CHECK(!mkdir(directory,0700));snprintf(config,sizeof config,"%s/config",directory);
 native_opendir=dlsym(0,"opendir");native_access=dlsym(0,"access");CHECK(native_opendir&&native_access);
 printf("opendir GOT=%x real=%x\n",*(unsigned *)LS_OPENDIR_GOT,(unsigned)native_opendir);
 for(unsigned i=0;i<sizeof(ls_scanner_guards)/sizeof(ls_scanner_guards[0]);i++)printf("guard %x equal=%d\n",ls_scanner_guards[i].address,!memcmp((void *)ls_scanner_guards[i].address,ls_scanner_guards[i].bytes,16));
 library_scope_player_install();CHECK(*(unsigned *)LS_OPENDIR_GOT==(unsigned)ls_scanner_opendir);
 ls_real_opendir=mapped_opendir;*(unsigned *)LS_ACCESS_GOT=(unsigned)mapped_access;
 CHECK(!fixture("",1));CHECK(!fixture("/音乐 空格",1));CHECK(!fixture("/音乐 空格/sub",1));CHECK(!fixture("/音乐 空格2",1));
 const char *files[]={"/root.flac","/音乐 空格/a.flac","/音乐 空格/sub/b.mp3","/音乐 空格2/c.flac"};for(unsigned i=0;i<4;i++)CHECK(!fixture(files[i],0));
 LibraryScope *c=calloc(1,sizeof *c);CHECK(c&&ls_save(c));CHECK(count_worker()==4);
 c->count=1;snprintf(c->path[0],LS_PATH,"/tmp/sdcard/音乐 空格");CHECK(ls_save(c));CHECK(count_worker()==2&&ls_scan_skipped==1);
 /* Non-scanner caller must still see the excluded directory and its song. */
 void *d=ls_scanner_opendir("/tmp/sdcard/音乐 空格");CHECK(d);int found=0;unsigned char *entry;while((entry=readdir(d)))if(!strcmp((char *)entry+11,"a.flac"))found=1;closedir(d);CHECK(found);
 c->count=0;CHECK(ls_save(c));CHECK(count_worker()==4);
 int fd=open(config,1|0x200);CHECK(fd>=0);CHECK(write(fd,"broken",6)==6);close(fd);CHECK(count_worker()==4&&!ls_scan_ok);
 free(c);unlink(config);
 for(unsigned i=0;i<4;i++){char p[1024],b[2048];snprintf(p,sizeof p,"/tmp/sdcard%s",files[i]);unlink(map_path(p,b));}
 const char *dirs[]={"/音乐 空格/sub","/音乐 空格","/音乐 空格2",""};for(unsigned i=0;i<4;i++){char p[1024],b[2048];snprintf(p,sizeof p,"/tmp/sdcard%s",dirs[i]);rmdir(map_path(p,b));}rmdir(directory);
 printf("PASS original native scan_file: empty=4, excluded parent and descendants=2, similar name retained, manual opendir sees excluded song, removal=4, malformed config fail-open=4; no database/PCM/player main\n");return 0;
}
int __libc_start_main(int (*fn)(int,char **,char **),int argc,char **argv,void (*init)(void),void (*fini)(void),void (*rtld)(void),void *stack){
 (void)fn;int (*next)(int (*)(int,char **,char **),int,char **,void (*)(void),void (*)(void),void (*)(void),void *)=dlsym((void *)-1,"__libc_start_main");return next?next(run,argc,argv,init,fini,rtld,stack):90;
}
