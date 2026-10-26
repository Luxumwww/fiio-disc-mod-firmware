extern void *opendir(const char *),*readdir(void *);
extern int closedir(void *),mkdir(const char *,unsigned),rmdir(const char *);
extern char *realpath(const char *,char *),*mkdtemp(char *);
#define LS_O_WRITE_CREATE (1|0x100|0x200)
#include "scope_core.h"
#include "scanner_guard.h"
static void *(*ls_real_opendir)(const char *);
static LibraryScope ls_scan_config;
static unsigned ls_scan_skipped;
static int ls_scan_ok;
static char ls_empty[64];
#ifndef LS_STATUS
#define LS_STATUS "/tmp/disc-runtime/library-scope.player"
#endif
static void ls_scanner_status(void){
 char b[180];int n=snprintf(b,sizeof b,"hook=1\nconfig_valid=%d\nexcluded_folders=%u\nskipped_folders=%u\n",ls_scan_ok,ls_scan_config.count,ls_scan_skipped);
 int fd=open(LS_STATUS,LS_O_WRITE_CREATE,0600);if(fd>=0){write(fd,b,n);close(fd);}
}
static __attribute__((noinline)) void *ls_scanner_opendir(const char *path){
 unsigned caller=(unsigned long)__builtin_return_address(0);
 /* The file browser has its own directory walkers. Only this verified call
  * in stock scan_file is eligible, including automatic library updates. */
 if(caller==0x468414){
  char normalized[LS_PATH];if(ls_normalize(path,normalized)){
   unsigned root=ls_root_length(normalized);
   if(root&&!normalized[root]){ls_scan_ok=ls_load(&ls_scan_config);ls_scan_skipped=0;ls_scanner_status();}
   if(root&&ls_scan_ok&&ls_excluded(&ls_scan_config,normalized)){
    if(!ls_empty[0]){char temp[]="/tmp/disc-library-empty-XXXXXX";if(mkdtemp(temp))memcpy(ls_empty,temp,sizeof temp);}
    if(ls_empty[0]){void *dir=ls_real_opendir(ls_empty);if(dir){ls_scan_skipped++;ls_scanner_status();return dir;}}
    /* Failure to allocate an empty directory does not hide unrelated songs. */
   }
  }
 }
 return ls_real_opendir(path);
}
static void library_scope_player_install(void){
 for(unsigned i=0;i<sizeof(ls_scanner_guards)/sizeof(ls_scanner_guards[0]);i++)if(memcmp((void *)ls_scanner_guards[i].address,ls_scanner_guards[i].bytes,16))return;
 ls_real_opendir=dlsym(0,"opendir");if(!ls_real_opendir)return;
 unsigned before=*(unsigned *)LS_OPENDIR_GOT;
 if(before!=(unsigned long)ls_real_opendir&&before!=LS_OPENDIR_UNRESOLVED)return;
 ls_scan_ok=ls_load(&ls_scan_config);*(unsigned *)LS_OPENDIR_GOT=(unsigned long)ls_scanner_opendir;ls_scanner_status();
}
static __attribute__((destructor)) void ls_empty_cleanup(void){if(ls_empty[0])rmdir(ls_empty);}
