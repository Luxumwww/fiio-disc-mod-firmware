extern float strtof(const char *,char **);
extern int flock(int,int),fsync(int),unlink(const char *);
#include "../audio-dsp/shared_profile.h"
#include "../audio-dsp/shared_toggle.h"
#include "../audio-dsp/profile_format.h"
static unsigned dsp_shared_ui_error;
static int dsp_shared_ui_read(DiscChain *chain,unsigned *enabled){char text[8192];if(!textread("/usr/data/disc-runtime/module-data/disc-remote-control/shared-profile.conf",text,sizeof(text)))return 0;return disc_shared_parse(text,chain,enabled)?1:-1;}
static int dsp_shared_ui_store(const DiscChain *chain,unsigned enabled){char text[8192];int n=disc_shared_format(chain,enabled,text,sizeof(text));if(!n)return 0;const char *path="/usr/data/disc-runtime/module-data/disc-remote-control/shared-profile.conf",*tmp="/usr/data/disc-runtime/module-data/disc-remote-control/shared-profile.conf.ui-new";int f=open(tmp,1|0x100|0x200,0600);if(f<0)return 0;int ok=write(f,text,n)==n&&!fsync(f);close(f);if(ok&&!rename(tmp,path))return 1;unlink(tmp);return 0;}
static int dsp_shared_ui_seed(DiscChain *c,unsigned *enabled,unsigned bank){
 if(bank>3)return 0;char text[8192],path[160];unsigned masks[4];
 text[0]=0;textread("/usr/data/disc-runtime/module-data/disc-audio-dsp/settings",text,sizeof(text));disc_dsp_read_masks(text,masks);
 *enabled=masks[bank]!=0;c->count=0;
 if(masks[bank]&DISC_DSP_CUSTOM){snprintf(path,sizeof(path),"/usr/data/disc-runtime/module-data/disc-remote-control/chain-%u.conf",bank);return textread(path,text,sizeof(text))&&disc_chain_parse(text,c);}
 float values[12][12];for(unsigned i=0;i<12;i++)for(unsigned j=0;j<12;j++)values[i][j]=disc_param_default[i][j];
 snprintf(path,sizeof(path),"/usr/data/disc-runtime/module-data/disc-remote-control/dsp-%u.params",bank);
 if(textread(path,text,sizeof(text))&&!disc_profile_parse(text,values))return 0;
 const unsigned order[]={2,0,4,3,5,6,7,8,9,10,11};
 for(unsigned i=0;i<11;i++){unsigned t=order[i];if(!(masks[bank]&(1u<<t)))continue;if(c->count>=DISC_CHAIN_MAX)return 0;DiscChainNode *n=&c->nodes[c->count++];n->id=c->count;n->type=t;n->enabled=1;for(unsigned j=0;j<12;j++)n->params[j]=values[t][j];}return disc_chain_valid(c);
}
static int dsp_shared_ui_toggle(unsigned type,unsigned bank){
 dsp_shared_ui_error=1;
 int lock=open("/usr/data/disc-runtime/module-data/disc-remote-control/controls.lock",2|0x100,0600);if(lock<0)return 0;if(flock(lock,6)){close(lock);return 0;}
 DiscChain c;unsigned enabled;int ok=0,status=dsp_shared_ui_read(&c,&enabled);
 if(status==0&&(type==DISC_HAAS_TYPE||type==DISC_FFHAAS_TYPE||type==DISC_STEREOFX_TYPE||type==DISC_WARMGLOSS_TYPE))status=dsp_shared_ui_seed(&c,&enabled,bank)?1:-1;
 if(status==1){dsp_shared_ui_error=disc_shared_toggle_error(&c,enabled,type);ok=!dsp_shared_ui_error&&disc_shared_toggle_type(&c,&enabled,type);
  if(ok){ok=dsp_shared_ui_store(&c,enabled);dsp_shared_ui_error=ok?0:1;}
 }if(!ok&&!dsp_shared_ui_error)dsp_shared_ui_error=1;flock(lock,8);close(lock);return ok;
}
