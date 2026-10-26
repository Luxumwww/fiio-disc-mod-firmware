#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include "mode_preferences.h"
static char root[128];
static const char *mapped(const char *p,char out[512]){assert(!strncmp(p,"/usr/data/disc-runtime/module-data/",35));const char *n=strrchr(p,'/');snprintf(out,512,"%s/%s",root,n+1);return out;}
static int test_open(const char *p,int flags,...){char b[512];int f=flags&3;if(flags&0x100)f|=O_CREAT;if(flags&0x200)f|=O_TRUNC;return open(mapped(p,b),f,0600);}
static int test_rename(const char *a,const char *b){char x[512],y[512];return rename(mapped(a,x),mapped(b,y));}
static int test_unlink(const char *p){char b[512];return unlink(mapped(p,b));}
static int textread(const char *p,char *b,unsigned size){int f=test_open(p,0);if(f<0)return 0;int n=read(f,b,size-1);close(f);if(n<0)return 0;b[n]=0;return 1;}
#define open test_open
#define rename test_rename
#define unlink test_unlink
#include "../mobile-remote/native_shared_ui.h"
#undef open
#undef rename
#undef unlink
static void put(const char *name,const char *data){char p[512];snprintf(p,sizeof(p),"%s/%s",root,name);FILE *f=fopen(p,"w");assert(f);fputs(data,f);fclose(f);}
int main(void){snprintf(root,sizeof(root),"/tmp/disc-native-store169-XXXXXX");assert(mkdtemp(root));put("settings","288 16 16 32\n");
 DiscChain c;unsigned master;assert(dsp_shared_ui_toggle(13,3));assert(dsp_shared_ui_read(&c,&master)==1&&master&&c.count==2&&c.nodes[0].type==5&&c.nodes[1].type==13);
 c.nodes[1].params[0]=4.7f;assert(dsp_shared_ui_store(&c,master));assert(dsp_shared_ui_toggle(13,0));assert(dsp_shared_ui_read(&c,&master)==1&&!c.nodes[1].enabled&&c.nodes[1].params[0]==4.7f&&c.nodes[0].enabled);
 char before[8192],after[8192];const char *shared="/usr/data/disc-runtime/module-data/disc-remote-control/shared-profile.conf";assert(textread(shared,before,sizeof(before)));
 char lock[512];snprintf(lock,sizeof(lock),"%s/controls.lock",root);int fd=open(lock,O_RDWR);assert(fd>=0&&!flock(fd,LOCK_EX|LOCK_NB));assert(!dsp_shared_ui_toggle(13,0));assert(textread(shared,after,sizeof(after))&&!strcmp(before,after));flock(fd,LOCK_UN);close(fd);
 put("shared-profile.conf","DSP-SHARED-1 2\n1 0\n");assert(!dsp_shared_ui_toggle(13,3));assert(textread(shared,after,sizeof(after))&&!strcmp(after,"DSP-SHARED-1 2\n1 0\n"));test_unlink(shared);
 put("dsp-3.params","broken profile");assert(!dsp_shared_ui_toggle(13,3));assert(!textread(shared,after,sizeof(after)));
 char path[512];const char *names[]={"settings","controls.lock","dsp-3.params"};for(unsigned i=0;i<3;i++){snprintf(path,sizeof(path),"%s/%s",root,names[i]);unlink(path);}assert(!rmdir(root));puts("PASS native store: real locked atomic writes, preserved parameters, legacy seeding, invalid profiles and lock contention refuse without overwrite");return 0;}
