/* Native audio settings. All objects and callbacks stay on the UI thread.
 * Parents remain live while a child is visible: scroll/status ownership and
 * external navigation are handled independently of gesture completion. */
extern float strtof(const char *,char **);
extern int flock(int,int);
#include "../audio-dsp/shared_profile.h"
#include "../audio-dsp/mode_preferences.h"
#include "../audio-dsp/profile_format.h"
#include "shared_edit.h"
extern unsigned long strtoul(const char *,char **,int);
static int pu_read(const char *path,char *b,unsigned size){int f=open(path,0);if(f<0)return 0;int n=read(f,b,size-1);close(f);if(n<=0||n>=(int)size-1)return 0;b[n]=0;return n;}
static int pu_write(const char *path,const char *tmp,const char *b,unsigned size){int f=open(tmp,1|0x100|0x200,0600);if(f<0)return 0;int ok=write(f,b,size)==(int)size&&!fsync(f);close(f);if(ok&&!rename(tmp,path))return 1;unlink(tmp);return 0;}
#define PU_SHARED "/usr/data/disc-runtime/module-data/disc-remote-control/shared-profile.conf"
#define PU_RG "/usr/data/disc-runtime/module-data/disc-bt-meter/replaygain.conf"
static LvObj *pu_origin,*pu_entry,*pu_stack[4],*pu_message[4],*pu_slider,*pu_value_label,*pu_param_rows[12],*pu_node_rows[32],*pu_usb_labels[6];
static LvObj *pu_status;static unsigned pu_mode,pu_status_hidden,pu_depth,pu_views[4],pu_action,pu_arg,pu_updated,pu_changed;
static unsigned pu_type,pu_id,pu_param,pu_node_ids[32],pu_node_types[32],pu_node_count,pu_rg_mode,pu_rg_protect=1;
static float pu_baseline,pu_value,pu_rg_preamp;static int pu_dirty,pu_slider_steps;
static float pu_reset_snapshot[12];
static LvObj *pu_rg_labels[3];static unsigned pu_retry_token;
static char pu_error[100];
enum {PU_MAIN,PU_NODES,PU_PARAMS,PU_EDIT,PU_USB,PU_REPLAYGAIN,PU_RG_CHOICES,PU_RESET_CONFIRM};
enum {PU_OPEN=1,PU_DSP,PU_USB_OPEN,PU_RG_OPEN,PU_NODE,PU_PARAM,PU_BACK,PU_MINUS,PU_PLUS,PU_RESET,PU_USB_RETRY,PU_RG_MODE,PU_RG_SELECT,PU_RG_PREAMP,PU_RG_PROTECT,PU_RESET_APPLY};
static unsigned pu_ms(void){long t[2];return clock_gettime(1,t)?0:(unsigned)t[0]*1000u+(unsigned)t[1]/1000000u;}
static unsigned pu_bank(void){return pu_mode==0?3:pu_mode==3?0:pu_mode==1?1:2;}
static int pu_chain_read(DiscChain *c,unsigned *master){
 char b[8192],path[180];if(pu_read(PU_SHARED,b,sizeof b))return disc_shared_parse(b,c,master);
 if(!access(PU_SHARED,0))return 0; /* Invalid/unreadable shared wins over legacy. */
 unsigned masks[4];b[0]=0;pu_read("/usr/data/disc-runtime/module-data/disc-audio-dsp/settings",b,sizeof b);disc_dsp_read_masks(b,masks);
 unsigned bank=pu_bank();*master=masks[bank]!=0;c->count=0;
 if(masks[bank]&DISC_DSP_CUSTOM){snprintf(path,sizeof path,"/usr/data/disc-runtime/module-data/disc-remote-control/chain-%u.conf",bank);return pu_read(path,b,sizeof b)&&disc_chain_parse(b,c);}
 float values[12][12];for(unsigned i=0;i<12;i++)for(unsigned j=0;j<12;j++)values[i][j]=disc_param_default[i][j];
 snprintf(path,sizeof path,"/usr/data/disc-runtime/module-data/disc-remote-control/dsp-%u.params",bank);if(pu_read(path,b,sizeof b)&&!disc_profile_parse(b,values))return 0;
 const unsigned order[]={2,0,4,3,5,6,7,8,9,10,11};for(unsigned i=0;i<11;i++){unsigned type=order[i];if(!(masks[bank]&(1u<<type)))continue;DiscChainNode *n=&c->nodes[c->count++];n->id=c->count;n->type=type;n->enabled=1;for(unsigned j=0;j<12;j++)n->params[j]=values[type][j];}return disc_chain_valid(c);
}
static int pu_get_node(DiscChainNode *out){
 DiscChain c;unsigned master;if(!pu_chain_read(&c,&master))return 0;
 if(pu_id){for(unsigned i=0;i<c.count;i++)if(c.nodes[i].id==pu_id&&c.nodes[i].type==pu_type){*out=c.nodes[i];return 1;}return 0;}
 *out=(DiscChainNode){0};out->type=pu_type;for(unsigned j=0;j<disc_native_effects[pu_type].count;j++)out->params[j]=disc_native_effects[pu_type].params[j].initial;return 1;
}
static int pu_commit(void){
 if(!pu_dirty)return 1;int lock=open("/usr/data/disc-runtime/module-data/disc-remote-control/controls.lock",2|0x100,0600);if(lock<0){snprintf(pu_error,sizeof pu_error,"音效配置不可写，请重试");return 0;}if(flock(lock,6)){close(lock);snprintf(pu_error,sizeof pu_error,"配置正在更新，请稍后重试");return 0;}
 DiscChain c;unsigned master,id=pu_id;int result=pu_chain_read(&c,&master)?disc_shared_edit(&c,&id,pu_type,pu_param,pu_baseline,pu_value):-2;
 if(result==1){char b[8192];int n=disc_shared_format(&c,master,b,sizeof b);result=n&&disc_shared_parse(b,&c,&master)&&pu_write(PU_SHARED,PU_SHARED ".native-new",b,(unsigned)n)?1:-4;}
 flock(lock,8);close(lock);
 if(result==1){if(!pu_id)for(unsigned i=0;i<pu_node_count;i++)if(!pu_node_ids[i]&&pu_node_types[i]==pu_type)pu_node_ids[i]=id;pu_id=id;for(unsigned i=0;i<c.count;i++)if(c.nodes[i].id==id)pu_baseline=pu_value=c.nodes[i].params[pu_param];pu_dirty=0;pu_error[0]=0;return 1;}
 snprintf(pu_error,sizeof pu_error,"%s",result==-1?"参数已被修改，请返回后重试":result==-3?"已保存16项，请先移除一项":"参数未保存，请重试");pu_dirty=0;return 0;
}
static int pu_reset_defaults(void){
 int lock=open("/usr/data/disc-runtime/module-data/disc-remote-control/controls.lock",2|0x100,0600);
 if(lock<0){snprintf(pu_error,sizeof pu_error,"音效配置不可写，请重试");return -4;}
 if(flock(lock,6)){close(lock);snprintf(pu_error,sizeof pu_error,"配置正在更新\n请稍后重试");return -4;}
 DiscChain c;unsigned master;int result=pu_chain_read(&c,&master)?disc_shared_reset(&c,pu_id,pu_type,pu_reset_snapshot):-2;
 if(result==1){char b[8192];int n=disc_shared_format(&c,master,b,sizeof b);if(!n||!pu_write(PU_SHARED,PU_SHARED ".native-new",b,(unsigned)n))result=-4;}
 flock(lock,8);close(lock);
 if(result>=0)pu_error[0]=0;else snprintf(pu_error,sizeof pu_error,"%s",result==-1?"参数已变化\n请取消后重试":"恢复默认未保存\n请重试");return result;
}
static void pu_click(void *e){if(!stationary_clicked(e))return;unsigned data=(unsigned long)FN(0x4e394c,void *(*)(void *))(e);pu_action=data&255;pu_arg=data>>8;}
static void pu_back(void *e){if(FN(0x488860,int (*)(void *))(e)==3)pu_action=PU_BACK;}
static void pu_deleted(void *e){LvObj *o=lv_event_target(e);if(o==pu_origin)pu_origin=pu_entry=0;if(o==pu_entry)pu_entry=0;for(unsigned i=0;i<4;i++)if(o==pu_stack[i]){pu_stack[i]=pu_message[i]=0;}}
static void pu_status_restore(void){LvObj *s=LV_GLOBAL(0x8de408);if(s&&s==pu_status&&lv_valid(s)&&*(unsigned *)0x8e16d4==pu_mode&&(*(unsigned *)((char *)s+36)&1)!=pu_status_hidden)FN(pu_status_hidden?0x479090:0x4790b0,void (*)(void))();}
static void pu_close(int restore){
 pu_dirty=0;pu_action=0;pu_slider=pu_value_label=0;
 if(restore&&pu_origin&&lv_valid(pu_origin)&&*(unsigned *)0x8e16d4==pu_mode){toolbox_clear_native_layers();screen_load(pu_origin);pu_status_restore();}
 for(int i=3;i>=0;i--){LvObj *old=pu_stack[i];pu_stack[i]=pu_message[i]=0;if(old&&lv_valid(old))screen_delete(&old);}pu_depth=0;
}
static LvObj *pu_screen(const char *title,int list){
 LvObj *p=lv_create(0);if(!p)return 0;screen_register(p);lv_size(p,360,360);lv_radius(p,0,0);lv_border(p,0,0);lv_bg(p,0xff000000,0);bg_opa(p,255,0);
 pad_left(p,0,0);pad_right(p,0,0);pad_top(p,0,0);pad_bottom(p,0,0);FN(0x4f6b30,void (*)(LvObj *,int,unsigned))(p,5,0);
 FN(0x4f0268,void (*)(LvObj *,int))(p,0);FN(0x4f02c8,void (*)(LvObj *,int))(p,12);if(list){FN(0x5030a0,void (*)(LvObj *,int))(p,1);FN(0x5030e4,void (*)(LvObj *,int,int,int))(p,0,0,2);}
 native_title(p,title,0xb4fb);lv_add_cb(p,pu_deleted,DISC_LV_DELETE,0);lv_add_cb(p,pu_back,1,0);lv_add_cb(p,pu_back,2,0);lv_add_cb(p,pu_back,8,0);return p;
}
static LvObj *pu_row(LvObj *p,const char *text,unsigned action,unsigned arg){unsigned n=child_count(p);original_row(p,text,action|(arg<<8),pu_click,0);LvObj *o=child(p,(int)n);if(o)protect_tap(o);return o;}
static LvObj *pu_info(LvObj *p,const char *text){LvObj *row=pu_row(p,text,0,0);if(row){lv_clear_flag(row,DISC_LV_CLICKABLE);LvObj *arrow=child(row,1);if(arrow)lv_add_flag(arrow,DISC_LV_HIDDEN);}return row?child(row,0):0;}
static void pu_footer(LvObj *p){LvObj *o=lv_create(p);if(!o)return;lv_size(o,360,40);lv_border(o,0,0);bg_opa(o,0,0);pad_top(o,0,0);pad_bottom(o,0,0);lv_clear_flag(o,DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);lv_add_flag(o,0x4000);}
static void pu_message_set(const char *s){LvObj *o=pu_message[pu_depth];if(o&&lv_valid(o))lv_text(o,s);}
static void pu_format_value(char *b,unsigned size,unsigned type,unsigned param,float value){
 if(type==12&&param==3){const char *s[]={"峰值均衡","低频搁架","高频搁架","低通滤波","高通滤波"};unsigned v=(unsigned)value;snprintf(b,size,"%s",s[v<5?v:0]);return;}
 if(type==13&&param==2){const char *s[]={"延迟右声道","延迟左声道","居中环绕"};unsigned v=(unsigned)value;snprintf(b,size,"%s",s[v<3?v:0]);return;}
 const DiscNativeParam *p=&disc_native_effects[type].params[param];snprintf(b,size,p->step>=1?"%.0f %s":p->step>=.1f?"%.1f %s":"%.2f %s",(double)value,p->unit);
}
static void pu_editor_text(void){char b[100];if(pu_type==99)snprintf(b,sizeof b,"%.1f dB",(double)pu_value);else pu_format_value(b,sizeof b,pu_type,pu_param,pu_value);if(pu_value_label)lv_text(pu_value_label,b);}
static void pu_slider_event(void *e){unsigned code=lv_event_code(e);if(lv_event_target(e)!=pu_slider)return;
 if(code==28){int v=FN(0x52a7f0,int (*)(LvObj *))(pu_slider);float low=pu_type==99?-12:disc_native_effects[pu_type].params[pu_param].low,step=pu_type==99?.5f:disc_native_effects[pu_type].params[pu_param].step,high=pu_type==99?12:disc_native_effects[pu_type].params[pu_param].high;
  pu_value=low+(float)v*step;if(pu_value>high)pu_value=high;pu_dirty=pu_value!=pu_baseline;pu_changed=pu_ms();pu_editor_text();}
 if(code==8&&pu_dirty)pu_changed=pu_ms()-250;
}
static LvObj *pu_step_button(LvObj *p,int x,const char *image,unsigned action){
 /* Mirror the stock brightness +/- button style, but use our own callback:
  * 0x470ea8 itself installs brightness-writing callbacks and is not called. */
 LvObj *o=lv_btn(p);if(!o)return 0;lv_size(o,44,44);lv_align(o,1,x,219);pad_left(o,0,0);pad_right(o,0,0);pad_top(o,0,0);pad_bottom(o,0,0);lv_radius(o,0,0);lv_border(o,0,0);FN(0x4f6cb0,void (*)(LvObj *,int,unsigned))(o,0,0);bg_opa(o,0,0);lv_bg(o,0xffffffff,0);
 LvObj *im=FN(0x52cb10,LvObj *(*)(LvObj *))(o);if(im){lv_align(im,9,0,0);FN(0x52cb44,void (*)(LvObj *,const void *))(im,image);}
 lv_add_flag(o,0x4000);FN(0x4f02c8,void (*)(LvObj *,int))(o,0);protect_tap(o);lv_add_cb(o,pu_click,DISC_LV_CLICKED,(void *)(unsigned long)action);return o;
}
static void pu_rg_load(void){char b[80];pu_rg_mode=0;pu_rg_preamp=0;pu_rg_protect=1;if(pu_read(PU_RG,b,sizeof b)){unsigned version,mode,protect;float preamp;int at=0;if(sscanf(b,"%u %u %f %u %n",&version,&mode,&preamp,&protect,&at)==4&&!b[at]&&version==1&&mode<=2&&protect<=1&&isfinite(preamp)&&preamp>=-12&&preamp<=12){pu_rg_mode=mode;pu_rg_preamp=preamp;pu_rg_protect=protect;}}}
static int pu_rg_save(unsigned mode,float preamp,unsigned protect){char b[80];int n=snprintf(b,sizeof b,"1 %u %.3f %u\n",mode,(double)preamp,protect);int ok=pu_write(PU_RG,PU_RG ".new",b,(unsigned)n);pu_rg_load();if(!ok)snprintf(pu_error,sizeof pu_error,"响度设置未保存，请重试");else pu_error[0]=0;return ok;}
static void pu_update_usb(void);
static void pu_make(unsigned view){
 if(pu_depth>=4)return;const char *title=view==PU_MAIN?"音频设置":view==PU_NODES?"DSP 参数":view==PU_PARAMS?disc_native_effects[pu_type].name:view==PU_EDIT?(pu_type==99?"响度预增益":disc_native_effects[pu_type].params[pu_param].label):view==PU_USB?"USB 输出详情":view==PU_REPLAYGAIN?"响度管理":view==PU_RESET_CONFIRM?"恢复默认参数":"响度模式";
 LvObj *p=pu_screen(title,view!=PU_EDIT);if(!p)return;pu_stack[pu_depth]=p;pu_views[pu_depth]=view;pu_message[pu_depth]=0;pu_error[0]=0;
 if(view==PU_MAIN){pu_row(p,"DSP 参数",PU_DSP,0);pu_row(p,"USB 输出详情",PU_USB_OPEN,0);pu_row(p,"响度管理",PU_RG_OPEN,0);}
 else if(view==PU_NODES){
  DiscChain c;unsigned master;pu_node_count=0;unsigned types=0;
  if(pu_chain_read(&c,&master)){
   for(unsigned i=0;i<c.count;i++){DiscChainNode *n=&c.nodes[i];char text[100];snprintf(text,sizeof text,"%s · %u",disc_native_effects[n->type].name,n->id);pu_node_ids[pu_node_count]=n->id;pu_node_types[pu_node_count]=n->type;pu_node_rows[pu_node_count]=pu_row(p,text,PU_NODE,pu_node_count);pu_node_count++;types|=1u<<n->type;}
   for(unsigned type=0;type<17;type++)if(type!=1&&!(types&(1u<<type))&&pu_node_count<32){pu_node_ids[pu_node_count]=0;pu_node_types[pu_node_count]=type;char text[100];snprintf(text,sizeof text,"%s · 未添加",disc_native_effects[type].name);pu_node_rows[pu_node_count]=pu_row(p,text,PU_NODE,pu_node_count);pu_node_count++;}
   pu_info(p,"修改参数\n不会自动开启音效");
  }else pu_info(p,"音效配置无法读取");
 }else if(view==PU_PARAMS){
  DiscChainNode node;if(pu_get_node(&node)){pu_row(p,"恢复默认参数",PU_RESET,0);for(unsigned i=0;i<disc_native_effects[pu_type].count;i++){char value[60],text[120];pu_format_value(value,sizeof value,pu_type,i,node.params[i]);snprintf(text,sizeof text,"%s\n%s",disc_native_effects[pu_type].params[i].label,value);pu_param_rows[i]=pu_row(p,text,PU_PARAM,i);}pu_message[pu_depth]=pu_info(p,"修改后自动保存");}
  else pu_info(p,"该音效已移除，请返回刷新");
 }else if(view==PU_EDIT){
  /* Exact factory brightness slider geometry/style, with our numeric range. */
  float low=pu_type==99?-12:disc_native_effects[pu_type].params[pu_param].low,step=pu_type==99?.5f:disc_native_effects[pu_type].params[pu_param].step,high=pu_type==99?12:disc_native_effects[pu_type].params[pu_param].high;
  pu_slider_steps=(int)((high-low)/step+.5f);if(pu_slider_steps>32767)pu_slider_steps=32767;
  pu_value_label=lv_label(p);lv_add_style(pu_value_label,(const void *)0x83ac5c,0);lv_color(pu_value_label,0xffffffff,0);lv_size(pu_value_label,260,40);FN(0x52f4f0,void (*)(LvObj *,int))(pu_value_label,2);lv_align(pu_value_label,2,0,112);pu_editor_text();
  pu_slider=FN(0x53342c,LvObj *(*)(LvObj *))(p);if(pu_slider){lv_size(pu_slider,250,47);FN(0x52a754,void (*)(LvObj *,int,int))(pu_slider,0,pu_slider_steps);FN(0x52a4ec,void (*)(LvObj *,int,int))(pu_slider,(int)((pu_value-low)/step+.5f),0);FN(0x52a7c8,void (*)(LvObj *,int))(pu_slider,0);FN(0x4f6e80,void (*)(LvObj *,int,unsigned))(pu_slider,0,0x30000);lv_align(pu_slider,9,0,-10);lv_add_cb(pu_slider,pu_slider_event,28,0);lv_add_cb(pu_slider,pu_slider_event,8,0);}
  pu_step_button(p,58,"D:/usr/project/config/img/apps/settings/decrease.png",PU_MINUS);pu_step_button(p,260,"D:/usr/project/config/img/apps/settings/add.png",PU_PLUS);
  /* Absolute children do not take part in the factory list layout. */
  FN(0x4f0268,void (*)(LvObj *,int))(p,0);pu_message[pu_depth]=lv_label(p);lv_add_style(pu_message[pu_depth],(const void *)0x83ac5c,0);lv_color(pu_message[pu_depth],0xffaaaaaa,0);lv_size(pu_message[pu_depth],320,52);FN(0x52f4f0,void (*)(LvObj *,int))(pu_message[pu_depth],2);lv_align(pu_message[pu_depth],2,0,284);lv_text(pu_message[pu_depth],"调整后自动保存");
 }else if(view==PU_USB){for(unsigned i=0;i<6;i++)pu_usb_labels[i]=pu_info(p,"");pu_row(p,"重新连接",PU_USB_RETRY,0);pu_info(p,"保留当前音量\n和播放状态");pu_update_usb();}
 else if(view==PU_REPLAYGAIN){pu_rg_load();pu_rg_labels[0]=child(pu_row(p,"模式",PU_RG_MODE,0),0);pu_rg_labels[1]=child(pu_row(p,"预增益",PU_RG_PREAMP,0),0);pu_rg_labels[2]=child(pu_row(p,"标签峰值保护",PU_RG_PROTECT,0),0);pu_info(p,"仅本地 PCM\n无标签保持原样");pu_info(p,"缺少专辑标签\n使用单曲标签");pu_message[pu_depth]=pu_info(p,"读取状态中…");}
 else if(view==PU_RG_CHOICES){for(unsigned i=0;i<3;i++){LvObj *row=FN(0x46d5c0,LvObj *(*)(LvObj *,const char *,unsigned,void (*)(void *)))(p,i==0?"关闭":i==1?"单曲":"专辑",PU_RG_SELECT|(i<<8),pu_click);if(row){protect_tap(row);FN(0x46d848,void (*)(LvObj *,int))(row,pu_rg_mode==i);}}}
 else if(view==PU_RESET_CONFIRM){pu_info(p,disc_native_effects[pu_type].name);pu_info(p,"只恢复本音效参数\n开关和排序保持不变");pu_row(p,"恢复默认",PU_RESET_APPLY,0);pu_row(p,"取消",PU_BACK,0);pu_message[pu_depth]=pu_info(p,"");}
 if(view!=PU_EDIT)pu_footer(p);toolbox_clear_native_layers();screen_load(p);pu_updated=0;
}
static void pu_prepare(void){
 if(pu_stack[pu_depth]&&lv_active_screen(lv_disp_default())==pu_stack[pu_depth])toolbox_clear_native_layers();
 if(!page||!lv_valid(page))return;
 if(pu_stack[0]&&pu_origin!=page)pu_close(0);
 if(pu_origin!=page||!pu_entry||!lv_valid(pu_entry)){unsigned n=child_count(page);pu_entry=pu_row(page,"音频设置",PU_OPEN,0);if(!pu_entry)return;pu_origin=page;lv_add_cb(page,pu_deleted,DISC_LV_DELETE,0);lv_add_cb(pu_entry,pu_deleted,DISC_LV_DELETE,0);if(n>2)move_index(pu_entry,2);}
 LvObj *label=child(pu_entry,0);const char *text=label?*(const char **)((char *)label+44):0;if(label&&(!text||strcmp(text,"音频设置")))lv_text(label,"音频设置");
}
static void pu_pop(void){if(!pu_depth){pu_close(1);return;}LvObj *old=pu_stack[pu_depth];pu_stack[pu_depth]=pu_message[pu_depth]=0;pu_depth--;pu_slider=pu_value_label=0;toolbox_clear_native_layers();screen_load(pu_stack[pu_depth]);if(old&&lv_valid(old))screen_delete(&old);pu_updated=0;}
static void pu_refresh_params(void){DiscChainNode node;if(!pu_get_node(&node)){pu_message_set("该音效已移除，请返回刷新");return;}for(unsigned i=0;i<disc_native_effects[pu_type].count;i++)if(pu_param_rows[i]&&lv_valid(pu_param_rows[i])){char b[130],v[60];pu_format_value(v,sizeof v,pu_type,i,node.params[i]);snprintf(b,sizeof b,"%s\n%s",disc_native_effects[pu_type].params[i].label,v);lv_text(child(pu_param_rows[i],0),b);}}
static void pu_poll(void);
static const char *pu_state_field(const char *text,const char *key){
 for(const char *p=text;*p;){unsigned j=0;while(key[j]&&p[j]&&p[j]==key[j])j++;if(!key[j]&&p[j]=='=')return p+j+1;while(*p&&*p!='\n')p++;if(*p)p++;}return 0;
}
static unsigned pu_state_uint(const char *text,const char *key,unsigned fallback){
 const char *p=pu_state_field(text,key);if(p&&*p>='0'&&*p<='9'){char *end;unsigned long value=strtoul(p,&end,10);if(value<=0xfffffffful&&(*end=='\n'||!*end))return (unsigned)value;}
 return fallback;
}
static int pu_state_error(const char *text,const char *key){const char *p=pu_state_field(text,key);if(!p)return 0;int value=0;return sscanf(p,"%d",&value)==1?value:0;}
static int pu_state_text(const char *text,const char *key,char *out,unsigned size){
 if(!size)return 0;const char *p=pu_state_field(text,key);if(p){unsigned j=0;while(*p&&*p!='\n'&&j+1<size)out[j++]=*p++;out[j]=0;return 1;}out[0]=0;return 0;
}
static int pu_player_snapshot(char *state,unsigned size){
 if(!pu_read("/tmp/disc-runtime/player-usb.state",state,size))return 0;unsigned stamp=pu_state_uint(state,"updated",0),pid=pu_state_uint(state,"pid",0),now=pu_ms()/1000;
 return pid>0&&stamp<=now&&now-stamp<=3;
}
static void pu_update_usb(void){
 char state[1536],b[180],name[256],path[120];int fresh=pu_player_snapshot(state,sizeof state);
 unsigned present=fresh?pu_state_uint(state,"usb_present",0):0,card=fresh?pu_state_uint(state,"usb_card",99):99;
 snprintf(b,sizeof b,"设备\n%s",fresh?(present?"正在读取…":"未连接 USB 解码器"):"播放器状态未就绪");
 if(present&&card>=1&&card<=9){
  snprintf(path,sizeof path,"/proc/asound/card%u/stream0",card);if(pu_read(path,name,sizeof name)){unsigned i=0;while(name[i]&&name[i]!='\n')i++;name[i]=0;snprintf(b,sizeof b,"设备\n%.130s",name);}
  else{snprintf(path,sizeof path,"/proc/asound/card%u/id",card);if(pu_read(path,name,sizeof name))snprintf(b,sizeof b,"设备\n%.130s",name);}
 }if(pu_usb_labels[0])lv_text(pu_usb_labels[0],b);
 unsigned owned=fresh?pu_state_uint(state,"usb_owned",0):0,ready=fresh?pu_state_uint(state,"usb_ready",0):0,player=fresh?pu_state_uint(state,"player_state",0):0,recovering=fresh?pu_state_uint(state,"usb_recovering",0):0;int error=fresh?pu_state_error(state,"usb_error"):0;
 const char *status=!fresh?"状态过期":!owned?"USB 输出未开启":!present?"等待设备":error?"初始化失败":recovering?"正在初始化":ready?"音频连接就绪":player==0||player==3?"设备已识别，等待播放":"等待音频连接";
 if(error)snprintf(b,sizeof b,"连接状态\n%s（%d）",status,error);else snprintf(b,sizeof b,"连接状态\n%s",status);if(pu_usb_labels[1])lv_text(pu_usb_labels[1],b);
 snprintf(b,sizeof b,"实际输出\n未建立 PCM 连接");if(present&&card>=1&&card<=9){
  unsigned dev=pu_state_uint(state,"usb_device",0);if(dev<32){snprintf(path,sizeof path,"/proc/asound/card%u/pcm%up/sub0/hw_params",card,dev);char hw[640];if(pu_read(path,hw,sizeof hw)){unsigned rate=0;char format[32]={0};for(unsigned i=0;hw[i];i++)if(!i||hw[i-1]=='\n'){sscanf(hw+i,"rate: %u",&rate);sscanf(hw+i,"format: %31s",format);}if(rate)snprintf(b,sizeof b,"实际输出\n%.1f kHz · %s",(double)rate/1000,format);}}
 }if(pu_usb_labels[2])lv_text(pu_usb_labels[2],b);
 unsigned rate=fresh?pu_state_uint(state,"sample_rate",0):0,bits=fresh?pu_state_uint(state,"sample_bits",0):0;
 if(rate)snprintf(b,sizeof b,"播放器格式\n%.1f kHz · %u bit",(double)rate/1000,bits);else snprintf(b,sizeof b,"播放器格式\n等待音频");if(pu_usb_labels[3])lv_text(pu_usb_labels[3],b);
 snprintf(b,sizeof b,"传输状态\n%s",ready?"PCM 已打开（不代表耳机有声）":player==3?"已暂停":"未传输 PCM");if(pu_usb_labels[4])lv_text(pu_usb_labels[4],b);
 if(pu_retry_token&&fresh&&pu_state_uint(state,"manual_token",0)==pu_retry_token){if(pu_state_uint(state,"manual_result",99))snprintf(b,sizeof b,"重新连接\n设备状态已变化，请重试");else snprintf(b,sizeof b,"重新连接\n%s",recovering?"正在恢复":error?"恢复失败，可再次重试":"已执行");}
 else snprintf(b,sizeof b,"当前模式\n%s",pu_mode==0?"纯音":pu_mode==3?"蓝牙接收":"仅纯音和蓝牙支持 USB 输出");if(pu_usb_labels[5])lv_text(pu_usb_labels[5],b);
}
static void pu_retry_usb(void){
 char state[1536],b[100];if(!pu_player_snapshot(state,sizeof state)||!pu_state_uint(state,"usb_owned",0)){if(pu_usb_labels[5])lv_text(pu_usb_labels[5],"请先开启 USB 输出");return;}
 unsigned mode=pu_state_uint(state,"work_mode",0);if((pu_mode!=0&&pu_mode!=3)||mode!=(pu_mode==0?1u:2u)){if(pu_usb_labels[5])lv_text(pu_usb_labels[5],"当前模式无法重新连接");return;}
 if(!pu_state_uint(state,"usb_present",0)){if(pu_usb_labels[5])lv_text(pu_usb_labels[5],"未识别 USB 设备，请检查连接");return;}
 unsigned token=pu_ms();if(!token)token=1;int n=snprintf(b,sizeof b,"%u %u %u %u %u\n",pu_state_uint(state,"pid",0),mode,pu_state_uint(state,"usb_generation",0),pu_ms()/1000,token);
 int f=open("/tmp/disc-runtime/usb-reconnect.request.new",1|0x100|0x200,0600);int ok=0;if(f>=0){ok=write(f,b,n)==n;close(f);if(ok)ok=!rename("/tmp/disc-runtime/usb-reconnect.request.new","/tmp/disc-runtime/usb-reconnect.request");}
 if(ok){pu_retry_token=token;if(pu_usb_labels[5])lv_text(pu_usb_labels[5],"重新连接请求已发送");}else if(pu_usb_labels[5])lv_text(pu_usb_labels[5],"请求未发送，请重试");
}
static void pu_update_rg(void){
 pu_rg_load();char b[160];snprintf(b,sizeof b,"模式\n%s",pu_rg_mode==1?"单曲":pu_rg_mode==2?"专辑":"关闭");if(pu_rg_labels[0])lv_text(pu_rg_labels[0],b);
 snprintf(b,sizeof b,"预增益\n%.1f dB",(double)pu_rg_preamp);if(pu_rg_labels[1])lv_text(pu_rg_labels[1],b);snprintf(b,sizeof b,"标签峰值保护\n%s",pu_rg_protect?"开启":"关闭");if(pu_rg_labels[2])lv_text(pu_rg_labels[2],b);
 char state[384];const char *status="等待播放器状态";unsigned now=pu_ms()/1000;
 if(!pu_rg_mode)status="响度管理已关闭";
 else if(pu_mode!=0)status="当前模式保持原样";
 else if(pu_read("/tmp/disc-runtime/replaygain.state",state,sizeof state)&&pu_state_uint(state,"updated",0)<=now&&now-pu_state_uint(state,"updated",0)<=3){
  unsigned source=pu_state_uint(state,"source",0);if(!pu_state_uint(state,"ready",0))status="响度功能尚未就绪";
  else if(pu_state_uint(state,"dsd",0))status="DSD 保持原样";
  else if(!pu_state_uint(state,"blocks",0))status="等待本地 PCM 播放";
  else if(!source)status="当前曲目无可用响度标签";
  else{char gain[40];pu_state_text(state,"gain_db",gain,sizeof gain);snprintf(b,sizeof b,"%s · %s dB%s",source==2?"专辑标签":source==3?"单曲回退":"单曲标签",gain,pu_state_uint(state,"peak_limited",0)?"\n已按标签峰值限制增益":"");status=b;}
 }pu_message_set(pu_error[0]?pu_error:status);
}
static void pu_push(unsigned view){if(pu_depth>=3)return;pu_depth++;pu_make(view);if(!pu_stack[pu_depth])pu_depth--;}
static void pu_poll(void){
 pu_prepare();LvObj *active=lv_active_screen(lv_disp_default());
 if(pu_stack[0]&&(!pu_origin||!lv_valid(pu_origin)||*(unsigned *)0x8e16d4!=pu_mode||active!=pu_stack[pu_depth])){pu_close(0);return;}
 unsigned action=pu_action,arg=pu_arg;pu_action=0;
 if(action==PU_OPEN&&!pu_stack[0]&&active==pu_origin){pu_mode=*(unsigned *)0x8e16d4;pu_status=LV_GLOBAL(0x8de408);pu_status_hidden=pu_status&&lv_valid(pu_status)?*(unsigned *)((char *)pu_status+36)&1:0;pu_depth=0;pu_make(PU_MAIN);return;}
 if(!pu_stack[0])return;unsigned view=pu_views[pu_depth],t=pu_ms();
 if(view==PU_EDIT&&pu_dirty&&((unsigned)(t-pu_changed)>=250||action==PU_BACK)){
  int ok;if(pu_type==99){ok=pu_rg_save(pu_rg_mode,pu_value,pu_rg_protect);pu_dirty=0;if(ok)pu_baseline=pu_value;}else ok=pu_commit();
  pu_message_set(ok?"已保存":pu_error);if(!ok&&action==PU_BACK)return;
 }
 if(action==PU_BACK){pu_pop();return;}
 if(action==PU_DSP&&view==PU_MAIN)pu_push(PU_NODES);
 else if(action==PU_USB_OPEN&&view==PU_MAIN){pu_retry_token=0;pu_push(PU_USB);}
 else if(action==PU_RG_OPEN&&view==PU_MAIN)pu_push(PU_REPLAYGAIN);
 else if(action==PU_NODE&&view==PU_NODES&&arg<pu_node_count){pu_type=pu_node_types[arg];pu_id=pu_node_ids[arg];pu_push(PU_PARAMS);}
 else if(action==PU_RESET&&view==PU_PARAMS){DiscChainNode n;if(pu_get_node(&n)){for(unsigned j=0;j<12;j++)pu_reset_snapshot[j]=n.params[j];pu_push(PU_RESET_CONFIRM);}else pu_message_set("该音效已移除\n请返回刷新");}
 else if(action==PU_RESET_APPLY&&view==PU_RESET_CONFIRM){int result=pu_reset_defaults();if(result>=0){pu_pop();pu_refresh_params();pu_message_set(result?"已恢复默认参数":"当前已是默认参数");}else pu_message_set(pu_error);return;}
 else if(action==PU_PARAM&&view==PU_PARAMS&&arg<disc_native_effects[pu_type].count){DiscChainNode n;if(pu_get_node(&n)){pu_param=arg;pu_value=pu_baseline=n.params[arg];pu_dirty=0;pu_push(PU_EDIT);}}
 else if((action==PU_MINUS||action==PU_PLUS)&&view==PU_EDIT){float low=pu_type==99?-12:disc_native_effects[pu_type].params[pu_param].low,high=pu_type==99?12:disc_native_effects[pu_type].params[pu_param].high,step=pu_type==99?.5f:disc_native_effects[pu_type].params[pu_param].step;
  float next=pu_value+(action==PU_PLUS?step:-step);if(next<low)next=low;if(next>high)next=high;pu_value=next;pu_dirty=pu_value!=pu_baseline;pu_changed=t-250;pu_editor_text();if(pu_slider)FN(0x52a4ec,void (*)(LvObj *,int,int))(pu_slider,(int)((next-low)/step+.5f),0);
 }else if(action==PU_USB_RETRY&&view==PU_USB){pu_retry_usb();return;}
 else if(action==PU_RG_MODE&&view==PU_REPLAYGAIN)pu_push(PU_RG_CHOICES);
 else if(action==PU_RG_SELECT&&view==PU_RG_CHOICES&&arg<=2){if(pu_rg_save(arg,pu_rg_preamp,pu_rg_protect))pu_pop();else pu_message_set(pu_error);}
 else if(action==PU_RG_PREAMP&&view==PU_REPLAYGAIN){pu_type=99;pu_value=pu_baseline=pu_rg_preamp;pu_dirty=0;pu_push(PU_EDIT);}
 else if(action==PU_RG_PROTECT&&view==PU_REPLAYGAIN)pu_rg_save(pu_rg_mode,pu_rg_preamp,!pu_rg_protect);
 if(t-pu_updated>=1000||!pu_updated){pu_updated=t;view=pu_views[pu_depth];if(view==PU_PARAMS)pu_refresh_params();else if(view==PU_USB)pu_update_usb();else if(view==PU_REPLAYGAIN)pu_update_rg();}
}
