#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
static char config[256],directory[200];
#define LS_DIRECTORY directory
#define LS_CONFIG config
#define LS_O_WRITE_CREATE (O_WRONLY|O_CREAT|O_TRUNC)
static int fail_sync;
static int test_fsync(int fd){return fail_sync?-1:fsync(fd);}
#define fsync test_fsync
#include "scope_core.h"
#undef fsync
static void put(const char *s){FILE *f=fopen(config,"w");assert(f);fputs(s,f);fclose(f);}
int main(void){
 strcpy(directory,"/tmp/disc-scope-core-XXXXXX");assert(mkdtemp(directory));snprintf(config,sizeof config,"%s/config",directory);
 LibraryScope *c=calloc(1,sizeof *c),*d=calloc(1,sizeof *d);assert(c&&d);assert(ls_load(c)&&!c->count);
 char n[LS_PATH];assert(ls_normalize("/tmp//sdcard/a/./b/../音乐 ",n)&&!strcmp(n,"/tmp/sdcard/a/音乐 "));
 assert(!ls_normalize("/../../tmp/sdcard/a",n));assert(!ls_normalize("/tmp/sdcard/a\nb",n));assert(!ls_valid_path("/tmp/sdcard"));assert(!ls_valid_path("/tmp/sdcard/../udisk/a"));assert(!ls_valid_path("/tmp/sdcard-other/a"));assert(!ls_valid_path("/tmp/udiskfoo/a"));assert(ls_valid_path("/tmp/udisk12/音乐"));
 strcpy(c->path[0],"/tmp/sdcard/有声书 a");c->count=1;assert(ls_save(c)&&ls_load(d)&&d->count==1&&!strcmp(c->path[0],d->path[0]));
 assert(ls_excluded(d,"/tmp/sdcard/有声书 a"));assert(ls_excluded(d,"/tmp/sdcard/有声书 a/子目录/song.flac"));assert(!ls_excluded(d,"/tmp/sdcard/有声书 ab"));assert(!ls_excluded(d,"/tmp/sdcard/有声书"));assert(!ls_excluded(d,"/tmp/udisk/有声书 a"));
 strcpy(c->path[1],"/tmp/udisk1/Album");c->count=2;fail_sync=1;assert(!ls_save(c));fail_sync=0;assert(ls_load(d)&&d->count==1);assert(ls_save(c)&&ls_load(d)&&d->count==2);
 put(LS_MAGIC "/tmp/sdcard/x\n/tmp/sdcard/x\n");assert(!ls_load(d)&&!d->count);
 put(LS_MAGIC "/tmp/sdcard\n");assert(!ls_load(d)&&!d->count);
 put(LS_MAGIC "/tmp/sdcard/a");assert(!ls_load(d)&&!d->count);
 put("broken");assert(!ls_load(d)&&!d->count);
 c->count=LS_MAX;for(unsigned i=0;i<LS_MAX;i++)snprintf(c->path[i],LS_PATH,"/tmp/sdcard/目录%u",i);assert(ls_save(c)&&ls_load(d)&&d->count==LS_MAX);c->count++;assert(!ls_save(c));assert(ls_load(d)&&d->count==LS_MAX);
 c->count=0;assert(ls_save(c)&&ls_load(d)&&d->count==0);assert(!ls_excluded(d,"/tmp/sdcard/目录1/a.flac"));
 unlink(config);rmdir(directory);free(c);free(d);puts("PASS scope core: Chinese/spaces, directory boundaries, removable roots, normalization, corruption, 64 folders, persisted removal, failed fsync preserves original");return 0;
}
