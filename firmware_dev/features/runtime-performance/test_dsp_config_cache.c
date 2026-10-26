#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "../audio-dsp/mode_preferences.h"
#include "../audio-dsp/profile_format.h"
#include "../audio-dsp/shared_profile.h"

static unsigned dsp_enabled=1,dsp_shared_scope,dsp_shared_enabled,dsp_masks[4];
static float dsp_params[4][12][12];
static DiscChain dsp_chains[4];
static unsigned dsp_chain_valid[4],dsp_chain_hashes[4];
static char fixture_root[160];
static unsigned opens,shared_opens,legacy_opens,parsed_shared,parsed_settings,parsed_params,parsed_chain;
static int fail_shared_read,shared_fd=-1,failed_fd=-1;
static const char *fail_read_name;
static int fixture_open(const char *path,int flags){
 const char *name=strrchr(path,'/');char full[512];assert(name);
 snprintf(full,sizeof(full),"%s%s",fixture_root,name);opens++;
 int is_shared=!strcmp(name,"/shared-profile.conf");shared_opens+=is_shared;legacy_opens+=!is_shared;
 int f=open(full,flags);if(is_shared)shared_fd=f;failed_fd=fail_read_name&&!strcmp(name+1,fail_read_name)?f:-1;return f;
}
static ssize_t fixture_read(int fd,void *buf,size_t n){if((fd==shared_fd&&fail_shared_read)||fd==failed_fd)return -1;return read(fd,buf,n);}
#define open fixture_open
#define read fixture_read
#define DSP_CONFIG_PARSED(kind) (parsed_##kind++)
#include "../audio-dsp/config_cache.h"
#undef open
#undef read
static void fixture_write(const char *name,const char *text){
 char tmp[512],path[512];snprintf(tmp,sizeof(tmp),"%s/replacement.new",fixture_root);snprintf(path,sizeof(path),"%s/%s",fixture_root,name);
 int f=open(tmp,O_WRONLY|O_CREAT|O_TRUNC,0600);assert(f>=0);assert(write(f,text,strlen(text))==(ssize_t)strlen(text));assert(!close(f));assert(!rename(tmp,path));
}
static void fixture_unlink(const char *name){char path[512];snprintf(path,sizeof(path),"%s/%s",fixture_root,name);assert(!unlink(path));}
static void counters_reset(void){opens=shared_opens=legacy_opens=parsed_shared=parsed_settings=parsed_params=parsed_chain=0;}
static unsigned parses(void){return parsed_shared+parsed_settings+parsed_params+parsed_chain;}
static DiscChain chain(unsigned id){DiscChain c={0};c.count=1;c.nodes[0].id=id;c.nodes[0].type=0;c.nodes[0].enabled=1;memcpy(c.nodes[0].params,disc_param_default[0],sizeof(c.nodes[0].params));return c;}
static void shared(unsigned id,unsigned enabled){char text[8192];DiscChain c=chain(id);assert(disc_shared_format(&c,enabled,text,sizeof(text)));fixture_write("shared-profile.conf",text);}
static void legacy(unsigned mode){
 char name[80],text[8192];DiscChain c=chain(10+mode);int n=snprintf(text,sizeof(text),"1 1\n%u 0 1",10+mode);
 for(unsigned j=0;j<12;j++)n+=snprintf(text+n,sizeof(text)-(unsigned)n," %.6g",(double)c.nodes[0].params[j]);snprintf(text+n,sizeof(text)-(unsigned)n,"\n");
 snprintf(name,sizeof(name),"chain-%u.conf",mode);fixture_write(name,text);
 n=snprintf(text,sizeof(text),"2\n");for(unsigned i=0;i<12;i++)for(unsigned j=0;j<12;j++)n+=snprintf(text+n,sizeof(text)-(unsigned)n," %.9g",(double)(i==0&&j==0?1.f+.1f*mode:disc_param_default[i][j]));
 snprintf(name,sizeof(name),"dsp-%u.params",mode);fixture_write(name,text);
}
static void assert_shared(unsigned id,unsigned enabled){assert(dsp_shared_scope&&dsp_shared_enabled==enabled);for(unsigned i=0;i<4;i++)assert(dsp_chain_valid[i]&&dsp_chains[i].nodes[0].id==id);}
int main(void){
 strcpy(fixture_root,"/tmp/disc-dsp-config-XXXXXX");assert(mkdtemp(fixture_root));
 fixture_write("settings","4096 4096 4096 4096\n");for(unsigned i=0;i<4;i++)legacy(i);
 shared(99,1);dsp_config_poll();assert_shared(99,1);assert(opens==1&&legacy_opens==0&&parsed_shared==1);
 counters_reset();for(unsigned i=0;i<10;i++)dsp_config_poll();assert(opens==10&&legacy_opens==0&&!parses());
 printf("shared steady 10 ticks: old 110 opens; new %u opens/0 parses including FEATURES\n",opens+10);
 shared(99,1);dsp_config_poll();assert_shared(99,1);assert(!parses()); /* equal bytes, new inode */
 shared(98,1);dsp_config_poll();assert_shared(98,1);assert(parsed_shared==1); /* same size */
 shared(98,0);dsp_config_poll();assert_shared(98,0);assert(parsed_shared==2);
 shared(97,0);dsp_config_poll();assert_shared(97,0);assert(parsed_shared==3); /* edits while master off */
 fixture_write("shared-profile.conf","DSP-SHARED-1 0\nbroken\n");dsp_config_poll();assert(dsp_shared_scope&&dsp_shared_enabled==1);for(unsigned i=0;i<4;i++)assert(!dsp_chain_valid[i]);assert(!legacy_opens);
 unsigned before=parses();dsp_config_poll();assert(parses()==before); /* invalid content cached too */
 shared(97,1);dsp_config_poll();assert_shared(97,1);
 fail_shared_read=1;dsp_config_poll();assert(dsp_shared_scope&&dsp_shared_enabled==1);for(unsigned i=0;i<4;i++)assert(!dsp_chain_valid[i]);assert(!legacy_opens);
 fail_shared_read=0;dsp_config_poll();assert_shared(97,1);
 dsp_enabled=0;counters_reset();shared(96,0);for(unsigned i=0;i<10;i++)dsp_config_poll();assert(!opens&&!parses());
 printf("component disabled 10 ticks: old 110 opens; new 10 FEATURES opens/0 parses\n");
 dsp_enabled=1;dsp_config_poll();assert_shared(96,0);
 fixture_unlink("shared-profile.conf");counters_reset();dsp_config_poll();assert(!dsp_shared_scope&&!dsp_shared_enabled);assert(opens==10&&parsed_settings==1&&parsed_params==4&&parsed_chain==4);
 for(unsigned i=0;i<4;i++){assert(dsp_masks[i]==4096&&dsp_chain_valid[i]&&dsp_chains[i].nodes[0].id==10+i);assert(fabsf(dsp_params[i][0][0]-(1.f+.1f*i))<.00001f);}
 counters_reset();dsp_config_poll();assert(opens==10&&!parses());
 printf("legacy steady tick: old 11 opens; new %u opens/0 parses including FEATURES\n",opens+1);
 shared(88,1);dsp_config_poll();assert_shared(88,1);fixture_write("settings","0 0 0 0\n");fixture_unlink("shared-profile.conf");dsp_config_poll();assert(!dsp_shared_scope);for(unsigned i=0;i<4;i++)assert(!dsp_masks[i]);
 counters_reset();for(unsigned i=0;i<10;i++)dsp_config_poll();assert(opens==20&&!parses());
 printf("legacy master off 10 ticks: old 110 opens; new %u opens/0 parses including FEATURES\n",opens+10);
 fixture_write("settings","4096 4096 4096 4096\n");dsp_config_poll();for(unsigned i=0;i<4;i++)assert(dsp_chains[i].nodes[0].id==10+i); /* cached legacy must replace shared */
 fixture_write("chain-0.conf","broken\n");fixture_write("dsp-1.params","broken\n");dsp_config_poll();assert(!dsp_chain_valid[0]);assert(!memcmp(dsp_params[1],disc_param_default,sizeof(dsp_params[1])));
 legacy(0);legacy(1);dsp_config_poll();assert(dsp_chain_valid[0]);assert(dsp_params[1][0][0]>1.f);
 fixture_unlink("chain-2.conf");dsp_config_poll();assert(!dsp_chain_valid[2]&&!dsp_chain_hashes[2]);legacy(2);dsp_config_poll();assert(dsp_chain_valid[2]);
 fail_read_name="dsp-3.params";dsp_config_poll();assert(!memcmp(dsp_params[3],disc_param_default,sizeof(dsp_params[3])));fail_read_name=0;dsp_config_poll();assert(dsp_params[3][0][0]>1.f);
 fail_read_name="chain-1.conf";dsp_config_poll();assert(!dsp_chain_valid[1]&&!dsp_chain_hashes[1]);fail_read_name=0;dsp_config_poll();assert(dsp_chain_valid[1]);
 fixture_write("settings","broken\n");dsp_config_poll();for(unsigned i=0;i<4;i++)assert(!dsp_masks[i]);fixture_write("settings","4096 4096 4096 4096\n");dsp_config_poll();for(unsigned i=0;i<4;i++)assert(dsp_masks[i]==4096);
 /* All generated legacy data survives; this test deletes only its own files. */
 fixture_unlink("settings");for(unsigned i=0;i<4;i++){char name[80];snprintf(name,sizeof(name),"chain-%u.conf",i);fixture_unlink(name);snprintf(name,sizeof(name),"dsp-%u.params",i);fixture_unlink(name);}assert(!rmdir(fixture_root));
 puts("PASS: shared precedence, exact-byte caching, atomic replacement, same-size edits, master changes, corruption, read failure, disable/re-enable, all 4 legacy modes, delete/restore");return 0;
}
