/* Native preset list: a projection of the BLE engine's validated store.
 * Applying also works with BLE disabled. No helper process or audio owner. */
extern float strtof(const char *,char **);
extern int flock(int,int),fsync(int),unlink(const char *);
#include "../audio-dsp/profile_format.h"
#include "../audio-dsp/chain_format.h"
static LvObj *dsp_preset_heading,*dsp_preset_rows[24],*dsp_preset_empty;
static char dsp_preset_ids[24][33],dsp_preset_index[4096];
static unsigned dsp_preset_count;
static void dsp_ui_read_masks(void);
static unsigned dsp_ui_index(void);
static void dsp_ui_refresh(void);
static void dsp_presets_forget(void){dsp_preset_heading=dsp_preset_empty=0;dsp_preset_count=0;dsp_preset_index[0]=0;for(unsigned i=0;i<24;i++)dsp_preset_rows[i]=0;}
static int dsp_preset_id(const char *p,unsigned n){if(!n||n>32)return 0;for(unsigned i=0;i<n;i++)if(!((p[i]>='0'&&p[i]<='9')||(p[i]>='a'&&p[i]<='f')))return 0;return 1;}
static int dsp_preset_write(const char *path,const char *data){
 char tmp[256];snprintf(tmp,sizeof(tmp),"%s.new",path);unsigned n=0;while(data[n])n++;
 int f=open(tmp,1|0x100|0x200,0600);if(f<0)return 0;int ok=write(f,data,n)==(int)n&&!fsync(f);close(f);
 if(ok&&!rename(tmp,path))return 1;unlink(tmp);return 0;
}
static int dsp_preset_apply(const char *id,unsigned bank){
 if(bank>3||!dsp_preset_id(id,(unsigned)__builtin_strlen(id)))return 0;
 const char *root="/usr/data/disc-runtime/module-data/disc-remote-control";
 char path[256],buf[8192],old[8192]={0};snprintf(path,sizeof(path),"%s/preset-%s.profile",root,id);
 if(!textread(path,buf,sizeof(buf)))return 0;unsigned mask;float values[12][12];
 int is_chain=!memcmp(buf,"DSP-PRESET-3\n",13);
 unsigned at=0;while(buf[at]&&buf[at]!='\n')at++;if(!buf[at])return 0;
 if(is_chain){DiscChain chain;if(!disc_chain_parse(buf+at+1,&chain))return 0;mask=4096;}
 else{if(sscanf(buf,"DSP-PRESET-2 %u",&mask)!=1||(mask&~4093u))return 0;if(!disc_profile_parse(buf+at+1,values))return 0;}
 snprintf(path,sizeof(path),"%s/controls.lock",root);int lock=open(path,2|0x100,0600);if(lock<0)return 0;
 if(flock(lock,6)){close(lock);return 0;}
 DiscChain shared;unsigned enabled;if(dsp_shared_ui_read(&shared,&enabled)){DiscChain next={0};int ok=1;if(is_chain)ok=disc_chain_parse(buf+at+1,&next);else{const unsigned order[]={2,0,4,3,5,6,7,8,9,10,11};for(unsigned i=0;i<11;i++){unsigned type=order[i];if(!(mask&(1u<<type)))continue;if(next.count>=8){ok=0;break;}DiscChainNode *n=&next.nodes[next.count++];n->id=next.count;n->type=type;n->enabled=1;for(unsigned j=0;j<12;j++)n->params[j]=values[type][j];}}if(ok)ok=dsp_shared_ui_store(&next,1);flock(lock,8);close(lock);return ok;}
 snprintf(path,sizeof(path),is_chain?"%s/chain-%u.conf":"%s/dsp-%u.params",root,bank);int existed=textread(path,old,sizeof(old));
 int ok=dsp_preset_write(path,buf+at+1);
 if(ok){dsp_ui_read_masks();unsigned masks[4];for(unsigned i=0;i<4;i++)masks[i]=dsp_ui_masks[i];masks[bank]=mask;
  char state[96];snprintf(state,sizeof(state),"%u %u %u %u\n",masks[0],masks[1],masks[2],masks[3]);
  ok=dsp_preset_write("/usr/data/disc-runtime/module-data/disc-audio-dsp/settings",state);
  if(!ok){if(existed)dsp_preset_write(path,old);else unlink(path);}
 }
 flock(lock,8);close(lock);return ok;
}
static void dsp_preset_click(void *e){
 if(!control_clicked(e)||!dsp_page)return;LvObj *target=lv_event_target(e);
 for(unsigned i=0;i<dsp_preset_count;i++)if(target==dsp_preset_rows[i]){
  int ok=dsp_preset_apply(dsp_preset_ids[i],dsp_ui_index());dsp_ui_read_masks();dsp_ui_refresh();
  if(dsp_message)lv_text(dsp_message,ok?"预设已应用到当前模式":"预设未应用，请稍后重试");return;
 }
}
static void dsp_presets_refresh(void){
 if(!dsp_page)return;char buf[4096]={0};textread("/usr/data/disc-runtime/module-data/disc-remote-control/presets.index",buf,sizeof(buf));
 if(dsp_preset_heading&&!strcmp(buf,dsp_preset_index))return;
 for(unsigned i=0;i<24;i++)if(dsp_preset_rows[i]&&lv_valid(dsp_preset_rows[i]))FN(0x4f7e98,void (*)(LvObj **))(&dsp_preset_rows[i]);
 if(dsp_preset_empty&&lv_valid(dsp_preset_empty))FN(0x4f7e98,void (*)(LvObj **))(&dsp_preset_empty);
 int top=DSP_ROW_TOP+DSP_ORDER_COUNT*DSP_ROW_PITCH+152;dsp_preset_count=0;
 if(!dsp_preset_heading){dsp_preset_heading=lv_label(dsp_page);stylezero(dsp_preset_heading);smallfont(dsp_preset_heading,font22);lv_color(dsp_preset_heading,0xff54baff,0);lv_text(dsp_preset_heading,"我的预设");lv_clear_flag(dsp_preset_heading,0x40000);lv_align(dsp_preset_heading,2,0,top);}
 unsigned at=buf[0]=='2'&&buf[1]=='\n'?2:0;
 if(at)while(buf[at]&&dsp_preset_count<24){unsigned start=at;while(buf[at]&&buf[at]!='\t'&&buf[at]!='\n')at++;unsigned n=at-start;if(buf[at]!='\t'||!dsp_preset_id(buf+start,n))break;
  unsigned id=dsp_preset_count;for(unsigned j=0;j<n;j++)dsp_preset_ids[id][j]=buf[start+j];dsp_preset_ids[id][n]=0;at++;
  if(buf[at]<'0'||buf[at]>'3'||buf[at+1]!='\t')break;at+=2;start=at;while(buf[at]&&buf[at]!='\n')at++;n=at-start;if(!n||n>128)break;
  char name[129];for(unsigned j=0;j<n;j++)name[j]=buf[start+j];name[n]=0;if(buf[at])at++;
  unsigned count=FN(0x4f889c,unsigned (*)(LvObj *))(dsp_page);int layout[3]={44,244,0};
  FN(0x485ef8,void (*)(LvObj *,const char *,unsigned,void (*)(void *),const int *))(dsp_page,name,id,dsp_preset_click,layout);
  LvObj *row=FN(0x4f8850,LvObj *(*)(LvObj *,int))(dsp_page,count);dsp_preset_rows[id]=row;lv_size(row,360,50);padtop(row,0,0);padbottom(row,0,0);lv_align(row,1,0,top+44+(int)id*54);lv_clear_flag(row,16|0x40000);lv_add_flag(row,0x4000|0x2000);
  lv_add_cb(row,control_touch,1,0);lv_add_cb(row,control_touch,2,0);lv_add_cb(row,control_touch,8,0);dsp_preset_count++;
 }
 if(!dsp_preset_count){dsp_preset_empty=lv_label(dsp_page);stylezero(dsp_preset_empty);smallfont(dsp_preset_empty,font16);lv_color(dsp_preset_empty,0xffaaaaaa,0);lv_clear_flag(dsp_preset_empty,0x40000);lv_size(dsp_preset_empty,290,48);lv_text(dsp_preset_empty,"在手机 App 保存声音预设\n即可在这里使用");lv_align(dsp_preset_empty,2,0,top+44);}
 snprintf(dsp_preset_index,sizeof(dsp_preset_index),"%s",buf);
}
