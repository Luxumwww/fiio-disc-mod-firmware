/* USB DAC host feature module. Rendering libraries and DPU remain shared. */
static LvObj *dac_buttons[6],*dac_icons[6],*dac_captions[6],*dac_shade,*dac_brightness_holder,*dac_clip;
static int dac_menu_network_seen=-1,dac_native_grid_hidden;
static int dac_menu_mode(void){return dlna_visual()||(usbdac_enabled&&*(unsigned *)0x8e16d4==1);}
static int dac_folded=1,dac_fold_seen=-1,dac_bright_seen=-1,dac_bright_y=185;
static int dac_connection_seen=-1,dac_eq_seen=-1,dac_rotation_seen=-1,dac_brightness_shifted,dac_holder_height;
static unsigned dac_down_at;static int dac_x,dac_y,dac_dragging;
static void dac_deleted(void *e){LvObj *o=lv_event_target(e);for(unsigned i=0;i<6;i++){if(o==dac_buttons[i])dac_buttons[i]=dac_icons[i]=0;if(o==dac_captions[i])dac_captions[i]=0;}if(o==dac_clip){dac_clip=0;dac_fold_seen=-1;}if(o==dac_shade)dac_shade=0;if(o==dac_brightness_holder){dac_brightness_holder=0;dac_bright_seen=-1;}}
static void dac_bright_move(LvObj *o,int y){dac_bright_y=y;LvObj *holder=parent(o);if(holder&&lv_valid(holder)&&dac_shade&&lv_valid(dac_shade)){int *hb=(int *)((char *)holder+20),*sb=(int *)((char *)dac_shade+20);lv_align(o,2,0,sb[1]+y-hb[1]);}}
static void dac_pref(const char *key,int value){char path[192],stage[196];snprintf(path,sizeof(path),"/usr/data/disc-runtime/module-data/disc-bt-meter/%s",key);snprintf(stage,sizeof(stage),"%s.new",path);int f=open(stage,1|0x100|0x200,0600);if(f>=0){char c=value?'1':'0';int n=write(f,&c,1);close(f);if(n==1)rename(stage,path);}}
static void dac_toggle_icons(void){
 if(dac_captions[3]){lv_text(dac_captions[3],dlna_visual()?(net_info_enabled?"信息显示 开":"信息显示 关"):visual_source_caption());src(dac_icons[3],(dlna_visual()?net_info_enabled:visual_post)?&info_icon_1:&info_icon_0);}
 if(dac_captions[4]){lv_text(dac_captions[4],(dlna_visual()?net_normalize_enabled:dac_normalize_enabled)?"动效归一化 开":"动效归一化 关");src(dac_icons[4],(dlna_visual()?net_normalize_enabled:dac_normalize_enabled)?&normalize_icon_1:&normalize_icon_0);}
}
static void dac_info_click(void *e){if(!control_clicked(e))return;if(dlna_visual()){net_info_enabled=!net_info_enabled;dac_pref("dlna-info-enabled",net_info_enabled);}else{visual_source_toggle();return;}dac_toggle_icons();}
static void dac_normalize_click(void *e){if(!control_clicked(e))return;if(dlna_visual()){net_normalize_enabled=!net_normalize_enabled;dac_pref("dlna-normalize-enabled",net_normalize_enabled);}else{dac_normalize_enabled=!dac_normalize_enabled;dac_pref("usbdac-normalize-enabled",dac_normalize_enabled);}xy_gain=(DiscVisualGain){0};dac_toggle_icons();}
static void dac_close_shade(void *e){
 if(vt_editing||!active||!dac_menu_mode()||!menu_open)return;
 unsigned code=lv_event_code(e);int pt[2];void *input=FN(0x4e42e8,void *(*)(void))();if(!input)return;FN(0x4e45bc,void (*)(void *,int *))(input,pt);
 if(code==1){dac_x=pt[0];dac_y=pt[1];dac_down_at=now();dac_dragging=1;}
 else if((code==8||code==3)&&dac_dragging){dac_dragging=0;int dx=pt[0]-dac_x,dy=pt[1]-dac_y;
  if(code==8&&now()-dac_down_at<2000&&dy*dy>400&&dy*dy>dx*dx){
   if(dac_y>=300||(dy<0&&dac_folded)){LvObj *shade=LV_GLOBAL(0x8deee4);if(shade&&lv_valid(shade))animate(shade,FN(0x4eb600,void (*)(LvObj *,int)),*(int *)((char *)shade+24),-360);}
   else dac_folded=dy<0;
  }
 }
}
static void dac_connection_click(void *e){
 if(!control_clicked(e)||!usbdac_enabled||*(unsigned *)0x8e16d4!=1)return;
 /* Same preset byte and native 0818 message used by stock PC/phone buttons. */
 volatile unsigned char *value=(void *)0x8e172c;unsigned next=*value?0:1;*value=(unsigned char)next;
 FN(0x48b28c,void (*)(const void *,int,int))((const void *)0x6a0254,(int)next,0);
 dac_connection_seen=-1;
}
static void usbdac_menu_visibility(unsigned mode){
 int wanted=active&&dac_menu_mode(),network=dlna_visual();
 if(network){show(LV_GLOBAL(0x8dee94),0);dac_native_grid_hidden=1;}else if(dac_native_grid_hidden){if(mode!=1)show(LV_GLOBAL(0x8dee94),1);dac_native_grid_hidden=0;}
 show(dac_clip,wanted);for(unsigned i=0;i<6;i++){show(dac_buttons[i],wanted&&(!network||i)&&(!online_music_session()||(i!=3&&i!=4)));show(dac_captions[i],wanted&&(!network||i)&&(!online_music_session()||(i!=3&&i!=4)));}
 if(mode==1||network){show(LV_GLOBAL(0x8deec0),!wanted);show(LV_GLOBAL(0x8deebc),!wanted);}
 if(!wanted&&dac_brightness_shifted){LvObj *bright=LV_GLOBAL(0x8deeb4),*slider=LV_GLOBAL(0x8deeb0);if(bright&&lv_valid(bright)){LvObj *holder=parent(bright);if(holder&&lv_valid(holder)&&dac_holder_height>0)lv_size(holder,360,dac_holder_height);lv_size(bright,340,61);lv_align(bright,2,0,*(unsigned char *)0x8dee8e?0:90);}if(slider&&lv_valid(slider))lv_size(slider,221,23);dac_brightness_shifted=0;}
}
static void usbdac_prepare_menu(void){
 if(!active||!dac_menu_mode()||!menu_open)return;
 ensurefonts();LvObj *shade=LV_GLOBAL(0x8deee4);if(!shade||!lv_valid(shade))return;
 if(shade!=dac_shade){dac_shade=shade;for(unsigned c=1;c<=8;c++)if(c==1||c==2||c==8)lv_add_cb(shade,dac_close_shade,c,0);lv_add_cb(shade,dac_deleted,33,0);}
 if(!dac_clip){dac_clip=lv_create(shade);stylezero(dac_clip);lv_add_flag(dac_clip,0x4000);opa(dac_clip,0,0);lv_size(dac_clip,360,dac_folded?96:194);lv_align(dac_clip,1,0,68);lv_add_cb(dac_clip,dac_deleted,33,0);dac_fold_seen=-1;}
 if(dac_fold_seen!=(online_music_session()?2:dac_folded)){int h=online_music_session()?96:dac_folded?96:194;int *box=(int *)((char *)dac_clip+20);if(dac_fold_seen<0)lv_size(dac_clip,360,h);else animate(dac_clip,menu_height,box[3]-box[1]+1,h);dac_fold_seen=online_music_session()?2:dac_folded;}
 void (*callbacks[6])(void *)={dac_connection_click,open_eq,rotate_click,dac_info_click,dac_normalize_click,open_dsp};
 const void *icons[6]={*(const void **)0x83a704,*(const void **)0x83a6d4,*(const void **)0x83a6f4,&info_icon_0,&normalize_icon_0,&dsp_icon_0};
 const char *names[6]={"电脑连接","均衡器","旋转屏幕",dlna_visual()?"信息显示":visual_source_caption(),"动效归一化","DSP"};
 for(unsigned i=0;i<6;i++)if(!dac_buttons[i]){control(dac_clip,&dac_buttons[i],&dac_icons[i],&dac_captions[i],49+((int)i%3)*99,i<3?0:98,callbacks[i],icons[i],names[i]);lv_add_cb(dac_buttons[i],dac_deleted,33,0);lv_add_cb(dac_captions[i],dac_deleted,33,0);dac_connection_seen=dac_eq_seen=dac_rotation_seen=-1;dac_toggle_icons();}
 if(dac_menu_network_seen!=(online_music_session()?2:dlna_visual())){dac_menu_network_seen=online_music_session()?2:dlna_visual();dac_toggle_icons();for(unsigned i=0;i<6;i++){int slot=online_music_session()?(i==5?2:(int)i-1):dlna_visual()?(int)i-1:(int)i;if(slot<0)continue;int x=49+(slot%3)*99,y=slot<3?0:98;lv_align(dac_buttons[i],1,x,y);lv_align(dac_captions[i],1,x-28,y+66);}}
 unsigned connection=*(unsigned char *)0x8e172c,eq=*(unsigned char *)0x8e1720,rotation=*(unsigned char *)0x8e171b;
 if(dac_connection_seen!=(int)connection){lv_text(dac_captions[0],connection?"手机连接":"电脑连接");src(dac_icons[0],*(const void **)(connection?0x83a70c:0x83a704));dac_connection_seen=(int)connection;}
 if(dac_eq_seen!=(int)eq){src(dac_icons[1],*(const void **)(eq?0x83a6d8:0x83a6d4));dac_eq_seen=(int)eq;}
 if(dac_rotation_seen!=(int)rotation){src(dac_icons[2],*(const void **)(0x83a6f4+(rotation&3)*4));dac_rotation_seen=(int)rotation;}
 LvObj *bright=LV_GLOBAL(0x8deeb4),*slider=LV_GLOBAL(0x8deeb0);
 if(bright&&lv_valid(bright)){
  LvObj *holder=parent(bright);if(holder&&lv_valid(holder)){
   int *hb=(int *)((char *)holder+20),*sb=(int *)((char *)shade+20),*bb=(int *)((char *)bright+20);
   if(holder!=dac_brightness_holder){dac_brightness_holder=holder;dac_holder_height=hb[3]-hb[1]+1;lv_add_cb(holder,dac_close_shade,1,0);lv_add_cb(holder,dac_close_shade,2,0);lv_add_cb(holder,dac_close_shade,8,0);lv_add_cb(holder,dac_deleted,33,0);}
   /* USB DAC follows the unchanged Bluetooth holder-relative spacing.
    * Retain absolute shade coordinates only for the separate DLNA menu. */
   int network=dlna_visual(),bright_state=(online_music_session()?2:dac_folded)+(network?4:0);
   int target=network?(online_music_session()?185:dac_folded?185:283):hb[1]-sb[1]+visual_toolbar_brightness_offset(dac_folded);
   if(dac_bright_seen!=bright_state){if(dac_bright_seen<0)dac_bright_y=target;else animate(bright,dac_bright_move,dac_bright_y,target);dac_bright_seen=bright_state;}
   int y=sb[1]+dac_bright_y-hb[1],height=network?sb[1]+344-hb[1]:280;if(height<dac_holder_height)height=dac_holder_height;
   if(hb[2]-hb[0]+1!=360||hb[3]-hb[1]+1!=height)lv_size(holder,360,height);
   if(bb[2]-bb[0]+1!=280||bb[3]-bb[1]+1!=61)lv_size(bright,280,61);
   if(bb[0]!=sb[0]+40||bb[1]!=sb[1]+dac_bright_y)lv_align(bright,2,0,y);
   dac_brightness_shifted=1;
  }
 }
 if(slider&&lv_valid(slider))lv_size(slider,174,23);
 show(dac_clip,1);for(unsigned i=0;i<6;i++){show(dac_buttons[i],(!dlna_visual()||i)&&(!online_music_session()||(i!=3&&i!=4)));show(dac_captions[i],(!dlna_visual()||i)&&(!online_music_session()||(i!=3&&i!=4)));}
}
