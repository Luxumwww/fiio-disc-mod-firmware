static LvObj *net_audio_info,*net_audio_parent;
static unsigned net_info_checked;static char net_info_text[96];
static void net_info_deleted(void *event){(void)event;net_audio_info=net_audio_parent=0;net_info_text[0]=0;}
static void network_info_poll(unsigned t){
 if(!dlna_visual()||online_music_session()){show(net_audio_info,0);return;}
 LvObj *play=LV_GLOBAL(0x8de290);if(!play||!lv_valid(play))return;
 if(net_audio_parent!=play){
  net_audio_info=lv_label(play);net_audio_parent=play;stylezero(net_audio_info);ensurefonts();smallfont(net_audio_info,font16);
  lv_color(net_audio_info,0xffdddddd,0);textalign(net_audio_info,2,0);lv_size(net_audio_info,320,40);lv_align(net_audio_info,2,0,308);
  lv_clear_flag(net_audio_info,2|16);lv_add_cb(net_audio_info,net_info_deleted,33,0);net_info_text[0]=0;net_info_checked=0;
 }
 show(net_audio_info,net_info_enabled&&!xy_progress);if(!net_info_enabled||t-net_info_checked<500)return;net_info_checked=t;
 discover(t);unsigned player=0,h[40],mp=0,mode=0,rate=0,bits=0,channels=0;char b[96];
 unsigned transport=0;int valid=copy(0x83dfc4,&player,4)&&copy(player,h,sizeof(h))&&copy(0x83dfc0,&mp,4)&&copy(mp,&mode,4)&&disc_dlna_format(h,mode,&rate,&bits,&channels)&&(h[18]==1||h[18]==2||(h[18]==3&&copy(0x89ad98,&transport,4)&&transport==3));
 if(valid){
  if(rate%1000)snprintf(b,sizeof(b),"PCM %u.%u kHz · %ubit 处理\n%u 声道",rate/1000,(rate%1000)/100,bits,channels);
  else snprintf(b,sizeof(b),"PCM %u kHz · %ubit 处理\n%u 声道",rate/1000,bits,channels);
  metarate=rate;metabits=bits;snprintf(metacodec,sizeof(metacodec),"DLNA PCM");
 }else{snprintf(b,sizeof(b),"等待手机投放\n连接同一 Wi-Fi");metarate=metabits=0;}
 if(strcmp(b,net_info_text)){snprintf(net_info_text,sizeof(net_info_text),"%s",b);lv_text(net_audio_info,b);}
}
