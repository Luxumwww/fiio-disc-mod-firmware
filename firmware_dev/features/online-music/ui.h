#include "playback_clock.h"
static OnlineClock online_clock;
static char online_drawn_time[320];static unsigned online_drawn_progress=~0u;
static int online_drawn_toggle=-1;
static int online_drawn_lyric=-999;static unsigned online_drawn_lyric_revision,online_frame_at;
extern int sscanf(const char *,const char *,...);
/* Native screen, file IPC only: no provider/network work on the UI thread. */
static LvObj *online_rows[3],*online_page,*online_origin,*online_status,*online_query_label;
static LvObj *online_actions[96];
static unsigned online_action_count,online_seq,online_response_at,online_generation;
static int online_open_pending,online_back_pending,online_view,online_source,online_pending_action=-1;
static char online_query[181];
static const char *online_sources[3]={"netease","qq","kugou"};
static const char *online_names[3]={"网易云音乐","QQ 音乐","酷狗音乐"};
static unsigned online_login_at;static int online_login_poll;
static unsigned char online_qr_pixels[196*196*4];
static struct {unsigned header,size;const void *data;} online_qr_desc={5u|(196u<<10)|(196u<<21),196u*196u*4u,online_qr_pixels};
static char online_results[30][804];
static unsigned online_count;
static unsigned online_transport_at,online_transport_revision;static int online_transport_pending;
static unsigned online_list_generation,online_list_page=1,online_track_page=1,online_selected_list;
static int online_more,online_list_favorite;
static char online_collection_title[320],online_list_title[320],online_feedback[320];
static unsigned online_feedback_until;
static char online_drawn_title[320],online_drawn_artist[320],online_drawn_mini_title[320],online_drawn_mini_artist[360];
static LvObj *online_mode_button;
static char online_play_name[320],online_play_artist[320],online_play_album[320],online_play_state[32],online_play_mode[16],online_quality[16],online_format[16],online_download_message[320];
static unsigned online_play_revision,online_position,online_duration,online_rate,online_bits,online_selected_mask,online_cover_revision,online_play_at;
static int online_library_recommend,online_resume_pending,online_player_origin_view=0,online_account_saved;
static LvObj *online_play_title,*online_play_artist_label,*online_play_time,*online_play_toggle,*online_play_format,*online_cover,*online_mini,*online_mini_title,*online_mini_artist,*online_mini_parent;
static LvObj *online_thumbs[30];static unsigned char online_thumb_pixels[30][48*48*4];static struct {unsigned header,size;const void *data;} online_thumb_desc[30];static unsigned online_thumb_loaded;
static unsigned char online_cover_pixels[144*144*4];
static struct {unsigned header,size;const void *data;} online_cover_desc={5u|(144u<<10)|(144u<<21),144u*144u*4u,online_cover_pixels};
static void online_player_refresh(void);
static void online_miniplayer(void);
static void online_settings_refresh(void){if(online_view!=9||!online_status)return;char info[160],rate_text[48],bits_text[48];if(online_rate)snprintf(rate_text,48,"%u.%u kHz",online_rate/1000,online_rate%1000/100);else snprintf(rate_text,48,"采样率未知");if(online_bits)snprintf(bits_text,48,"%u bit",online_bits);else snprintf(bits_text,48,!strcmp(online_format,"MP3")||!strcmp(online_format,"M4A")||!strcmp(online_format,"OGG")?"压缩音频 · 无源PCM位深":"源位深未知");snprintf(info,160,"%s · %s\n%s",online_format,rate_text,bits_text);lv_text(online_status,info);if(online_mode_button)lv_text(child(online_mode_button,0),!strcmp(online_play_mode,"shuffle")?"随机播放":!strcmp(online_play_mode,"repeat")?"单曲循环":"顺序播放");}

#include "ime_ui.h"
#include "presentation.h"
static void online_make(int kind);
static void online_clear(void);
static void online_keyboard(void);
static int online_library_send(const char *action,unsigned page);
static int online_send(const char *,unsigned,unsigned);
static int online_command(const char *,const char *,const char *);
static void online_virtual_refresh(int);
static void online_restore_catalog(int);
static void online_list_continue(void);
extern void *realloc(void *,__SIZE_TYPE__);
typedef struct {unsigned generation,index,page;char text[804];} OnlineItem;
typedef struct {OnlineItem *items;unsigned count,capacity,page,total,first;int more,source,favorite,recommend,scroll;char title[320];} OnlineCatalog;
static OnlineCatalog online_catalogs[2];
static int online_loading_more,online_load_failed;
static LvObj *online_row_title[30],*online_row_detail[30],*online_list_tail,*online_list_batch;
static unsigned online_row_generation[30],online_row_index[30],online_batch_generation[30],online_batch_index[30],online_art_generation,online_art_generation_next;
static unsigned online_more_generation,online_more_index;
static char online_more_name[804];
static int online_download_pending,online_download_current,online_download_return_view;
static char online_download_items[720],online_drawn_download[320];
static OnlineCatalog *online_catalog(void){return online_view==4?&online_catalogs[0]:online_view==5?&online_catalogs[1]:0;}
static int online_scroll_position(void){if(!online_page)return 0;void *spec=*(void **)((char *)online_page+8);return spec?-*(int *)((char *)spec+0x14):0;}



