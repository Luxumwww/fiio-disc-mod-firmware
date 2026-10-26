/* Factory list/choice constructors. DSP parent remains alive at its scroll
 * position and retains ownership of its original status-bar state. */
#include "rules.h"
static LvObj *src_pages[2],*src_owner,*src_entry,*src_rule_labels[7],*src_status,*src_choice_status;
static LvObj *src_choices[4];static unsigned src_depth,src_view,src_rule,src_action,src_argument,src_updated;
static DiscSrcRules src_rules;
static int src_bad_config;
enum {SRC_UI_OPEN=1,SRC_UI_ENABLE,SRC_UI_RULE,SRC_UI_VALUE,SRC_UI_BACK};
static void src_ui_make(unsigned);
static void src_ui_close(int restore);
static int src_ui_active(void){LvObj *active=lv_active_screen(lv_disp_default());return (src_pages[0]&&active==src_pages[0])||(src_pages[1]&&active==src_pages[1]);}
static void src_ui_load(void){char b[128];src_rules=disc_src_default();src_bad_config=0;if(textread(DISC_SRC_CONFIG,b,sizeof(b))){DiscSrcRules r;if(disc_src_parse(b,&r))src_rules=r;else src_bad_config=1;}else if(!access(DISC_SRC_CONFIG,0))src_bad_config=1;}
static int src_ui_save(const DiscSrcRules *r){
 if(!disc_src_rules_valid(r))return 0;char b[96];int n=snprintf(b,sizeof(b),"1 %u %u %u %u %u %u %u\n",r->enabled,r->shift[0],r->shift[1],r->shift[2],r->shift[3],r->shift[4],r->shift[5]);
 int f=open(DISC_SRC_CONFIG ".new",1|0x100|0x200,0600),ok=0;if(f>=0){ok=write(f,b,n)==n&&!fsync(f);if(close(f))ok=0;if(ok)ok=!rename(DISC_SRC_CONFIG ".new",DISC_SRC_CONFIG);}if(!ok)unlink(DISC_SRC_CONFIG ".new");src_ui_load();return ok;
}
static void src_ui_rate(char *b,unsigned size,unsigned rate){if(rate%1000)snprintf(b,size,"%u.%u",rate/1000,rate%1000/100);else snprintf(b,size,"%u",rate/1000);}
static void src_ui_deleted(void *e){LvObj *o=lv_event_target(e);if(o==src_entry)src_entry=0;if(o==src_owner)src_owner=0;for(unsigned i=0;i<2;i++)if(o==src_pages[i])src_pages[i]=0;}
static void src_ui_touch(void *e){control_touch(e);}
static void src_ui_click(void *e){if(!control_clicked(e))return;unsigned action=(unsigned long)FN(0x4e394c,void *(*)(void *))(e);src_action=action&255;src_argument=action>>8;}
static void src_ui_back(void *e){if(FN(0x488860,int (*)(void *))(e)==3)src_action=SRC_UI_BACK;}
static void src_ui_protect(LvObj *o){if(!o)return;const unsigned events[]={1,2,8,3};for(unsigned i=0;i<4;i++)lv_add_cb(o,src_ui_touch,events[i],0);}
static LvObj *src_ui_row(LvObj *p,const char *text,unsigned action,unsigned arg){unsigned n=FN(0x4f889c,unsigned (*)(LvObj *))(p);FN(0x485ef8,void (*)(LvObj *,const char *,unsigned,void (*)(void *),const int *))(p,text,action|(arg<<8),src_ui_click,0);LvObj *row=FN(0x4f8850,LvObj *(*)(LvObj *,int))(p,(int)n);src_ui_protect(row);return row;}
static LvObj *src_ui_info(LvObj *p,const char *text){LvObj *row=src_ui_row(p,text,0,0);if(!row)return 0;lv_clear_flag(row,2);show(FN(0x4f8850,LvObj *(*)(LvObj *,int))(row,1),0);return FN(0x4f8850,LvObj *(*)(LvObj *,int))(row,0);}
static void src_ui_refresh(void){
 if(!src_pages[0])return;if(src_rule_labels[0])lv_text(src_rule_labels[0],src_rules.enabled?"启用状态：开启":"启用状态：关闭");
 for(unsigned i=0;i<6;i++)if(src_rule_labels[i+1]){char from[24],to[24],b[70];src_ui_rate(from,sizeof(from),disc_src_rates[i]);src_ui_rate(to,sizeof(to),disc_src_rates[i]>>src_rules.shift[i]);if(src_rules.shift[i])snprintf(b,sizeof(b),"%s → %s kHz",from,to);else snprintf(b,sizeof(b),"%s kHz：保持原始",from);lv_text(src_rule_labels[i+1],b);}
}
static LvObj *src_ui_screen(const char *title){
 LvObj *p=lv_create(0);if(!p)return 0;FN(0x486620,void (*)(LvObj *))(p);stylezero(p);lv_size(p,360,360);lv_bg(p,0xff000000,0);opa(p,255,0);lv_add_flag(p,2|16);FN(0x4f6b30,void (*)(LvObj *,int,unsigned))(p,5,0);FN(0x4f0268,void (*)(LvObj *,int))(p,0);FN(0x4f02c8,void (*)(LvObj *,int))(p,12);FN(0x5030a0,void (*)(LvObj *,int))(p,1);FN(0x5030e4,void (*)(LvObj *,int,int,int))(p,0,0,2);
 FN(0x46d88c,void (*)(LvObj *,const char *,unsigned))(p,title,0xb4fb);lv_add_cb(p,src_ui_deleted,33,0);const unsigned ev[]={1,2,8};for(unsigned i=0;i<3;i++)lv_add_cb(p,src_ui_back,ev[i],0);return p;
}
static void src_ui_make(unsigned view){
 char title[48];if(view==0)snprintf(title,sizeof title,"DSP 降采样");else if(view==1)snprintf(title,sizeof title,"启用降采样");else {char rate[24];src_ui_rate(rate,sizeof rate,disc_src_rates[src_rule]);snprintf(title,sizeof title,"%s kHz 音频",rate);}
 LvObj *p=src_ui_screen(title);if(!p)return;src_view=view;src_depth=view?1:0;src_pages[src_depth]=p;
 for(unsigned i=0;i<4;i++)src_choices[i]=0;
 src_choice_status=0;
 if(!view){
  LvObj *row=src_ui_row(p,"",SRC_UI_ENABLE,0);src_rule_labels[0]=row?FN(0x4f8850,LvObj *(*)(LvObj *,int))(row,0):0;
  for(unsigned i=0;i<6;i++){row=src_ui_row(p,"",SRC_UI_RULE,i);src_rule_labels[i+1]=row?FN(0x4f8850,LvObj *(*)(LvObj *,int))(row,0):0;}
  src_ui_info(p,"仅降低 DSP 内部采样率");src_ui_info(p,"保持原输出采样率");src_ui_info(p,"高品质滤波 · 自动保存");src_status=src_ui_info(p,src_bad_config?"配置无效，请重新设置":"关闭时保持原有处理");src_ui_refresh();
 }else{
  unsigned limit=view==1?1:disc_src_max_shift(disc_src_rates[src_rule]),selected=view==1?src_rules.enabled:src_rules.shift[src_rule];
  for(unsigned i=0;i<=limit;i++){char b[48];if(view==1)snprintf(b,sizeof(b),"%s",i?"开启":"关闭");else if(!i)snprintf(b,sizeof(b),"保持原始");else {char rate[24];src_ui_rate(rate,sizeof rate,disc_src_rates[src_rule]>>i);snprintf(b,sizeof(b),"%s kHz",rate);}LvObj *row=FN(0x46d5c0,LvObj *(*)(LvObj *,const char *,unsigned,void (*)(void *)))(p,b,SRC_UI_VALUE|(i<<8),src_ui_click);src_choices[i]=row;src_ui_protect(row);if(row)FN(0x46d848,void (*)(LvObj *,int))(row,selected==i);}
  src_choice_status=src_ui_info(p,"选择后自动保存");
 }
 LvObj *footer=lv_create(p);if(footer){lv_size(footer,360,40);stylezero(footer);opa(footer,0,0);}FN(0x4e2734,void (*)(LvObj *))(p);src_updated=0;control_target=0;control_moved=1;
}
static void src_ui_close(int restore){
 LvObj *owner=src_owner;src_owner=0;src_action=0;
 if(restore&&owner&&lv_valid(owner)&&*(unsigned *)0x8e16d4==dsp_ui_mode)FN(0x4e2734,void (*)(LvObj *))(owner);
 for(int i=1;i>=0;i--){LvObj *old=src_pages[i];src_pages[i]=0;if(old&&lv_valid(old))FN(0x4f7e98,void (*)(LvObj **))(&old);}src_depth=src_view=0;src_status=src_choice_status=0;for(unsigned i=0;i<7;i++)src_rule_labels[i]=0;
}
static void src_ui_poll(void){
 if(src_pages[0]&&(!src_owner||!lv_valid(src_owner)||*(unsigned *)0x8e16d4!=dsp_ui_mode)){src_ui_close(0);return;}
 if(src_pages[0]&&!src_ui_active()){src_ui_close(0);return;}
 unsigned action=src_action,arg=src_argument;src_action=0;
 if(action==SRC_UI_OPEN&&!src_pages[0]&&dsp_page&&lv_active_screen(lv_disp_default())==dsp_page){src_owner=dsp_page;src_ui_load();src_ui_make(0);}
 else if(src_ui_active()){
  if(action==SRC_UI_BACK){if(src_depth){LvObj *old=src_pages[1];src_pages[1]=0;src_depth=src_view=0;FN(0x4e2734,void (*)(LvObj *))(src_pages[0]);if(old&&lv_valid(old))FN(0x4f7e98,void (*)(LvObj **))(&old);src_ui_refresh();}else src_ui_close(1);}
  else if(action==SRC_UI_ENABLE&&!src_depth)src_ui_make(1);
  else if(action==SRC_UI_RULE&&!src_depth&&arg<6){src_rule=arg;src_ui_make(2);}
  else if(action==SRC_UI_VALUE&&src_depth){DiscSrcRules next=src_rules;if(src_view==1)next.enabled=arg;else next.shift[src_rule]=arg;int ok=src_ui_save(&next);if(ok){unsigned selected=src_view==1?src_rules.enabled:src_rules.shift[src_rule];for(unsigned i=0;i<4;i++)if(src_choices[i])FN(0x46d848,void (*)(LvObj *,int))(src_choices[i],selected==i);src_ui_refresh();if(src_choice_status)lv_text(src_choice_status,"已保存");}else {if(src_status)lv_text(src_status,"设置未保存，请重试");if(src_choice_status)lv_text(src_choice_status,"设置未保存，请重试");}}
 }
 if(src_pages[0]&&!src_depth&&src_status&&now()-src_updated>=1000){src_updated=now();char state[2048];unsigned rate=0,active=0,error=0,transition=0,chain_error=0;if(textread("/tmp/disc-runtime/audio-dsp.state",state,sizeof state))for(unsigned i=0;state[i];i++)if(!i||state[i-1]=='\n'){unsigned v;if(sscanf(state+i,"processing_rate=%u",&v)==1)rate=v;else if(sscanf(state+i,"chain_error=%u",&v)==1)chain_error=v;else if(sscanf(state+i,"src_active=%u",&v)==1)active=v;else if(sscanf(state+i,"src_config_error=%u",&v)==1)error=v;else if(sscanf(state+i,"src_transition=%u",&v)==1)transition=v;}
  char b[64],number[24];src_ui_rate(number,sizeof number,rate);if(error||src_bad_config)snprintf(b,sizeof(b),"配置无效，请重新设置");else if(chain_error&&src_rules.enabled)snprintf(b,sizeof(b),"降采样未生效，请重试");else if(transition)snprintf(b,sizeof(b),"正在平滑切换…");else if(active)snprintf(b,sizeof(b),"实际处理：%s kHz",number);else snprintf(b,sizeof(b),"%s",src_rules.enabled?"当前音频保持原始采样率":"关闭时保持原有处理");const char *previous=*(const char **)((char *)src_status+44);if(!previous||strcmp(previous,b))lv_text(src_status,b);
 }
}
static void src_ui_add_entry(void){
 unsigned n=FN(0x4f889c,unsigned (*)(LvObj *))(dsp_page);int layout[3]={44,244,0};FN(0x485ef8,void (*)(LvObj *,const char *,unsigned,void (*)(void *),const int *))(dsp_page,"DSP 降采样",SRC_UI_OPEN,src_ui_click,layout);src_entry=FN(0x4f8850,LvObj *(*)(LvObj *,int))(dsp_page,(int)n);if(src_entry){lv_size(src_entry,360,50);padtop(src_entry,0,0);padbottom(src_entry,0,0);lv_clear_flag(src_entry,16|0x40000);lv_align(src_entry,1,0,DSP_ROW_TOP+DSP_ORDER_COUNT*DSP_ROW_PITCH+72);src_ui_protect(src_entry);lv_add_cb(src_entry,src_ui_deleted,33,0);}
}