static LvObj *online_progress,*online_progress_fill,*online_lyric_labels[5],*online_toggle_image;
static int online_exit_waiting;static int online_exit_pending,online_swipe_page,online_song_more=-1;
static unsigned online_queue_revision,online_lyrics_revision,online_lyrics_count;
static int online_lyrics_times[600];static char online_lyrics_text[600][320];
static struct {int valid,view,scroll,more,favorite,recommend;unsigned count,generation,list_generation,list_page,track_page,selected_list;char title[320],list_title[320],rows[30][804];} online_browser;
static void online_save_browser(void){if(online_view!=0&&online_view!=2&&online_view!=4&&online_view!=5&&online_view!=13)return;online_browser.valid=1;online_browser.view=online_view;online_browser.count=online_count;online_browser.generation=online_generation;online_browser.more=online_more;online_browser.favorite=online_list_favorite;online_browser.recommend=online_library_recommend;online_browser.list_generation=online_list_generation;online_browser.list_page=online_list_page;online_browser.track_page=online_track_page;online_browser.selected_list=online_selected_list;snprintf(online_browser.title,320,"%s",online_collection_title);snprintf(online_browser.list_title,320,"%s",online_list_title);for(unsigned i=0;i<online_count;i++)snprintf(online_browser.rows[i],804,"%s",online_results[i]);void *spec=*(void **)((char *)online_page+8);online_browser.scroll=spec?-*(int *)((char *)spec+0x14):0;OnlineCatalog *cat=online_catalog();if(cat)cat->scroll=online_browser.scroll;}
static void online_return_browser(void){if(!online_browser.valid){online_make(0);return;}online_count=online_browser.count;online_generation=online_browser.generation;online_more=online_browser.more;online_list_favorite=online_browser.favorite;online_library_recommend=online_browser.recommend;online_list_generation=online_browser.list_generation;online_list_page=online_browser.list_page;online_track_page=online_browser.track_page;online_selected_list=online_browser.selected_list;snprintf(online_collection_title,320,"%s",online_browser.title);snprintf(online_list_title,320,"%s",online_browser.list_title);for(unsigned i=0;i<online_count;i++)snprintf(online_results[i],804,"%s",online_browser.rows[i]);if(online_browser.view==4||online_browser.view==5)online_restore_catalog(online_browser.view);else {online_make(online_browser.view);FN(0x4f15ac,void (*)(LvObj *,int,int,int))(online_page,0,-online_browser.scroll,0);}online_virtual_refresh(1);}
static __typeof__(online_browser) online_queue_parent;
static int online_queue_restore_scroll=-1;
static void online_exit_result(unsigned yes){if(yes)online_exit_pending=1;else if(online_page)screen_load(online_page);}
static void online_request_exit(void){FN(0x483170,void (*)(void (*)(unsigned),unsigned))(online_exit_result,4);LvObj *dialog=LV_GLOBAL(0x8def44);if(dialog&&lv_valid(dialog)){LvObj *label=child(dialog,0);if(label)lv_text(label,"退出在线音乐？\n将停止当前播放");}}
static void online_finish_exit(void){
 unlink("/tmp/disc-online-music/session");unlink("/tmp/disc-online-music/active");online_play_revision=online_queue_revision=0;online_clock.valid=0;online_loading_more=0;online_browser.valid=0;online_resume_pending=0;
 if(*(unsigned char *)0x8e1729==12)FN(0x4775a8,void (*)(unsigned))(1);
 else if(online_origin&&lv_valid(online_origin))screen_load(online_origin);
 online_clear();
}
static int online_read(const char *path,char *b,unsigned cap){int fd=open(path,0);if(fd<0)return 0;int n=read(fd,b,cap-1);close(fd);if(n<0)return 0;b[n]=0;return n;}
static void online_deleted(void *e){LvObj *target=lv_event_target(e);for(unsigned i=0;i<3;i++)if(target==online_rows[i])online_rows[i]=0;if(target==online_origin)online_origin=0;if(target==online_page){online_play_title=online_play_artist_label=online_play_time=online_play_toggle=online_play_format=online_cover=0;online_mode_button=0;online_drawn_title[0]=online_drawn_artist[0]=online_drawn_time[0]=0;online_drawn_progress=~0u;online_drawn_lyric=-999;online_drawn_toggle=-1;online_thumb_loaded=0;for(unsigned k=0;k<30;k++)online_thumbs[k]=0;online_progress=online_progress_fill=online_toggle_image=0;for(unsigned k=0;k<5;k++)online_lyric_labels[k]=0;online_page=online_status=online_query_label=0;online_actions[95]=0;online_action_count=0;online_list_tail=online_list_batch=0;for(unsigned k=0;k<30;k++)online_row_title[k]=online_row_detail[k]=0;online_back_pending=0;online_login_poll=0;}}
static int online_touch_x,online_touch_y,online_touch_active,online_shade_pending;
static void online_swipe(void *e){int r=FN(0x488860,int (*)(void *))(e);if(r==3&&((online_view!=8&&online_view!=12)||online_touch_x<36))online_back_pending=1;
 unsigned code=lv_event_code(e);int pt[2];void *input=indev_active();if(!input)return;indev_point(input,pt);
 if(code==1){online_touch_x=pt[0];online_touch_y=pt[1];online_touch_active=1;}else if(code==8&&online_touch_active){online_touch_active=0;int dx=pt[0]-online_touch_x,dy=pt[1]-online_touch_y;if(online_touch_y<40&&dy>45&&dy*dy>dx*dx)online_shade_pending=1;else if((online_view==8||online_view==12)&&online_touch_x>=36&&dx*dx>dy*dy&&dx*dx>2500){if(online_view==8&&dx<0)online_swipe_page=12;else if(online_view==12&&dx>0)online_swipe_page=8;}}
}
static void online_more_click(void *e){if(!stationary_clicked(e))return;LvObj *o=lv_event_target(e);unsigned row=(unsigned)(unsigned long)FN(0x4e394c,void *(*)(void *))(e);(void)o;online_song_more=(int)row;online_pending_action=200+(int)row;}
static void online_click(void *e){if(!stationary_clicked(e))return;LvObj *o=lv_event_target(e);if(o==online_actions[95]){online_pending_action=100;return;}for(unsigned i=0;i<online_action_count;i++)if(o==online_actions[i]){online_pending_action=(int)i;return;}}
static void online_confirmed(void){online_open_pending=1;}
static void online_enter(void *e){if(!stationary_clicked(e))return;for(unsigned i=0;i<3;i++)if(lv_event_target(e)==online_rows[i])online_source=(int)i;if(FN(0x46f290,int (*)(void))()!=17){FN(0x46fdac,void (*)(void (*)(void)))(FN(0x474838,void (*)(void)));return;}
 if(*(unsigned char *)0x8e171c){FN(0x46b7a0,void (*)(void))();FN(0x459f20,void (*)(unsigned))(1);FN(0x459f14,void (*)(void (*)(void)))(online_confirmed);FN(0x45a940,void (*)(void))();FN(0x459f20,void (*)(unsigned))(0);}else online_confirmed();}
static void online_clear(void){if(!online_page)return;LvObj *old=online_page;online_page=0;screen_delete(&old);online_play_title=online_play_artist_label=online_play_time=online_play_toggle=online_play_format=online_cover=0;online_mode_button=0;online_drawn_title[0]=online_drawn_artist[0]=online_drawn_time[0]=0;online_drawn_progress=~0u;online_drawn_lyric=-999;online_drawn_toggle=-1;online_thumb_loaded=0;for(unsigned k=0;k<30;k++)online_thumbs[k]=0;online_progress=online_progress_fill=online_toggle_image=0;for(unsigned k=0;k<5;k++)online_lyric_labels[k]=0;online_status=online_query_label=0;online_action_count=0;online_actions[95]=0;online_pending_action=-1;online_list_tail=online_list_batch=0;for(unsigned k=0;k<30;k++)online_row_title[k]=online_row_detail[k]=0;}
static LvObj *online_button(const char *s,int x,int y,int width,int height){LvObj *b=lv_btn(online_page);lv_size(b,width,height);lv_align(b,1,x,y);lv_bg(b,0xff1b2635,0);lv_border(b,0,0);lv_radius(b,12,0);pad_top(b,0,0);pad_bottom(b,0,0);pad_left(b,0,0);pad_right(b,0,0);LvObj *t=lv_label(b);online_font(t,18);lv_text(t,s);lv_size(t,width-4,0x200007d1);FN(0x52f4f0,void (*)(LvObj *,int))(t,4);FN(0x4f6e50,void (*)(LvObj *,unsigned,unsigned))(t,2,0);lv_color(t,0xffffffff,0);lv_align(t,9,0,0);lv_clear_flag(t,2|16);protect_tap(b);lv_add_cb(b,online_click,7,0);online_actions[online_action_count++]=b;return b;}
#include "list_ui.h"
static void online_make(int kind){
 online_clear();online_view=kind;if(kind==4||kind==5){OnlineCatalog *cat=online_catalog();cat->first=~0u;online_count=30;for(unsigned k=0;k<30;k++)online_results[k][0]=0;}online_page=lv_create(0);if(!online_page)return;screen_register(online_page);lv_add_cb(online_page,online_deleted,33,0);
 lv_size(online_page,360,360);lv_border(online_page,0,0);lv_bg(online_page,0xff080c12,0);pad_top(online_page,0,0);pad_bottom(online_page,24,0);pad_left(online_page,0,0);pad_right(online_page,0,0);
 if(kind==0||kind==2||kind==4||kind==5||kind==9||kind==10||kind==11||kind==13){FN(0x4f0268,void (*)(LvObj *,int))(online_page,0);FN(0x4f02c8,void (*)(LvObj *,int))(online_page,12);}else lv_clear_flag(online_page,16);
 const char *heading=kind==1?"搜索音乐":kind==2?"搜索结果":kind==3?"扫码登录":kind==4?online_list_title:kind==5?online_collection_title:kind==6?"账号管理":kind==7?"退出登录":kind==8||kind==12?"":kind==9?"播放设置":kind==10?"批量下载":kind==11?"音质选择":kind==13?"当前播放队列":kind==17?"下载歌曲":kind==14?"歌曲操作":kind==15?"正在加载":online_names[online_source];
 // Keep a first header child so the preserved keyboard/login layout remains safe.
 LvObj *header=online_label(online_page,heading,64,34,232,32,22,0xfff4f6fa,1);
 if(kind==8||kind==12)lv_add_flag(header,1);
 lv_add_cb(online_page,online_swipe,1,0);lv_add_cb(online_page,online_swipe,2,0);lv_add_cb(online_page,online_swipe,8,0);
 online_status=lv_label(online_page);lv_add_style(online_status,(void *)0x83ac5c,0);lv_size(online_status,288,52);FN(0x52f4f0,void (*)(LvObj *,int))(online_status,0);lv_color(online_status,0xffaaaaaa,0);lv_align(online_status,2,0,kind==2||kind==4||kind==5?78:290);lv_clear_flag(online_status,2|16);
 if(kind==0){
  lv_add_flag(online_status,1);
  LvObj *search=online_button("",270,62,44,44);lv_radius(search,22,0);online_image(search,&online_icon_search,6,6);
  LvObj *liked=online_button("",30,186,300,68);online_card_text(liked,"我喜欢的音乐","收藏的歌曲",&online_icon_heart,0xffdc354f);
  LvObj *lists=online_button("",30,262,300,68);online_card_text(lists,"我的歌单","创建与收藏的歌单",&online_icon_queue,0xff276ee0);
  LvObj *recommend=online_button("",30,110,300,68);lv_bg(recommend,0xff321b25,0);online_card_text(recommend,"推荐歌单","发现更多音乐",&online_icon_music,0xffe93648);
  LvObj *account=online_button(online_account_saved?"已登录  >":"扫码登录  >",100,72,150,30);lv_bg(account,0xff080c12,0);online_font(child(account,0),16);lv_color(child(account,0),0xffafb9ca,0);
  LvObj *current=online_button("",30,338,300,68);online_card_text(current,"正在播放","点此展开播放页",&online_icon_play,0xffd62e47);
  online_button("退出在线音乐",90,420,180,44);
 }else if(kind==8||kind==12){
  lv_add_flag(online_status,1);
  online_play_title=online_label(online_page,online_play_name,66,26,228,32,22,0xfff4f6fa,1);
  online_play_artist_label=online_label(online_page,online_play_artist,74,62,212,24,16,0xffaab4c6,1);
  online_marquee(online_play_title,online_play_name,228);online_marquee(online_play_artist_label,online_play_artist,212);
  if(kind==8){
   online_cover=online_image(online_page,(const OnlineImage *)&online_cover_desc,108,88);
   LvObj *left=online_button("",26,146,44,44);lv_radius(left,22,0);online_image(left,&online_icon_circle,6,6);
   LvObj *queue=online_button("",290,146,44,44);lv_radius(queue,22,0);online_image(queue,&online_icon_queue,6,6);
   online_progress=lv_create(online_page);online_button_style(online_progress);lv_size(online_progress,200,4);lv_radius(online_progress,2,0);lv_align(online_progress,1,80,242);lv_clear_flag(online_progress,2|16);
   online_progress_fill=lv_create(online_progress);online_button_style(online_progress_fill);lv_size(online_progress_fill,1,4);lv_align(online_progress_fill,1,0,0);lv_bg(online_progress_fill,0xfffa3b55,0);lv_clear_flag(online_progress_fill,2|16);
   online_play_time=online_label(online_page,"",90,252,180,20,14,0xffbec6d5,1);
   LvObj *prev=online_button("",76,280,48,48);lv_radius(prev,24,0);online_image(prev,&online_icon_prev,8,8);
   online_play_toggle=online_button("",150,272,60,60);lv_radius(online_play_toggle,30,0);lv_bg(online_play_toggle,0xffed304a,0);online_toggle_image=online_image(online_play_toggle,&online_icon_pause,14,14);
   LvObj *next=online_button("",236,280,48,48);lv_radius(next,24,0);online_image(next,&online_icon_next,8,8);
   /* Quality remains available in playback settings; no overlapping bottom pill. */
  }else{
   for(unsigned j=0;j<5;j++)online_lyric_labels[j]=online_label(online_page,"",42,112+(int)j*38,276,32,j==2?22:18,j==2?0xfffafafa:0xff929cac,1);
   online_label(online_page,"右滑返回封面",110,314,140,22,14,0xff6f7b8e,1);
  }
  online_player_refresh();
 }else if(kind==9){
  lv_align(online_status,2,0,86);lv_size(online_status,260,48);online_font(online_status,16);FN(0x4f6e50,void (*)(LvObj *,int,unsigned))(online_status,2,0);
  online_mode_button=online_button(!strcmp(online_play_mode,"shuffle")?"随机播放":!strcmp(online_play_mode,"repeat")?"单曲循环":"顺序播放",56,154,248,46);
  online_button("选择音质",76,212,208,46);online_button("下载当前歌曲",76,270,208,46);
 }else if(kind==11){
  lv_align(online_status,2,0,78);lv_size(online_status,260,36);lv_text(online_status,"切换当前歌曲 · 以实际音源为准");
  const char *names[5]={"自动 · 最高可用","标准","高品质","无损 FLAC","Hi-Res"};for(unsigned i=0;i<5;i++){LvObj *q=online_button(names[i],62,112+(int)i*54,236,42);const char *values[5]={"auto","standard","exhigh","lossless","hires"};if(!strcmp(values[i],online_quality)){lv_bg(q,0xff56202e,0);online_image(q,&online_icon_check,204,5);}}online_font(online_status,16);lv_align(online_status,2,0,390);lv_text(online_status,"歌曲不支持时自动降档");
 }else if(kind==1){online_keyboard();
 }else if(kind==6){
  lv_align(online_status,2,0,96);lv_size(online_status,260,48);
  online_button("扫码登录 / 更新登录",44,166,272,48);
  online_button("退出登录",76,232,208,44);
  lv_text(online_status,"登录信息保存在播放器");
 }else if(kind==7){
  lv_align(online_status,2,0,114);lv_text(online_status,"退出后需要重新扫码");
  online_button("保留登录",54,202,120,46);online_button("确认退出",186,202,120,46);
 }else if(kind==3){
  int f=open("/tmp/disc-online-music/login.bgra",0);unsigned n=0;if(f>=0){while(n<sizeof(online_qr_pixels)){int k=read(f,online_qr_pixels+n,sizeof(online_qr_pixels)-n);if(k<=0)break;n+=(unsigned)k;}close(f);}if(n==sizeof(online_qr_pixels)){LvObj *qr=FN(0x52cb10,LvObj *(*)(LvObj *))(online_page);FN(0x52cb44,void (*)(LvObj *,const void *))(qr,&online_qr_desc);lv_align(qr,2,0,76);lv_clear_flag(qr,2|16);}
  lv_align(child(online_page,0),1,64,30);
  online_button("刷新二维码",114,310,132,28);
  lv_align(online_status,2,0,276);lv_size(online_status,250,30);FN(0x52f4f0,void (*)(LvObj *,int))(online_status,4);lv_text(online_status,"等待扫码");
 }else if(kind==15){lv_align(online_status,2,0,158);lv_size(online_status,260,64);online_font(online_status,18);FN(0x4f6e50,void (*)(LvObj *,int,unsigned))(online_status,2,0);
 }else if(kind==17){
  lv_align(online_status,2,0,108);lv_size(online_status,274,126);online_font(online_status,18);lv_text(online_status,"正在准备下载…");
  const char *dirs[3]={"网易云","QQ音乐","酷狗音乐"};char target[120];snprintf(target,120,"流媒体下载/%s",dirs[online_source]);online_label(online_page,target,48,244,264,26,16,0xff9aa8be,1);online_button("返回",114,286,132,40);online_drawn_download[0]=0;
 }else if(kind==14){
  lv_align(online_status,2,0,88);lv_text(online_status,online_more_name[0]?online_more_name:"当前歌曲");
  online_button("播放这首歌",70,168,220,48);online_button("下载这首歌",70,226,220,48);online_button("批量选择下载",70,284,220,48);
 }else{
  if(kind==13){lv_align(online_status,2,0,76);online_font(online_status,14);lv_text(online_status,"点歌曲切换播放");}
  if(kind==5||kind==2){LvObj *all=online_button("播放全部",100,104,160,38);lv_bg(all,0xffad243c,0);online_actions[--online_action_count]=0;online_actions[95]=all;}
  for(unsigned i=0;i<online_count;i++){
   char title[404],artist[404];unsigned j=0;while(online_results[i][j]&&online_results[i][j]!='\n'&&j<sizeof(title)-1){title[j]=online_results[i][j];j++;}title[j]=0;snprintf(artist,sizeof(artist),"%s",online_results[i][j]=='\n'?online_results[i]+j+1:"");
   LvObj *b=online_button("",30,(kind==5||kind==2?154:108)+(int)i*72,300,64);lv_bg(b,kind==10&&(online_selected_mask&(1u<<i))?0xff214e68:0xff141b25,0);lv_add_flag(b,0x4000);
   LvObj *label=child(b,0);online_font(label,20);lv_size(label,184,27);lv_align(label,1,68,7);FN(0x4f6e50,void (*)(LvObj *,int,unsigned))(label,0,0);lv_text(label,title);online_row_title[i]=label;if(kind!=4)online_marquee(label,title,184);
   LvObj *detail=lv_label(b);online_font(detail,16);lv_color(detail,0xffa0aabb,0);lv_size(detail,184,22);FN(0x52f4f0,void (*)(LvObj *,int))(detail,4);lv_align(detail,1,68,36);lv_text(detail,artist);online_row_detail[i]=detail;if(kind!=4)online_marquee(detail,artist,184);lv_clear_flag(detail,2|16);
   if(kind!=10){online_image(b,&online_icon_music,20,16);online_thumb_desc[i].header=5u|(48u<<10)|(48u<<21);online_thumb_desc[i].size=48u*48u*4u;online_thumb_desc[i].data=online_thumb_pixels[i];online_thumbs[i]=online_image(b,(const OnlineImage *)&online_thumb_desc[i],10,8);lv_add_flag(online_thumbs[i],1);}

   if(kind==2||kind==5){LvObj *more=lv_create(b);online_button_style(more);lv_add_flag(more,2);lv_radius(more,0,0);lv_size(more,40,52);lv_align(more,1,256,6);lv_bg(more,0xff141b25,0);online_image(more,&online_icon_more,4,10);protect_tap(more);lv_clear_flag(more,16|0x4000);lv_add_cb(more,online_more_click,7,(void *)(unsigned long)i);}
  }
  if(kind==10){int y=(kind==5||kind==2?152:120)+(int)online_count*72;online_button("全选 / 取消",42,y,132,40);online_button("开始下载",186,y,132,40);lv_text(online_status,"选择选择歌曲歌曲 · 保存到 TF 卡");}
  else if(kind==4||kind==5){lv_align(online_status,2,0,74);online_font(online_status,14);lv_size(online_status,260,24);FN(0x52f4f0,void (*)(LvObj *,int))(online_status,4);lv_text(online_status,"");online_list_tail=online_button("",80,120,200,40);if(kind==5)online_list_batch=online_button("批量下载",106,170,148,40);}
  else if(kind!=13){lv_align(online_status,2,0,74);online_font(online_status,14);lv_size(online_status,260,24);lv_text(online_status,online_count?"":"没有找到歌曲");if(kind==2&&online_count)online_button("批量选择下载",76,(kind==5||kind==2?152:120)+(int)online_count*72,208,40);}
 }
 online_settings_refresh();online_virtual_refresh(1);screen_load(online_page);
}
static void online_keyboard(void){
 lv_align(child(online_page,0),1,64,26);
 online_query_label=lv_label(online_page);lv_add_style(online_query_label,(void *)0x83ac5c,0);lv_size(online_query_label,270,32);FN(0x52f4f0,void (*)(LvObj *,int))(online_query_label,1);lv_align(online_query_label,2,0,62);lv_text(online_query_label,online_query[0]?online_query:"输入歌曲或歌手");
 online_ime_update();lv_align(online_status,2,0,94);lv_size(online_status,260,28);lv_text(online_status,online_compose[0]?online_compose:online_input_mode==0?"拼音输入":online_input_mode==1?"英文输入":"数字与符号");
 const char *keys=online_input_mode==2?"1234567890-/'&.,!?()@#+=:_":"qwertyuiopasdfghjklzxcvbnm";unsigned count=26;
 for(unsigned i=0;i<count;i++){char t[2]={keys[i],0};int row=i<10?0:i<19?1:2,col=i<10?(int)i:i<19?(int)i-10:(int)i-19;int n=row==0?10:row==1?9:(int)count-19;online_button(t,(360-n*35)/2+col*35,164+row*42,33,40);}
 /* Reserved key slots keep candidate/button IDs stable in every panel. */
 
 online_button(online_input_mode==0?"中文":online_input_mode==1?"英文":"123",86,300,46,28);online_button("删除",134,300,46,28);online_button("空格",182,300,46,28);online_button("搜索",230,300,46,28);
 online_button("<",16,126,26,32);for(unsigned i=0;i<3;i++){LvObj *candidate=online_button(online_candidates[i],44+(int)i*82,124,80,34);if(!online_candidates[i][0])lv_clear_flag(candidate,2);}online_button(">",294,126,26,32);
}
static int online_library_send(const char *action,unsigned pg){char b[256];online_seq=milliseconds();if(!online_seq)online_seq=1;int n=!strcmp(action,"tracks")?snprintf(b,sizeof(b),"%u\ttracks\t%u\t%u,%u\n",online_seq,online_list_generation,online_selected_list,pg):snprintf(b,sizeof(b),"%u\t%s\t%s\t%u\n",online_seq,action,online_sources[online_source],pg);if(n<0||(unsigned)n>=sizeof(b)){online_seq=0;return 0;}int f=open("/tmp/disc-online-music/request.next",1|0x100|0x200,0600);if(f<0){online_seq=0;return 0;}int ok=write(f,b,n)==n;close(f);return ok&&!rename("/tmp/disc-online-music/request.next","/tmp/disc-online-music/request");}
static int online_send(const char *action,unsigned gen,unsigned index){char b[512];online_seq=milliseconds();if(!online_seq)online_seq=1;int n=!strcmp(action,"search")?snprintf(b,sizeof(b),"%u\tsearch\t%s\t%s\n",online_seq,online_sources[online_source],online_query):!strcmp(action,"play")?snprintf(b,sizeof(b),"%u\tplay\t%u\t%u\n",online_seq,gen,index):snprintf(b,sizeof(b),"%u\t%s\t%s\n",online_seq,action,online_sources[online_source]);if(n<0||(unsigned)n>=sizeof(b)){online_seq=0;return 0;}int f=open("/tmp/disc-online-music/request.next",1|0x100|0x200,0600);if(f<0){online_seq=0;return 0;}int ok=write(f,b,n)==n;close(f);return ok&&!rename("/tmp/disc-online-music/request.next","/tmp/disc-online-music/request");}
static int online_command(const char *action,const char *arg,const char *value){char b[1536];online_seq=milliseconds();if(!online_seq)online_seq=1;int n=snprintf(b,sizeof(b),"%u\t%s\t%s\t%s\n",online_seq,action,arg?arg:"",value?value:"");if(n<0||(unsigned)n>=sizeof(b)){online_seq=0;return 0;}int f=open("/tmp/disc-online-music/request.next",1|0x100|0x200,0600);if(f<0){online_seq=0;return 0;}int ok=write(f,b,n)==n;close(f);return ok&&!rename("/tmp/disc-online-music/request.next","/tmp/disc-online-music/request");}
static void online_search(void){if(!online_query[0]){lv_text(online_status,"请输入关键词");return;}if(!online_send("search",0,0)){lv_text(online_status,"后台不可用");return;}lv_text(online_status,"正在搜索…");}
static void online_response(void){char b[32768];if(!online_read("/tmp/disc-online-music/response",b,sizeof(b)))return;char *p=b,*end;unsigned seq=(unsigned)strtoull(p,&end,10);if(end==p||*end!='\t'||seq!=online_seq)return;p=end+1;char *kind=p;while(*p&&*p!='\t')p++;if(!*p)return;*p++=0;unsigned gen=(unsigned)strtoull(p,&end,10);if(end==p||*end!='\t')return;p=end+1;char *message=p;while(*p&&*p!='\n')p++;if(*p)*p++=0;
 if(!strcmp(kind,"results")||!strcmp(kind,"lists")||!strcmp(kind,"tracks")){
  int library_kind=!strcmp(kind,"lists")?4:!strcmp(kind,"tracks")?5:2;
  if(library_kind!=2){unsigned pg=0,total=0;char more[8];unsigned list_gen=0,list_index=0;int fields=sscanf(p,"@\t%u\t%7s\t%u\t%u\t%u",&pg,more,&list_gen,&list_index,&total);if(fields<2)return;if(library_kind==5&&fields>=4&&list_gen){online_list_generation=list_gen;online_selected_list=list_index;}online_more=!strcmp(more,"true");online_response_total=total;if(library_kind==4){online_list_generation=gen;online_list_page=pg;snprintf(online_list_title,sizeof(online_list_title),"%s",message);}else{online_track_page=pg;snprintf(online_collection_title,sizeof(online_collection_title),"%s",message);}while(*p&&*p!='\n')p++;if(*p)p++;}

  online_generation=gen;online_count=0;while(*p&&online_count<30){char *line=p;while(*p&&*p!='\n')p++;if(*p)*p++=0;char *q=line;while(*q&&*q!='\t')q++;if(!*q)break;q++;for(char *t=q;*t;t++)if(*t=='\t')*t='\n';snprintf(online_results[online_count],sizeof(online_results[0]),"%s",q);online_count++;}online_seq=0;if(library_kind==4||library_kind==5){online_catalog_accept(library_kind,gen,message);return;}online_make(library_kind);
 }else if(!strcmp(kind,"queue")){online_queue_revision=gen;online_count=0;while(*p&&*p!='\n')p++;if(*p)p++;while(*p&&online_count<30){char *line=p;while(*p&&*p!='\n')p++;if(*p)*p++=0;char *q=line;while(*q&&*q!='\t')q++;if(!*q)break;q++;for(char *t=q;*t;t++)if(*t=='\t')*t='\n';snprintf(online_results[online_count++],804,"%s",q);}online_seq=0;online_make(13);if(online_queue_restore_scroll>=0){FN(0x4f15ac,void (*)(LvObj *,int,int,int))(online_page,0,-online_queue_restore_scroll,0);online_queue_restore_scroll=-1;}
 }else if(!strcmp(kind,"art-ready")){online_seq=0;
 }else if(!strcmp(kind,"download-started")){online_seq=0;if(online_view!=17)online_make(17);
 }else if(!strcmp(kind,"exited")){online_seq=0;online_exit_waiting=0;online_finish_exit();
 }else if(!strcmp(kind,"qr")){online_seq=0;online_make(3);online_login_poll=1;online_login_at=milliseconds();
 }else if(!strcmp(kind,"waiting")){online_seq=0;if(online_status)lv_text(online_status,online_view==3?(!strcmp(message,"已扫码，请在手机确认")?"请在手机确认":!strcmp(message,"正在处理，请稍后")?"正在处理…":"等待扫码"):message);
 }else if(!strcmp(kind,"logged-in")){online_seq=0;online_login_poll=0;online_account_saved=1;online_make(0);
 }else if(!strcmp(kind,"account")||!strcmp(kind,"expired")||!strcmp(kind,"logged-out")){online_seq=0;online_login_poll=0;if(!strcmp(kind,"logged-out")){online_account_saved=0;online_make(0);}else if(!strcmp(kind,"account")){online_account_saved=!strcmp(message,"已保存登录");if(online_view==0){LvObj *label=child(online_actions[4],0);lv_text(label,online_account_saved?"已登录  >":"扫码登录  >");}}if(online_status)lv_text(online_status,online_view==3&&!strcmp(kind,"expired")?"已过期，请刷新":message);
 }else if(!strcmp(kind,"playing")){
  online_transport_pending=0;online_seq=0;online_player_origin_view=online_browser.valid?online_browser.view:0;online_make(8);
 }else if(!strcmp(kind,"controlled")||!strcmp(kind,"downloaded")){online_transport_pending=0;online_seq=0;snprintf(online_feedback,sizeof(online_feedback),"%s",message);online_feedback_until=milliseconds()+4000;if(online_view==11)online_make(8);if(online_status)lv_text(online_status,message);
 }else if(!strcmp(kind,"error")&&online_loading_more){online_loading_more=0;online_load_failed=1;online_seq=0;if(online_list_tail)lv_text(child(online_list_tail,0),"加载失败 · 点此重试");
 }else if(online_status){if(!strcmp(kind,"error")&&online_view==15){online_return_browser();if(online_status)lv_clear_flag(online_status,1);}lv_text(online_status,message);if(!strcmp(kind,"error")){online_transport_pending=0;snprintf(online_feedback,sizeof(online_feedback),"%s",message);online_feedback_until=milliseconds()+6000;online_seq=0;online_login_poll=0;if(online_exit_waiting){online_exit_waiting=0;online_make(0);lv_clear_flag(online_status,1);lv_text(online_status,message);}}}
}
static void online_shade_animate(LvObj *o,int target){unsigned a[32];FN(0x51b3e0,void (*)(void *))(a);a[0]=(unsigned)o;a[1]=0x4eb600;a[8]=a[9]=*(unsigned *)((char *)o+24);a[10]=(unsigned)target;a[11]=250;FN(0x51b7b0,void *(*)(void *))(a);}
static void online_time_refresh(void){
 if(online_transport_pending&&online_play_time){lv_text(online_play_time,"正在切换…");return;}
 if((online_view!=8&&online_view!=12)||!online_play_time)return;
 unsigned pos=online_clock.valid?online_clock_position(&online_clock,milliseconds(),online_duration):online_position*1000u;
 char text[320];unsigned seconds=pos/1000u;
 snprintf(text,sizeof(text),"%u:%02u / %u:%02u  %s",seconds/60,seconds%60,online_duration/60,online_duration%60,online_format);
 const char *shown=milliseconds()<online_feedback_until?online_feedback:text;
 if(strcmp(online_drawn_time,shown)){lv_text(online_play_time,shown);snprintf(online_drawn_time,320,"%s",shown);}
 if(online_progress_fill){unsigned width=online_duration?pos/5u/online_duration:0;if(width>200)width=200;if(width!=online_drawn_progress){online_drawn_progress=width;lv_size(online_progress_fill,width?width:1,4);}}
}
static void online_player_refresh(void){
 if((online_view!=8&&online_view!=12)||!online_play_title)return;
 const char *title=online_play_name[0]?online_play_name:"正在连接歌曲…";if(strcmp(online_drawn_title,title)){lv_text(online_play_title,title);online_marquee(online_play_title,title,228);snprintf(online_drawn_title,320,"%s",title);}if(strcmp(online_drawn_artist,online_play_artist)){lv_text(online_play_artist_label,online_play_artist);online_marquee(online_play_artist_label,online_play_artist,212);snprintf(online_drawn_artist,320,"%s",online_play_artist);}
 online_time_refresh();if(online_play_format){char format[80];if(online_rate)snprintf(format,sizeof(format),"%s %u.%u kHz",online_format,online_rate/1000,online_rate%1000/100);else snprintf(format,sizeof(format),"%s · 音质",online_format);lv_text(child(online_play_format,0),format);}
 if(online_toggle_image){int playing=!strcmp(online_play_state,"PLAYING");if(playing!=online_drawn_toggle){online_drawn_toggle=playing;FN(0x52cb44,void (*)(LvObj *,const void *))(online_toggle_image,playing?&online_icon_pause:&online_icon_play);}}
 if(online_cover){if(online_cover_revision==online_play_revision&&online_play_revision)lv_clear_flag(online_cover,1);else lv_add_flag(online_cover,1);}
}
static void online_mini_deleted(void *e){(void)e;online_mini=online_mini_title=online_mini_artist=online_mini_parent=0;online_drawn_mini_title[0]=online_drawn_mini_artist[0]=0;}
static void online_mini_click(void *e){if(!stationary_clicked(e))return;online_resume_pending=1;if(online_mini_parent&&lv_valid(online_mini_parent))online_shade_animate(online_mini_parent,-360);}
static void online_lyrics_load(void){
 if(online_view!=12)return;
 if(online_lyrics_revision!=online_play_revision){char b[65536];if(online_read("/tmp/disc-online-music/lyrics",b,sizeof(b))){char *p=b,*end;unsigned rev=(unsigned)strtoull(p,&end,10);if(rev==online_play_revision&&*end=='\n'){online_lyrics_count=0;p=end+1;while(*p&&online_lyrics_count<600){char *line=p;while(*p&&*p!='\n')p++;if(*p)*p++=0;char *tab=line;while(*tab&&*tab!='\t')tab++;if(!*tab)continue;*tab++=0;online_lyrics_times[online_lyrics_count]=(int)strtol(line,0,10);snprintf(online_lyrics_text[online_lyrics_count++],320,"%s",tab);}online_lyrics_revision=rev;online_drawn_lyric=-999;}}}
}
/* No IPC, provider work or repeated text allocation in the display callback. */
static void online_lyrics_refresh(void){
 if(online_view!=12||!online_lyric_labels[0])return;
 unsigned pos=online_clock.valid?online_clock_position(&online_clock,milliseconds(),online_duration):online_position*1000u;
 unsigned lo=0,hi=online_lyrics_revision==online_play_revision?online_lyrics_count:0;
 while(lo<hi){unsigned mid=lo+(hi-lo)/2;if(online_lyrics_times[mid]<=(int)pos)lo=mid+1;else hi=mid;}
 int current=(int)lo-1;
 if(current==online_drawn_lyric&&online_drawn_lyric_revision==online_play_revision)return;
 online_drawn_lyric=current;online_drawn_lyric_revision=online_play_revision;
 for(unsigned j=0;j<5;j++){int k=current+(int)j-2;lv_text(online_lyric_labels[j],online_lyrics_revision==online_play_revision&&k>=0&&(unsigned)k<online_lyrics_count?online_lyrics_text[k]:j==2&&(online_lyrics_revision!=online_play_revision||!online_lyrics_count)?"正在加载歌词…":"");}
}
static void online_frame_tick(void){
 if(!online_page||(online_view!=8&&online_view!=12)||*(unsigned char *)0x8e1729!=12)return;
 unsigned now=milliseconds();if(now-online_frame_at<40)return;online_frame_at=now;
 online_time_refresh();online_lyrics_refresh();
}
static void online_thumb_refresh(void){
 if(online_view!=2&&online_view!=4&&online_view!=5&&online_view!=13)return;
 for(unsigned i=0;i<online_count;i++){if(!online_thumbs[i]||(online_thumb_loaded&(1u<<i)))continue;int *box=(int *)((char *)online_actions[i]+20);if(box[3]<0||box[1]>=360)continue;
  char path[180];snprintf(path,sizeof(path),"/tmp/disc-online-music/thumb-%s-%u-%u.bgra",online_view==13?"queue":online_view==4?"lists":online_view==5?"tracks":"results",online_view==13?online_queue_revision:(online_view==4||online_view==5)?online_row_generation[i]:online_generation,(online_view==4||online_view==5)?online_row_index[i]:i);int fd=open(path,0);if(fd<0)continue;unsigned n=0;while(n<sizeof(online_thumb_pixels[i])){int k=read(fd,online_thumb_pixels[i]+n,sizeof(online_thumb_pixels[i])-n);if(k<=0)break;n+=(unsigned)k;}close(fd);if(n==sizeof(online_thumb_pixels[i])){online_thumb_loaded|=1u<<i;FN(0x52cb44,void (*)(LvObj *,const void *))(online_thumbs[i],&online_thumb_desc[i]);lv_clear_flag(online_thumbs[i],1);break;}
 }
}
static void online_miniplayer(void){
 unsigned at=milliseconds();if(at-online_play_at<200)return;online_play_at=at;online_thumb_refresh();
 if((online_view==10||online_view==17)&&online_status){char download[320];if(online_read("/tmp/disc-online-music/download-status",download,sizeof(download))&&strcmp(online_drawn_download,download)){lv_text(online_status,download);snprintf(online_drawn_download,320,"%s",download);}}
 if(online_view==0&&online_action_count>=6){LvObj *title=child(online_actions[5],0);if(title&&online_play_name[0])lv_text(title,online_play_name);if(!access("/tmp/disc-online-music/active",0))lv_clear_flag(online_actions[5],1);else lv_add_flag(online_actions[5],1);}
 if(*(unsigned char *)0x8e1729!=12){if(online_mini)lv_add_flag(online_mini,1);return;}
 char b[2400];if(online_read("/tmp/disc-online-music/nowplaying",b,sizeof(b))){
  unsigned rev=0,pos=0,duration=0,rate=0,bits=0,pos_ms=0;char state[32],mode[16],quality[16],format[16];
  char *header_end=b;while(*header_end&&*header_end!='\n')header_end++;char terminator=*header_end;*header_end=0;
  int fields=sscanf(b,"%u\t%31s\t%15s\t%u\t%u\t%15s\t%15s\t%u\t%u\t%u",&rev,state,mode,&pos,&duration,quality,format,&rate,&bits,&pos_ms);*header_end=terminator;
  if(fields>=9){
   if(fields<10)pos_ms=pos*1000u;online_clock_accept(&online_clock,rev,pos_ms,!strcmp(state,"PLAYING"),at);
   online_play_revision=rev;online_position=pos;online_duration=duration;online_rate=rate;online_bits=bits;
   snprintf(online_play_state,sizeof(online_play_state),"%s",state);snprintf(online_play_mode,sizeof(online_play_mode),"%s",mode);snprintf(online_quality,sizeof(online_quality),"%s",quality);snprintf(online_format,sizeof(online_format),"%s",format);
   char *c=b;while(*c&&*c!='\n')c++;if(*c)c++;
   char *dest[5]={online_play_name,online_play_artist,online_play_album,0,online_download_message};for(unsigned i=0;i<5;i++){char *v=c;while(*c&&*c!='\n')c++;if(*c)*c++=0;if(dest[i])snprintf(dest[i],320,"%s",v);}
   char revision[32];if(online_read("/tmp/disc-online-music/cover.revision",revision,sizeof(revision))&&(unsigned)strtoull(revision,0,10)==rev&&online_cover_revision!=rev){int fd=open("/tmp/disc-online-music/cover.bgra",0);if(fd>=0){unsigned n=0;while(n<sizeof(online_cover_pixels)){int k=read(fd,online_cover_pixels+n,sizeof(online_cover_pixels)-n);if(k<=0)break;n+=(unsigned)k;}close(fd);if(n==sizeof(online_cover_pixels)){online_cover_revision=rev;if(online_cover)FN(0x52cb44,void (*)(LvObj *,const void *))(online_cover,&online_cover_desc);}}}
   online_lyrics_load();online_player_refresh();online_lyrics_refresh();
   online_settings_refresh();
  }
 }
 LvObj *shade=LV_GLOBAL(0x8deee4);if(!shade||!lv_valid(shade)||access("/tmp/disc-online-music/active",0)||!online_play_revision){if(online_mini)lv_add_flag(online_mini,1);return;}
 if(!online_mini){online_mini=lv_btn(shade);online_mini_parent=shade;lv_add_flag(online_mini,0x60000);lv_size(online_mini,244,64);lv_align(online_mini,1,58,254);lv_bg(online_mini,0xff202a38,0);lv_border(online_mini,0,0);lv_radius(online_mini,24,0);pad_top(online_mini,0,0);pad_bottom(online_mini,0,0);pad_left(online_mini,0,0);pad_right(online_mini,0,0);lv_clear_flag(online_mini,16|0x4000);protect_tap(online_mini);lv_add_cb(online_mini,online_mini_click,7,0);lv_add_cb(online_mini,online_mini_deleted,33,0);
  online_mini_title=lv_label(online_mini);lv_add_style(online_mini_title,(void *)0x83ac5c,0);online_font(online_mini_title,18);lv_size(online_mini_title,208,26);lv_align(online_mini_title,1,18,6);FN(0x52f4f0,void (*)(LvObj *,int))(online_mini_title,4);lv_color(online_mini_title,0xffffffff,0);lv_clear_flag(online_mini_title,2|16);
  online_mini_artist=lv_label(online_mini);lv_add_style(online_mini_artist,(void *)0x83ac5c,0);online_font(online_mini_artist,16);lv_size(online_mini_artist,208,22);lv_align(online_mini_artist,1,18,35);FN(0x52f4f0,void (*)(LvObj *,int))(online_mini_artist,4);lv_color(online_mini_artist,0xffb2bac8,0);lv_clear_flag(online_mini_artist,2|16);
 }
 // Expanded DSP grid uses the lower area for brightness; show the card when folded.
 LvObj *bright=LV_GLOBAL(0x8deeb4);int expanded=bright&&lv_valid(bright)&&*(int *)((char *)bright+24)-*(int *)((char *)shade+24)>250;
 if(expanded)lv_add_flag(online_mini,1);else lv_clear_flag(online_mini,1);
 if(strcmp(online_drawn_mini_title,online_play_name)){lv_text(online_mini_title,online_play_name);online_marquee(online_mini_title,online_play_name,208);snprintf(online_drawn_mini_title,320,"%s",online_play_name);}char mini[360];snprintf(mini,sizeof(mini),"%s · %s",!strcmp(online_play_state,"PLAYING")?"播放中":"已暂停",online_play_artist);if(strcmp(online_drawn_mini_artist,mini)){lv_text(online_mini_artist,mini);online_marquee(online_mini_artist,mini,208);snprintf(online_drawn_mini_artist,360,"%s",mini);}
}
static void online_poll(void){
 if(online_transport_pending&&(online_play_revision!=online_transport_revision||milliseconds()-online_transport_at>30000))online_transport_pending=0;online_virtual_refresh(0);online_miniplayer();online_list_continue();
 if(online_download_pending){online_download_pending=0;unlink("/tmp/disc-online-music/download-status");if(online_download_current)online_command("download-current","","");else online_command("download-items",online_download_items,"");online_make(17);}
 if(online_swipe_page){int to=online_swipe_page;online_swipe_page=0;online_make(to);}
 if(online_exit_pending){online_exit_pending=0;online_exit_waiting=1;if(!online_command("exit","","")){online_exit_waiting=0;online_make(0);lv_clear_flag(online_status,1);lv_text(online_status,"退出失败，请重试");}}

 if(online_shade_pending){online_shade_pending=0;LvObj *shade=LV_GLOBAL(0x8deee4);if(shade&&lv_valid(shade)){lv_clear_flag(shade,1);online_shade_animate(shade,0);}}
 if(online_resume_pending){online_resume_pending=0;online_save_browser();if(!online_origin){online_origin=lv_active_screen(lv_disp_default());if(online_origin)lv_add_cb(online_origin,online_deleted,33,0);}online_make(8);}
 LvObj *media=LV_GLOBAL(0x8de1b0);if(media&&lv_valid(media))for(unsigned i=0;i<3;i++)if(!online_rows[i]){unsigned n=child_count(media);original_row(media,online_names[i],i,online_enter,0);if(child_count(media)>n){online_rows[i]=child(media,-1);protect_tap(online_rows[i]);lv_add_cb(online_rows[i],online_deleted,33,0);}}
 if(online_open_pending){int f=open("/tmp/disc-online-music/session",1|0x100|0x200,0600);if(f>=0){write(f,"1",1);close(f);}online_exit_waiting=0;online_open_pending=0;online_origin=media;if(media)lv_add_cb(media,online_deleted,33,0);online_make(0);online_send("account",0,0);}
 if(!online_page)return;
 if(online_back_pending&&!online_exit_waiting){online_back_pending=0;char b[64];int n=snprintf(b,sizeof(b),"%u\tcancel\n",milliseconds());int f=open("/tmp/disc-online-music/request.next",1|0x100|0x200,0600);if(f>=0){int ok=write(f,b,n)==n;close(f);if(ok)rename("/tmp/disc-online-music/request.next","/tmp/disc-online-music/request");}online_seq=0;online_loading_more=0;if(online_view==8||online_view==12){online_return_browser();if(online_view==13){online_queue_restore_scroll=online_browser.scroll;online_command("queue","","");}}else if(online_view==9){online_make(8);}else if(online_view==11||online_view==13){if(online_view==13&&online_queue_parent.valid)online_browser=online_queue_parent;online_make(8);}else if(online_view==10||online_view==14){online_return_browser();}else if(online_view==17){online_download_return();}else if(online_view==5){OnlineCatalog *cat=&online_catalogs[1];cat->scroll=online_scroll_position();if(cat->favorite){online_make(0);}else{online_restore_catalog(4);}}else if(online_view==4){online_catalogs[0].scroll=online_scroll_position();online_make(0);}else if(online_view==3){online_make(6);online_send("account",0,0);}else if(online_view)online_make(0);else{online_request_exit();}return;}
 if(online_pending_action>=0){int i=online_pending_action,continue_keyboard=1;online_pending_action=-1;if(i>=200){online_save_browser();online_song_more=i-200;online_more_generation=online_row_generation[online_song_more];online_more_index=online_row_index[online_song_more];if(online_view!=5){online_more_generation=online_generation;online_more_index=(unsigned)online_song_more;}snprintf(online_more_name,804,"%s",online_results[online_song_more]);online_make(14);return;}
  if(online_view==0){if(i==0){online_make(1);}else if(i==1||i==2){online_library_recommend=0;online_list_favorite=i==1;online_list_page=1;online_loading_more=0;online_library_send(i==1?"favorites":"lists",1);online_make(15);lv_text(online_status,i==1?"正在加载收藏歌曲…":"正在加载歌单…");}else if(i==3){online_library_recommend=1;online_send("recommend",0,0);online_make(15);lv_text(online_status,"正在加载推荐…");}else if(i==4){if(online_account_saved){online_make(6);online_send("account",0,0);}else if(online_send("login",0,0)){online_make(15);lv_text(online_status,"正在生成二维码…");}}else if(i==6){online_request_exit();}else if(online_play_revision){online_save_browser();online_make(8);}else lv_text(online_status,"先选择一首歌曲播放");}
  else if(online_view==8){if(i==0)online_make(9);else if(i==1){online_queue_parent=online_browser;online_queue_restore_scroll=-1;online_command("queue","","");}else if(i>=2&&i<=4){if(!online_transport_pending&&online_command("control",i==2?"previous":i==3?"toggle":"next","")){online_transport_pending=1;online_transport_at=milliseconds();online_transport_revision=online_play_revision;if(online_play_time)lv_text(online_play_time,i==3?"正在更新…":"正在切换…");}}else if(i==5)online_make(11);}
  else if(online_view==13){if((unsigned)i<online_count){online_save_browser();char rev[24],index[24];snprintf(rev,24,"%u",online_queue_revision);snprintf(index,24,"%d",i);online_command("queue-play",rev,index);}}
  else if(online_view==14){if(i==0){online_return_browser();i=online_song_more;online_song_more=-1;if(i>=0){if(*(unsigned char *)0x8e1729!=12)dlna_enter_native();screen_load(online_page);if(online_send("play",online_more_generation,online_more_index)){online_make(15);lv_text(online_status,"正在连接歌曲…");}}}else if(i==1){snprintf(online_download_items,720,"%u:%u",online_more_generation,online_more_index);online_request_download(0);}else{online_return_browser();online_open_batch();}}

  else if(online_view==9){if(i==0){const char *m=!strcmp(online_play_mode,"order")?"shuffle":!strcmp(online_play_mode,"shuffle")?"repeat":"order";online_command("control","mode",m);}else if(i==1)online_make(11);else if(i==2)online_request_download(1);}
  else if(online_view==17){online_download_return();}
  else if(online_view==11){const char *q[5]={"auto","standard","exhigh","lossless","hires"};if(i<5)online_command("control","quality",q[i]);}
  else if(online_view==10){if((unsigned)i<online_count){online_selected_mask^=1u<<i;online_make(10);}else if((unsigned)i==online_count){online_selected_mask=online_selected_mask?0:(1u<<online_count)-1;online_make(10);}else{unsigned n=0;online_download_items[0]=0;for(unsigned j=0;j<online_count;j++)if(online_selected_mask&(1u<<j))n+=snprintf(online_download_items+n,720-n,"%s%u:%u",n?",":"",online_batch_generation[j],online_batch_index[j]);if(n)online_request_download(0);else lv_text(online_status,"请先选择歌曲");}}
  else if(online_view==6){if(i==0){online_login_poll=0;if(online_send("login",0,0))lv_text(online_status,"正在生成二维码…");}else online_make(7);}
  else if(online_view==7){if(i==0){online_make(6);online_send("account",0,0);}else if(online_send("logout",0,0))lv_text(online_status,"正在退出登录…");}
  else if(online_view==1){const char *keys=online_input_mode==2?"1234567890-/'&.,!?()@#+=:_":"qwertyuiopasdfghjklzxcvbnm";
   if(i<26){char t[2]={keys[i],0};if(online_input_mode==0){unsigned n=online_length(online_compose);if(n<64){online_compose[n]=t[0];online_compose[n+1]=0;online_candidate_page=0;}}else online_append(t);}
   else if(i==26){if(online_compose[0]){online_append(online_compose);online_compose[0]=0;if(online_ime_ready)online_ime_reset();}online_input_mode=(online_input_mode+1)%3;}
   else if(i==27)online_erase();else if(i==28){if(online_compose[0])online_ime_pick(0);else online_append(" ");}
   else if(i==29){if(online_compose[0])online_ime_pick(0);if(!online_compose[0]){online_search();continue_keyboard=0;}}
   else if(i==30){if(online_candidate_page)online_candidate_page--;}else if(i>=31&&i<=33)online_ime_pick((unsigned)i-31);else if(i==34){if((online_candidate_page+1)*3<online_candidate_count)online_candidate_page++;}
   if(continue_keyboard)online_make(1);
  }
  else if(online_view==3){online_login_poll=0;if(online_send("login",0,0))lv_text(online_status,"正在生成二维码…");}
  else if(online_view==4){OnlineCatalog *cat=&online_catalogs[0];if(i>=30){online_load_failed=0;online_list_continue();}else if(i>=0&&cat->first+(unsigned)i<cat->count){unsigned index=cat->first+(unsigned)i;cat->scroll=online_scroll_position();online_selected_list=cat->items[index].index;online_list_generation=cat->items[index].generation;online_track_page=1;online_loading_more=0;online_library_send("tracks",1);online_make(15);lv_text(online_status,"正在加载歌曲…");}}
  else if((online_view==2&&(unsigned)i==online_count)||(online_view==5&&i==31)){online_open_batch();}
  else if(online_view==5&&i==30){online_load_failed=0;online_list_continue();}
  else if((online_view==2||online_view==5)&&(i==100||(unsigned)i<online_count)){if(i==100)i=0;if(!online_count)return;online_save_browser();unsigned gen=online_view==5?online_row_generation[i]:online_generation,index=online_view==5?online_row_index[i]:(unsigned)i;if(*(unsigned char *)0x8e1729!=12)dlna_enter_native();screen_load(online_page);if(!online_send("play",gen,index))lv_text(online_status,"后台不可用");else{online_make(15);lv_text(online_status,"正在连接歌曲…");}}

 }

 if((online_view==4||online_view==5)&&!online_seq&&!online_loading_more&&online_count){OnlineCatalog *cat=online_catalog();int offset=(online_scroll_position()-(online_view==5?154:108))/72;if(offset<0)offset=0;unsigned index=(unsigned)offset;if(index>=cat->count)index=cat->count?cat->count-1:0;unsigned last=index+5;if(last>=cat->count)last=cat->count?cat->count-1:0;unsigned gen1=cat->count?cat->items[index].generation:0,gen2=cat->count?cat->items[last].generation:0;if(gen1&&(gen1!=online_art_generation||gen2!=online_art_generation_next)){online_art_generation=gen1;online_art_generation_next=gen2;char gen[48];snprintf(gen,48,"%u,%u",gen1,gen2);online_command("art-pages",online_view==4?"lists":"tracks",gen);}}
 unsigned at=milliseconds();if(at-online_response_at>=150){online_response_at=at;if(online_seq)online_response();}
 if(online_page&&online_view==3&&online_login_poll&&!online_seq&&at-online_login_at>=2000){online_login_at=at;online_send("login-check",0,0);}
}
