/* V2.57 only. Called exclusively from the stable original UI extension.
 * No constructor, thread, code patch, signal, audio handle, or player write.
 * USB routing uses native commands and verified USB role controls on demand. */
typedef unsigned size_t;
extern int open(const char *,int,...),close(int),read(int,void *,size_t),write(int,const void *,size_t),rename(const char *,const char *),access(const char *,int);
extern int pread(int,void *,size_t,long),clock_gettime(int,void *),snprintf(char *,size_t,const char *,...),strcmp(const char *,const char *),memcmp(const void *,const void *,size_t);
extern unsigned long strtoul(const char *,char **,int);
extern int sscanf(const char *,const char *,...);
extern void *fopen(const char *,const char *);
extern char *fgets(char *,int,void *);
extern int fclose(void *);
extern long readlink(const char *,char *,size_t);
extern int getpid(void);
#include "../settings-debug/ui_api.h"
#include "assets.h"
#include "control_assets.h"
#include "visual_toolbar_geometry.h"
#include "abi_guard.h"
#include "xy_binding.h"
#define FN(a,t) LV_PTR(a,t)
#define img FN(0x52cb10,LvObj *(*)(LvObj *))
#define src FN(0x52cb44,void (*)(LvObj *,const void *))
#define parent FN(0x4f8838,LvObj *(*)(LvObj *))
#define opa FN(0x4f6b60,void (*)(LvObj *,unsigned,unsigned))
#define padtop FN(0x4f6af0,void (*)(LvObj *,int,unsigned))
#define padbottom FN(0x4f6b00,void (*)(LvObj *,int,unsigned))
#define padleft FN(0x4f6b10,void (*)(LvObj *,int,unsigned))
#define padright FN(0x4f6b20,void (*)(LvObj *,int,unsigned))
#define fontstyle FN(0x48831c,void **(*)(unsigned))
#define textalign FN(0x4f6e50,void (*)(LvObj *,unsigned,unsigned))
#include "../audio-visuals/usbdac_format.h"
#include "../dlna/audio_format.h"
#include "../audio-visuals/visual_normalize.h"
#include "../audio-visuals/visual_spectrum_input.h"
#include "../audio-visuals/visual_reconstruct.h"
#include "../visual-signal/ring.h"
static int visual_post,visual_source_error;
static void visual_source_toggle(void);
static const char *visual_source_caption(void){return visual_source_error?"保存失败":visual_post?"信号 音效后":"信号 原始";}
static int dac_info_enabled=1,dac_normalize_enabled=1,net_info_enabled=1,net_normalize_enabled=1;
static int online_music_session(void){return *(unsigned char *)0x8e1729==12&&!access("/tmp/disc-online-music/session",0);}
static int dlna_visual(void){return *(unsigned *)0x8e16d4==5&&*(unsigned char *)0x8e1729==12;}
static unsigned visual_pref_index(void){return dlna_visual()?2:*(unsigned *)0x8e16d4==1?1:0;}
static int visual_normalized(void){return dlna_visual()?net_normalize_enabled:*(unsigned *)0x8e16d4==1&&dac_normalize_enabled;}
static unsigned visual_source_data[DISC_SPECTRUM_SAMPLES*2*16];
static int usbdac_enabled;static unsigned usbdac_checked,last_visual_mode=99;
static int visual_mode(void){return dlna_visual()||disc_visual_mode(*(unsigned *)0x8e16d4,usbdac_enabled);}
static unsigned visual_native_callback(void){return dlna_visual()?0x4763e0:*(unsigned *)0x8e16d4==1?0x475840:0x477d50;}
static LvObj *layer,*led[128],*button,*caption,*level_glyph;
static LvObj *rate_button,*rate_caption,*rate_glyph;static int rate_enabled=1;
static LvObj *eq_button,*eq_caption,*eq_glyph;static int eq_seen=-1;
static LvObj *fold_button,*fold_glyph,*bright_holder,*gesture_shade;static int folded=1,holder_height;
static unsigned original_holder_flags,original_grid_bubble;static LvObj *gesture_grid;
static LvObj *menu_clip;static int menu_fold_state=-1,bright_fold_state=-1,bright_current_y;
static int menu_open,meter_visible,native_overlay_open;
static int pure_editing,vt_editing;
static LvObj *rotate_button,*rotate_caption,*rotate_glyph;static int rotate_seen=-1;
static unsigned lastfold;static void fold_change(void);
static int active,enabled=1,configured,shifted,drawn[2]={-1,-1},value[2];
static int smooth[2];static unsigned lastanimation;
static unsigned original_bright_flags;
static int badge_shifted;
static void *fast_timer;static unsigned normal_period;static int fast_ui;static unsigned refreshes;
static int memfd=-1,playerpid,lastbits,guarded;
static unsigned lasttotal,lasttotalhi,lastadvance,lastsample,lastdiscovery,laststate;
static unsigned attempts,valid_samples,last_interval,min_interval=0xffffffffu,max_interval;
static unsigned maps_lo[96],maps_hi[96],mapcount;
static unsigned metarate,metabits,metabitrate,metaat;
static unsigned flowtotal,flowhi;static int stream_flow;
static char metacodec[40]="Bluetooth";
static LvObj *rate_label,*format_label,*realtime_label,*layout_panel;
static void *font16,*font18,*font22;
static LvObj *saved_song,*saved_artist;static int song_height,artist_height;
static int longname,layout_long=-1;static char rate_text[96],format_text[64],codec_text[40],realtime_text[64];
static unsigned realtime_rendered;
/* Owned objects have delete callbacks that clear their pointers on this UI
 * thread. Native/global objects still use the stock tree validation, cached
 * only within one prepare call; never carry validity across UI frames. */
static int validation_frame;static unsigned validation_count;
static struct {const LvObj *object;int valid;} validation[48];
static int object_valid(const LvObj *o){
 if(!o)return 0;
 if(o==layer||o==button||o==caption||o==rate_button||o==rate_caption||o==eq_button||o==eq_caption||o==menu_clip||o==rate_label||o==format_label||o==realtime_label)return 1;
 if(validation_frame)for(unsigned i=0;i<validation_count;i++)if(validation[i].object==o)return validation[i].valid;
 int valid=FN(0x4e9a70,int (*)(const LvObj *))(o);
 if(validation_frame&&validation_count<48){validation[validation_count].object=o;validation[validation_count++].valid=valid;}
 return valid;
}
#undef lv_valid
#define lv_valid object_valid
static unsigned now(void){long t[2];if(clock_gettime(1,t))return 0;return t[0]*1000u+t[1]/1000000u;}
static void show(LvObj *o,int yes){if(o&&lv_valid(o)){unsigned hidden=(*(unsigned *)((char *)o+0x24)&1)!=0;if(hidden==!yes)return;if(yes)lv_clear_flag(o,1);else lv_add_flag(o,1);}}
static int textread(const char *p,char *b,unsigned z){int f=open(p,0);if(f<0)return 0;int n=read(f,b,z-1);close(f);if(n<=0)return 0;b[n]=0;return n;}
static int mapped(unsigned p,unsigned z){if(p>0x7fffffffu-z)return 0;for(unsigned i=0;i<mapcount;i++)if(p>=maps_lo[i]&&p+z<=maps_hi[i])return 1;return 0;}
static int copy(unsigned p,void *b,unsigned z){return memfd>=0&&mapped(p,z)&&pread(memfd,b,z,p)==(int)z;}
#include "../visual-signal/ui.h"
#include "realtime.h"
static void forget(void){vs_ui_release();if(memfd>=0)close(memfd);memfd=-1;playerpid=0;mapcount=0;lasttotal=lasttotalhi=lastadvance=lastbits=0;value[0]=value[1]=smooth[0]=smooth[1]=0;lastanimation=0;}
static void discover(unsigned t){
 if(t-lastdiscovery<1000)return;lastdiscovery=t;char b[96],path[64],exe[64];
 int p=0;
 if(textread("/tmp/disc-runtime/bt-player",b,sizeof(b)))p=(int)strtoul(b,0,10);
 if(p>0){snprintf(path,sizeof(path),"/proc/%d/exe",p);long len=readlink(path,exe,63);if(len<0||len>62)p=0;else{exe[len]=0;if(strcmp(exe,"/usr/bin/mq_player"))p=0;}}
 if(!p&&memfd>=0)p=playerpid;
 if(!p){int self=getpid();for(int candidate=self+1;candidate<self+65;candidate++){snprintf(path,sizeof(path),"/proc/%d/exe",candidate);long len=readlink(path,exe,63);if(len<0||len>62)continue;exe[len]=0;if(!strcmp(exe,"/usr/bin/mq_player")){p=candidate;break;}}}
 if(p<1){forget();return;}
 int reuse=p==playerpid&&memfd>=0;if(!reuse)forget();
 snprintf(path,sizeof(path),"/proc/%d/exe",p);long n=readlink(path,exe,63);if(n<0||n>62)return;exe[n]=0;if(strcmp(exe,"/usr/bin/mq_player"))return;
 snprintf(path,sizeof(path),"/proc/%d/maps",p);void *f=fopen(path,"r");if(!f)return;
 mapcount=0;char line[256],permission[8];unsigned low,high,linecount=0;while(fgets(line,sizeof(line),f)&&mapcount<96&&linecount++<512){
  if(sscanf(line,"%x-%x %4s",&low,&high,permission)==3&&permission[0]=='r'){maps_lo[mapcount]=low;maps_hi[mapcount++]=high;}
 }fclose(f);if(reuse)return;snprintf(path,sizeof(path),"/proc/%d/mem",p);memfd=open(path,0);if(memfd>=0){unsigned char signature[16];for(unsigned i=0;i<2;i++)if(!copy(player_guards[i].address,signature,16)||memcmp(signature,player_guards[i].bytes,16)){forget();return;}playerpid=p;}
}
static int level(unsigned long long energy){int lo=0,hi=100;while(lo<hi){int mid=(lo+hi+1)/2;if(energy>=energy_threshold[mid])lo=mid;else hi=mid-1;}if(lo==100)return 25600;
 unsigned long long base=energy_threshold[lo],range=energy_threshold[lo+1]-base;int a=0,b=255;
 while(a<b){int mid=(a+b+1)/2;if(energy>=base+((range*mid)>>8))a=mid;else b=mid-1;}return lo*256+a;}
static unsigned countermod(unsigned lo,unsigned hi,unsigned divisor){unsigned remainder=0;for(int i=63;i>=0;i--){unsigned bit=i>=32?(hi>>(i-32))&1u:(lo>>i)&1u;remainder=(remainder<<1)|bit;if(remainder>=divisor)remainder-=divisor;}return remainder;}
static int dac_home_sampling(void);
static void dac_home_feed(const unsigned char *,unsigned,unsigned,unsigned,unsigned,unsigned,unsigned);
static int usbdac_pcm(unsigned t){
 unsigned player,header[40],state,mode_pointer,workmode,rate,bits,channels,ring[10],after[10];
 if(!copy(0x83dfc4,&player,4)||!copy(player,header,sizeof(header))||!copy(0x839a74,&state,4)||!copy(0x83dfc0,&mode_pointer,4)||!copy(mode_pointer,&workmode,4)||!(dlna_visual()?disc_dlna_format(header,workmode,&rate,&bits,&channels):disc_uac_format(header,workmode,state,&rate,&bits,&channels)))return 0;
 if(!copy(player+0x130,ring,sizeof(ring)))return 0;
 int home_fft=page_kind==DISC_PAGE_HOME&&dac_home_sampling();
 unsigned step=page_kind==DISC_PAGE_HOME&&!home_fft?1:page_kind==DISC_PAGE_SPECTRUM||home_fft?disc_spectrum_step(rate):disc_scope_step(rate),samples=page_kind==DISC_PAGE_SPECTRUM||home_fft?DISC_SPECTRUM_SAMPLES:page_kind==DISC_PAGE_HOME?960u:disc_scope_samples(rate),frame=(bits/8)*channels,padding=page_kind!=DISC_PAGE_HOME&&page_kind!=DISC_PAGE_SPECTRUM?disc_scope_padding(rate):0,bytes=(samples*step+padding*2)*frame,capacity=ring[9],pos=ring[4];
 if(capacity<bytes||capacity>8*1024*1024||pos>=capacity||pos%frame||countermod(ring[2],ring[3],capacity)!=pos||!mapped(ring[0],capacity))return 0;
 if(ring[2]==lasttotal&&ring[3]==lasttotalhi)return lastadvance&&t-lastadvance<=300;
 if(ring[3]==0&&ring[2]<bytes)return 0;
 unsigned char *data=(void *)visual_source_data;
 unsigned offset=(pos+capacity-bytes)%capacity,head=bytes<capacity-offset?bytes:capacity-offset;
 if(!copy(ring[0]+offset,data,head)||(head<bytes&&!copy(ring[0],data+head,bytes-head)))return 0;
 if(!copy(player+0x130,after,sizeof(after))||after[0]!=ring[0]||after[9]!=capacity||after[3]!=ring[3]||after[2]<ring[2]||after[2]-ring[2]>=capacity-bytes)return 0;
 unsigned current[40];if(!copy(player,current,sizeof(current))||memcmp(header+3,current+3,4*4)||current[11]!=header[11]||current[38]!=header[38])return 0;
 if(disc_uac_is_dop(data,bits,channels,samples))return 0;
 if(home_fft)dac_home_feed(data,bits,channels,rate,t,samples,step);else xy_feed_pcm(data,bits,channels,rate,t,samples);lasttotal=ring[2];lasttotalhi=ring[3];lastadvance=t;lastbits=bits;metarate=rate;metabits=bits;return 1;
}
static int visual_post_pcm(unsigned t){
 unsigned mode=*(unsigned *)0x8e16d4==1?4:2;
 if(!vs_ui||!vs_ui_wanted)return 0;
 unsigned rate=vs_ui->rate;if(rate<8000||rate>384000||(mode==2&&rate>192000))return 0;
 int home_fft=mode==4&&page_kind==DISC_PAGE_HOME&&dac_home_sampling();
 unsigned samples=page_kind==DISC_PAGE_SPECTRUM||home_fft?DISC_SPECTRUM_SAMPLES:page_kind==DISC_PAGE_HOME?960u:disc_scope_samples(rate);
 unsigned step=page_kind==DISC_PAGE_SPECTRUM||home_fft?disc_spectrum_step(rate):page_kind==DISC_PAGE_HOME?1:disc_scope_step(rate);
 DiscVisualWindow window;int *data=(int *)visual_source_data;
 unsigned padding=page_kind!=DISC_PAGE_HOME&&page_kind!=DISC_PAGE_SPECTRUM?disc_scope_padding(rate):0,frames=samples*step+padding*2,end;
 if(!disc_vs_meta(vs_ui,rate,mode,(unsigned)playerpid,t,&window)||!disc_vs_clock(&vs_ui_clock,&window,frames,t,&end))return 0;
 if(!disc_vs_read_window(vs_ui,data,frames,rate,mode,(unsigned)playerpid,t,end,0,&window))return 0;
 if(home_fft)dac_home_feed((const unsigned char *)data,32,2,rate,t,samples,step);
 else xy_feed_pcm((const unsigned char *)data,32,2,rate,t,samples);
 if(mode==2&&page_kind==DISC_PAGE_HOME&&!xy_progress){unsigned long long energy[2]={0,0};for(unsigned i=0;i<960;i++)for(unsigned c=0;c<2;c++){int sample=data[i*2+c]>>16;energy[c]+=(long long)sample*sample;}for(unsigned c=0;c<2;c++)value[c]=level(energy[c]);}
 vs_ui_last_sequence=window.sequence;lastadvance=t;lastbits=32;return 1;
}
static int pcm_levels(unsigned t){
 if(visual_post&&!dlna_visual())return visual_post_pcm(t);
 unsigned ctx,a[8],player,ring[10],after[10];
 if(*(unsigned *)0x8e16d4==1||dlna_visual())return usbdac_pcm(t);
 if(!copy(0x898d70,&ctx,4)||!copy(ctx,a,32)||a[5]!=1||a[7]!=1||(a[3]!=1&&a[3]!=2)||a[1]<8000||a[1]>192000)return 0;
 /* Verified ALSA format table: 16=S16_LE, 32=S32_LE. Packed 24-bit is deliberately rejected. */
 unsigned bits=a[2],channels=a[3],frame=bits==32?4*channels:bits==16?2*channels:0;if(!frame)return 0;
 if(!copy(0x83dfc4,&player,4)||!copy(player+0x130,ring,40))return 0;
 unsigned samples=page_kind==DISC_PAGE_SPECTRUM?DISC_SPECTRUM_SAMPLES:page_kind==DISC_PAGE_HOME?960u:disc_scope_samples(a[1]);
 unsigned step=page_kind==DISC_PAGE_HOME?1:page_kind==DISC_PAGE_SPECTRUM?disc_spectrum_step(a[1]):disc_scope_step(a[1]);
 unsigned padding=page_kind!=DISC_PAGE_HOME&&page_kind!=DISC_PAGE_SPECTRUM?disc_scope_padding(a[1]):0;
 unsigned capacity=ring[9],pos=ring[4],bytes=(samples*step+padding*2)*frame;
 if(capacity<bytes||capacity>8*1024*1024||pos>=capacity||pos%frame||countermod(ring[2],ring[3],capacity)!=pos||!mapped(ring[0],capacity))return 0;
 if(ring[2]==lasttotal&&ring[3]==lasttotalhi){return lastadvance&&t-lastadvance<=300;}
 if(ring[2]<bytes&&ring[3]==0)return 0;
 unsigned offset=(pos+capacity-bytes)%capacity,head=bytes<capacity-offset?bytes:capacity-offset;
 unsigned char *data=(void *)visual_source_data;if(!copy(ring[0]+offset,data,head)||(head<bytes&&!copy(ring[0],data+head,bytes-head)))return 0;
 if(!copy(player+0x130,after,40)||after[0]!=ring[0]||after[9]!=capacity||after[3]!=ring[3]||after[2]<ring[2]||after[2]-ring[2]>=capacity-bytes)return 0;
 xy_feed_pcm(data,bits,channels,a[1],t,samples);
 unsigned long long energy[2]={0,0};
 if(page_kind==DISC_PAGE_HOME&&!xy_progress)for(unsigned i=0;i<960;i++)for(unsigned c=0;c<2;c++){
  unsigned index=i*channels+(channels==1?0:c);
  int s=bits==32?((int *)(void *)data)[index]>>16:((short *)(void *)data)[index];energy[c]+=(long long)s*s;
 }
 lasttotal=ring[2];lasttotalhi=ring[3];lastadvance=t;lastbits=bits;
 for(unsigned c=0;c<2;c++)value[c]=level(energy[c]);
 return 1;
}
static void deleted(void *e){validation_count=0;LvObj *o=lv_event_target(e);if(o==layer){layer=0;layout_panel=0;for(unsigned i=0;i<128;i++)led[i]=0;drawn[0]=drawn[1]=-1;}if(o==button){button=level_glyph=0;}if(o==caption)caption=0;if(o==rate_label)rate_label=0;if(o==format_label)format_label=0;if(o==realtime_label)realtime_label=0;if(o==rate_button){rate_button=rate_glyph=0;}if(o==rate_caption)rate_caption=0;if(o==eq_button)eq_button=eq_glyph=0;if(o==eq_caption)eq_caption=0;if(o==rotate_button)rotate_button=rotate_glyph=0;if(o==rotate_caption)rotate_caption=0;if(o==fold_button)fold_button=fold_glyph=0;if(o==bright_holder){bright_holder=0;holder_height=0;bright_fold_state=-1;}if(o==menu_clip){menu_clip=0;menu_fold_state=-1;}if(o==gesture_shade)gesture_shade=0;if(o==gesture_grid)gesture_grid=0;}
static void stylezero(LvObj *o){lv_border(o,0,0);padleft(o,0,0);padright(o,0,0);padtop(o,0,0);padbottom(o,0,0);lv_clear_flag(o,2|16);lv_add_flag(o,0x60000);}
#include "ldac_probe.h"
static void metadata(unsigned t){
 if(t-metaat<250)return;metaat=t;metarate=metabits=0;stream_flow=0;
 char previous[40];snprintf(previous,sizeof(previous),"%s",metacodec);
 snprintf(metacodec,sizeof(metacodec),"Bluetooth");
 unsigned ctx,a[8];if(copy(0x898d70,&ctx,4)&&copy(ctx,a,32)&&a[1]>=8000&&a[1]<=192000&&(a[3]==1||a[3]==2)&&(a[2]==16||a[2]==32)){
  /* get_sink_param stores the negotiated decoder ID at context +0x10.
   * /usr/data/bt_codec is a preference/cache and survives codec changes. */
  const char *codec=a[4]==1?"AAC":a[4]==2?"SBC":a[4]==3?"aptX":a[4]==5?"aptX-HD":a[4]==8?"LHDC V5":a[4]==9?"LDAC":"Bluetooth";
  snprintf(metacodec,sizeof(metacodec),"%s",codec);
  metarate=a[1];metabits=a[2];unsigned p,ring[10];if(a[5]==1&&a[7]==1&&copy(0x83dfc4,&p,4)&&copy(p+0x130,ring,40)){
   stream_flow=ring[2]!=flowtotal||ring[3]!=flowhi;flowtotal=ring[2];flowhi=ring[3];
  }else flowtotal=flowhi=0;
 }
 if(strcmp(previous,metacodec)){metabitrate=0;decoderclose();monitor_close();}
 longname=0;while(metacodec[longname])longname++;longname=longname>8;
 if(!strcmp(metacodec,"LDAC")&&metarate){if(stream_flow){unsigned current=decoder_bitrate(metarate);if(current)metabitrate=current;}else decoderclose();}else {metabitrate=0;decoderclose();}
}
static void smallfont(LvObj *o,void *style){if(o&&lv_valid(o)&&style)lv_add_style(o,style,0);}
static void ensurefonts(void){void **f;if(!font16){f=fontstyle(16);if(f)font16=*f;}if(!font18){f=fontstyle(18);if(f)font18=*f;}if(!font22){f=fontstyle(22);if(f)font22=*f;}}
static void place(unsigned address,int w,int h,int align,int x,int y){LvObj *o=LV_GLOBAL(address);if(o&&lv_valid(o)){lv_size(o,w,h);lv_align(o,align,x,y);}}
static void statictext(LvObj *o){if(o&&lv_valid(o))FN(0x52f4f0,void (*)(LvObj *,unsigned))(o,1);}
#include "bt_button_status.h"
static void compact_layout(LvObj *panel){
 bt_button_sync();
 ensurefonts();
 if(layout_panel!=panel){layout_panel=panel;layout_long=-1;codec_text[0]=rate_text[0]=format_text[0]=0;
  LvObj *song=LV_GLOBAL(0x8de3bc),*artist=LV_GLOBAL(0x8de3b8);
  if(song&&lv_valid(song)&&song!=saved_song){int *box=(int *)((char *)song+20);song_height=box[3]-box[1]+1;saved_song=song;}
  if(artist&&lv_valid(artist)&&artist!=saved_artist){int *box=(int *)((char *)artist+20);artist_height=box[3]-box[1]+1;saved_artist=artist;}
  place(0x8de3d4,76,76,2,0,188);place(0x8de3d0,52,52,1,54,204);place(0x8de3cc,52,52,1,254,204);
  const unsigned icons[]={0x8de3c8,0x8de3c0,0x8de3c4};for(unsigned i=0;i<3;i++){LvObj *o=LV_GLOBAL(icons[i]);if(o&&lv_valid(o)){FN(0x52dcac,void (*)(LvObj *,unsigned))(o,i?205:210);lv_align(o,9,0,0);}}
  place(0x8de3bc,244,40,2,0,91);smallfont(LV_GLOBAL(0x8de3bc),font22);statictext(LV_GLOBAL(0x8de3bc));
  place(0x8de3b8,228,30,2,0,131);smallfont(LV_GLOBAL(0x8de3b8),font16);statictext(LV_GLOBAL(0x8de3b8));
  LvObj *connected=LV_GLOBAL(0x8de3d8);if(connected&&lv_valid(connected)&&!rate_label){
   rate_label=lv_label(connected);stylezero(rate_label);smallfont(rate_label,font18);statictext(rate_label);lv_color(rate_label,0xffe6eef0,0);lv_add_cb(rate_label,deleted,33,0);
   format_label=lv_label(connected);stylezero(format_label);smallfont(format_label,font16);statictext(format_label);lv_color(format_label,0xff93adb8,0);lv_add_cb(format_label,deleted,33,0);
   realtime_label=lv_label(connected);stylezero(realtime_label);smallfont(realtime_label,font16);statictext(realtime_label);lv_color(realtime_label,0xff8bdde0,0);lv_size(realtime_label,240,30);lv_align(realtime_label,2,0,158);lv_add_cb(realtime_label,deleted,33,0);realtime_text[0]=0;
  }
 }
 LvObj *badge=LV_GLOBAL(0x8de3b4);
 /* V5's current frame budget is separate from A2DP capability limits and
  * from the wall-clock received bitrate shown above the play button. */
 int is_lhdcv5=!strcmp(metacodec,"LHDC V5");int has_tier=!strcmp(metacodec,"LDAC")||is_lhdcv5;int key=longname|(has_tier<<1);
 if(layout_long!=key&&badge&&lv_valid(badge)){
  smallfont(badge,longname?font16:font18);lv_border(badge,0,0);textalign(badge,2,0);opa(badge,0,0);
  statictext(badge);lv_color(badge,0xff6edfe3,0);lv_size(badge,240,32);lv_align(badge,2,0,251);
  if(rate_label){lv_size(rate_label,260,32);lv_align(rate_label,2,0,longname?279:271);textalign(rate_label,2,0);}
  if(format_label){lv_size(format_label,260,28);lv_align(format_label,2,0,306);}
  if(realtime_label){lv_align(realtime_label,2,0,158);textalign(realtime_label,2,0);}
  layout_long=key;
 }
 if(badge&&strcmp(codec_text,metacodec)){snprintf(codec_text,sizeof(codec_text),"%s",metacodec);lv_text(badge,codec_text);}
 show(badge,longname&&has_tier);
 char r[96],f[64],tier[32];if(metabitrate)snprintf(tier,sizeof(tier),"%u kbps",metabitrate);else snprintf(tier,sizeof(tier),"-- kbps");
 if(is_lhdcv5){unsigned current=v5_bitrate_current(now());if(current)snprintf(tier,sizeof(tier),"%u kbps",current);else snprintf(tier,sizeof(tier),"-- kbps");}
 if(!has_tier)snprintf(r,sizeof(r),"%s",metacodec);else if(longname)snprintf(r,sizeof(r),"%s",tier);else snprintf(r,sizeof(r),"%s   %s",metacodec,tier);
 if(metarate){unsigned tenth=(metarate%1000)/100;if(tenth)snprintf(f,sizeof(f),"%u.%u kHz  /  PCM %ubit",metarate/1000,tenth,metabits);else snprintf(f,sizeof(f),"%u kHz  /  PCM %ubit",metarate/1000,metabits);}else snprintf(f,sizeof(f),"-- kHz  /  PCM --bit");
 if(rate_label&&strcmp(rate_text,r)){snprintf(rate_text,sizeof(rate_text),"%s",r);lv_text(rate_label,r);}
 if(format_label&&strcmp(format_text,f)){snprintf(format_text,sizeof(format_text),"%s",f);lv_text(format_label,f);}
 unsigned t=now();if(t-realtime_rendered>=250||!realtime_text[0]){realtime_rendered=t;char real[64];if(monitorfd>=0&&realtime_supported(metacodec))snprintf(real,sizeof(real),"实时  %u kbps",realtime_kbps);else snprintf(real,sizeof(real),"实时  -- kbps");
  if(realtime_label&&strcmp(realtime_text,real)){snprintf(realtime_text,sizeof(realtime_text),"%s",real);lv_text(realtime_label,real);}}
 show(rate_label,1);show(format_label,metarate!=0);show(realtime_label,rate_enabled);
}
static void restore_layout(void){
 if(!layout_panel)return;layout_panel=0;layout_long=-1;
 show(rate_label,0);show(format_label,0);show(realtime_label,0);
 place(0x8de3d4,93,93,2,0,190);place(0x8de3d0,60,60,1,45,215);place(0x8de3cc,60,60,1,269,215);
 const unsigned icons[]={0x8de3c8,0x8de3c0,0x8de3c4};for(unsigned i=0;i<3;i++){LvObj *o=LV_GLOBAL(icons[i]);if(o&&lv_valid(o)){FN(0x52dcac,void (*)(LvObj *,unsigned))(o,256);lv_align(o,9,0,i?-12:0);}}
 smallfont(LV_GLOBAL(0x8de3bc),(void *)0x83ac3c);
 const unsigned labels[]={0x8de3bc,0x8de3b8,0x8de3b4};for(unsigned i=0;i<3;i++){LvObj *o=LV_GLOBAL(labels[i]);if(o&&lv_valid(o))FN(0x52f4f0,void (*)(LvObj *,unsigned))(o,3);}
 place(0x8de3bc,280,song_height?song_height:40,2,0,90);place(0x8de3b8,280,artist_height?artist_height:32,2,0,137);
 smallfont(LV_GLOBAL(0x8de3b8),(void *)0x83ac5c);
 LvObj *badge=LV_GLOBAL(0x8de3b4);if(badge&&lv_valid(badge)){show(badge,1);smallfont(badge,(void *)0x83ac64);lv_size(badge,100,40);lv_align(badge,2,0,311);opa(badge,127,0);lv_color(badge,0xffffffff,0);textalign(badge,2,0);}
}
static void refreshbutton(void){if(!button||!caption)return;lv_text(caption,enabled?"电平 开":"电平 关");if(level_glyph)src(level_glyph,enabled?&level_icon_1:&level_icon_0);}
static void refreshratebutton(void){if(rate_button&&rate_caption){lv_text(rate_caption,visual_source_caption());if(rate_glyph)src(rate_glyph,visual_post?&rate_icon_1:&rate_icon_0);}}
static int tapx,tapy,moved=1;static unsigned downat;
static LvObj *control_target;static int control_x,control_y,control_moved=1;static unsigned control_at;
static void control_touch(void *e){unsigned code=lv_event_code(e);int pt[2];void *input=FN(0x4e42e8,void *(*)(void))();if(!input)return;FN(0x4e45bc,void (*)(void *,int *))(input,pt);LvObj *target=lv_event_target(e);
 if(code==1){control_target=target;control_x=pt[0];control_y=pt[1];control_moved=0;control_at=now();}
 else if(target==control_target&&(code==2||code==8)){int dx=pt[0]-control_x,dy=pt[1]-control_y;if(dx*dx+dy*dy>196)control_moved=1;}
 else if(code==3&&target==control_target){control_moved=1;control_target=0;}}
static int control_clicked(void *e){if(vt_editing||lv_event_code(e)!=7||lv_event_target(e)!=control_target||control_moved||now()-control_at>2000)return 0;control_target=0;return 1;}
static void touch(void *e){unsigned code=lv_event_code(e);int pt[2];void *indev=FN(0x4e42e8,void *(*)(void))();if(!indev)return;FN(0x4e45bc,void (*)(void *,int *))(indev,pt);
 if(code==1){tapx=pt[0];tapy=pt[1];moved=0;downat=now();}else if(code==2||code==8){int dx=pt[0]-tapx,dy=pt[1]-tapy;if(dx*dx+dy*dy>100)moved=1;}}
static void menu_gesture(void *e,int bottom_exit){
 if(vt_editing||!active||*(unsigned *)0x8e16d4!=3)return;
 touch(e);if(lv_event_code(e)!=8||(bottom_exit&&tapy>=271)||now()-lastfold<=300)return;
 int pt[2];void *indev=FN(0x4e42e8,void *(*)(void))();if(!indev)return;FN(0x4e45bc,void (*)(void *,int *))(indev,pt);
 int dx=pt[0]-tapx,dy=pt[1]-tapy;
 /* Bottom-edge touches bubble to the unchanged stock shade close handler.
  * Content down/up gestures expand/collapse the extra control row. */
 if(dy*dy>400&&dy*dy>dx*dx){
  if(dy<0&&folded){
   LvObj *shade=LV_GLOBAL(0x8deee4);if(!shade||!lv_valid(shade))return;
   /* Same animation fields and callback used by stock 0x47f74c. */
   unsigned animation[32];FN(0x51b3e0,void (*)(void *))(animation);
   animation[0]=(unsigned)shade;animation[1]=0x4eb600;
   animation[8]=animation[9]=*(unsigned *)((char *)shade+24);
   animation[10]=(unsigned)-360;animation[11]=250;
   FN(0x51b7b0,void *(*)(void *))(animation);lastfold=now();
  }else {int want=dy<0;if(folded!=want)fold_change();}
 }
}
static void menu_touch(void *e){menu_gesture(e,1);}
static void brightness_touch(void *e){menu_gesture(e,0);}
static void animate(LvObj *o,void (*execute)(LvObj *,int),int from,int to){
 unsigned animation[32];FN(0x51b3e0,void (*)(void *))(animation);
 animation[0]=(unsigned)o;animation[1]=(unsigned)execute;
 animation[8]=animation[9]=(unsigned)from;animation[10]=(unsigned)to;animation[11]=250;
 FN(0x51b7b0,void *(*)(void *))(animation);
}
static void menu_height(LvObj *o,int height){lv_size(o,360,height);}
static void bright_move(LvObj *o,int y){bright_current_y=y;lv_align(o,2,0,y);}
static void menu_layout(void){
 if(!menu_clip||!lv_valid(menu_clip)||menu_fold_state==folded)return;
 int target=folded?96:194;
 if(menu_fold_state<0)lv_size(menu_clip,360,target);
 else {int *box=(int *)((char *)menu_clip+20);animate(menu_clip,menu_height,box[3]-box[1]+1,target);}
 menu_fold_state=folded;
}
static void toggle(void *e){if(!control_clicked(e))return;enabled=!enabled;refreshbutton();value[0]=value[1]=0;lastadvance=0;
 int f=open("/usr/data/disc-runtime/module-data/disc-bt-meter/display-enabled.new",1|0x100|0x200,0600);if(f>=0){char c=enabled?'1':'0';int n=write(f,&c,1);close(f);if(n==1)rename("/usr/data/disc-runtime/module-data/disc-bt-meter/display-enabled.new","/usr/data/disc-runtime/module-data/disc-bt-meter/display-enabled");}
 show(layer,active&&enabled&&*(unsigned *)0x8e16d4==3);
}
static void toggle_rate(void *e){if(control_clicked(e))visual_source_toggle();}
#include "toolbar_status.h"
static void eq_return(void){
 /* The native EQ swipe invokes this before deleting its own page. */
 if(*(unsigned *)0x8e16d4==1)FN(0x475878,void (*)(void))();
 else if(*(unsigned *)0x8e16d4==3)FN(0x477e8c,void (*)(void))();
 else if(dlna_visual())FN(0x4777e0,void (*)(void))();
 else FN(0x461760,void (*)(void))();
 disc_toolbar_restore(&eq_toolbar_status);eq_toolbar_status.bar=0;
}
#include "pure_eq_route.h"
static void open_eq(void *e){
 if(!control_clicked(e)||!visual_mode())return;
 eq_toolbar_status=disc_toolbar_capture();
 FN(0x479090,void (*)(void))();LvObj *shade=LV_GLOBAL(0x8deee4);
 if(shade&&lv_valid(shade)){show(shade,0);FN(0x4eb600,void (*)(LvObj *,int))(shade,-360);}
 FN(0x4670ac,void (*)(void (*)(void)))(eq_return);
}
static void rotate_click(void *e){if(!control_clicked(e)||!visual_mode())return;volatile unsigned char *rotation=(void *)0x8e171b;*rotation=(*rotation+1)&3u;FN(0x464708,void (*)(void))();}
static void control(LvObj *shade,LvObj **btn,LvObj **icon,LvObj **label,int x,int y,void (*cb)(void *),const void *picture,const char *name){
 *btn=lv_btn(shade);lv_size(*btn,64,64);lv_align(*btn,1,x,y);stylezero(*btn);lv_add_flag(*btn,2|0x4000);lv_radius(*btn,32,0);opa(*btn,0,0);
 lv_bg(*btn,0xff454545,0x20);opa(*btn,255,0x20);
 lv_add_cb(*btn,control_touch,1,0);lv_add_cb(*btn,control_touch,2,0);lv_add_cb(*btn,control_touch,8,0);lv_add_cb(*btn,control_touch,3,0);lv_add_cb(*btn,cb,7,0);lv_add_cb(*btn,deleted,33,0);
 *icon=img(*btn);stylezero(*icon);src(*icon,picture);lv_align(*icon,9,0,0);
 *label=lv_label(shade);stylezero(*label);smallfont(*label,font16);statictext(*label);lv_color(*label,0xffcccccc,0);textalign(*label,2,0);lv_size(*label,120,30);lv_align(*label,1,x-28,y+66);lv_text(*label,name);lv_add_cb(*label,deleted,33,0);
 lv_add_flag(*label,2|0x4000);lv_add_cb(*label,control_touch,1,0);lv_add_cb(*label,control_touch,2,0);lv_add_cb(*label,control_touch,8,0);lv_add_cb(*label,control_touch,3,0);lv_add_cb(*label,cb,7,0);
}
#include "dsp_ui.h"
#include "usb_output.h"
static void usb_deleted(void *e){validation_count=0;LvObj *o=lv_event_target(e);if(o==usb_button)usb_button=usb_glyph=0;if(o==usb_caption)usb_caption=0;}
static void fold_change(void){
 folded=!folded;lastfold=now();if(fold_glyph)src(fold_glyph,folded?&fold_icon_0:&fold_icon_1);
 int f=open("/usr/data/disc-runtime/module-data/disc-bt-meter/menu-expanded-v2.new",1|0x100|0x200,0600);if(f>=0){char c=folded?'0':'1';int n=write(f,&c,1);close(f);if(n==1)rename("/usr/data/disc-runtime/module-data/disc-bt-meter/menu-expanded-v2.new","/usr/data/disc-runtime/module-data/disc-bt-meter/menu-expanded-v2");}
}
static void brightness_layout(LvObj *bright,int restore){
 LvObj *holder=parent(bright);if(!holder||!lv_valid(holder))return;
 if(!restore&&holder!=bright_holder){bright_holder=holder;int *box=(int *)((char *)holder+20);holder_height=box[3]-box[1]+1;original_holder_flags=*(unsigned *)((char *)holder+0x24)&(2|16|0x4000);lv_add_cb(holder,deleted,33,0);lv_add_cb(holder,brightness_touch,1,0);lv_add_cb(holder,brightness_touch,2,0);lv_add_cb(holder,brightness_touch,8,0);}
 /* Transparent expanded holder must not intercept the bottom exit region.
  * Native brightness bubbles only to this holder, like the pure-mode grid,
  * so its vertical gesture expands content instead of closing the shade. */
 if(restore){lv_clear_flag(holder,(2|16|0x4000)&~original_holder_flags);lv_add_flag(holder,original_holder_flags);}else lv_clear_flag(holder,2|16|0x4000);
 int *hb=(int *)((char *)holder+20),*bb=(int *)((char *)bright+20);
 int target_h=restore?(holder_height?holder_height:160):280,target_w=restore?340:280;
 if(hb[2]-hb[0]+1!=360||hb[3]-hb[1]+1!=target_h)lv_size(holder,360,target_h);
 if(bb[2]-bb[0]+1!=target_w||bb[3]-bb[1]+1!=61)lv_size(bright,target_w,61);
 LvObj *slider=LV_GLOBAL(0x8deeb0);if(slider&&lv_valid(slider)){int *sb=(int *)((char *)slider+20),width=restore?221:174;if(sb[2]-sb[0]+1!=width||sb[3]-sb[1]+1!=23)lv_size(slider,width,23);}
 if(restore){FN(0x51b4b8,int (*)(void *,void *))(bright,bright_move);bright_fold_state=-1;lv_align(bright,2,0,*(unsigned char *)0x8dee8e?0:90);}
 else if(bright_fold_state!=folded){
  int target=visual_toolbar_brightness_offset(folded);
  if(bright_fold_state<0)bright_current_y=target;
  else animate(bright,bright_move,bright_current_y,target);
  bright_fold_state=folded;
 }
 /* Stock compact-menu refresh can reset the brightness x/y after opening.
  * Reapply the animation's CURRENT position before each draw, never its end. */
 if(!restore&&(bb[0]!=hb[0]+40||bb[1]!=hb[1]+bright_current_y))lv_align(bright,2,0,bright_current_y);
}
extern void *memcpy(void *,const void *,unsigned),*memset(void *,int,unsigned);
#include "dpu_meter.h"
#include "xy_page.h"
static void shade_capture_main(void){
 unsigned *base=(void *)DU(0x8e0924),frame=DU(0x8e0914);
 if(!base||frame>=2||DU(0x8e0910)!=518400)return;
 dpu_main_rotation=display_rotation();memcpy(dpu_frozen_xy,base+frame*129600,518400);
 if(*(unsigned *)0x8e16d4==3&&enabled&&dpu_on){
  for(unsigned j=0;j<128;j++){const Tile *tile=&tiles[j];int state=dpu_fill[j/64]-(int)(j%64)*4;if(state<0)state=0;if(state>4)state=4;const unsigned *source=tile->image[state].data;
   for(int y=0;y<tile->h;y++)for(int x=0;x<tile->w;x++){unsigned pixel=source[y*tile->w+x],alpha=pixel>>24;if(!alpha)continue;unsigned i=disc_physical_index(tile->x+x,tile->y+y,dpu_main_rotation),back=dpu_frozen_xy[i],out=0xff000000u;
    for(unsigned c=0;c<3;c++){unsigned a=(pixel>>(c*8))&255,b=(back>>(c*8))&255;out|=((a*alpha+b*(255-alpha)+127)/255)<<(c*8);}dpu_frozen_xy[i]=out;
   }
  }
 }
 for(unsigned i=0;i<129600;i++)dpu_frozen_xy[i]|=0xff000000u;
 dpu_main_ready=1;dpu_frozen_kind=2;if(!++dpu_main_generation)++dpu_main_generation;for(unsigned f=0;f<3;f++)dpu_content[f]=0;
}
static void shade_rotate_main(void){
 unsigned rotation=display_rotation();if(!dpu_main_ready||rotation==dpu_main_rotation)return;
 for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++)xy_snapshot[y*360+x]=dpu_frozen_xy[disc_physical_index(x,y,dpu_main_rotation)];
 for(unsigned y=0;y<360;y++)for(unsigned x=0;x<360;x++)dpu_frozen_xy[disc_physical_index(x,y,rotation)]=xy_snapshot[y*360+x];
 dpu_main_rotation=rotation;if(!++dpu_main_generation)++dpu_main_generation;for(unsigned f=0;f<3;f++)dpu_content[f]=0;
}
#include "pure_backdrop.h"
static void setupleds(LvObj *panel){
 layer=lv_create(panel);stylezero(layer);lv_size(layer,360,360);lv_align(layer,1,0,0);opa(layer,0,0);lv_add_cb(layer,deleted,33,0);
 for(unsigned i=0;i<128;i++){led[i]=img(layer);stylezero(led[i]);lv_align(led[i],1,tiles[i].x,tiles[i].y);src(led[i],&tiles[i].image[0]);}
 drawn[0]=drawn[1]=0;
}
void disc_bt_activate(int yes){
 if(!configured){guarded=1;for(unsigned i=0;i<sizeof(bt_guards)/sizeof(bt_guards[0]);i++)if(memcmp((void *)bt_guards[i].address,bt_guards[i].bytes,16))guarded=0;char b[8];if(textread(DISC_VS_PREF,b,sizeof(b)))visual_post=b[0]=='1';if(textread("/usr/data/disc-runtime/module-data/disc-bt-meter/display-enabled",b,sizeof(b)))enabled=b[0]=='1';if(textread("/usr/data/disc-runtime/module-data/disc-bt-meter/realtime-enabled",b,sizeof(b)))rate_enabled=b[0]=='1';if(textread("/usr/data/disc-runtime/module-data/disc-bt-meter/menu-expanded-v2",b,sizeof(b)))folded=b[0]!='1';if(textread("/usr/data/disc-runtime/module-data/disc-bt-meter/usbdac-info-enabled",b,sizeof(b)))dac_info_enabled=b[0]=='1';if(textread("/usr/data/disc-runtime/module-data/disc-bt-meter/usbdac-normalize-enabled",b,sizeof(b)))dac_normalize_enabled=b[0]=='1';if(textread("/usr/data/disc-runtime/module-data/disc-bt-meter/dlna-info-enabled",b,sizeof(b)))net_info_enabled=b[0]=='1';if(textread("/usr/data/disc-runtime/module-data/disc-bt-meter/dlna-normalize-enabled",b,sizeof(b)))net_normalize_enabled=b[0]=='1';configured=1;}
 active=yes&&guarded;if(!active){xy_reset_page();dpu_stop();forget();decoderclose();monitor_close();show(layer,0);show(button,0);show(caption,0);}
}
int disc_bt_fast(void){return fast_ui;}
#include "visual_refresh.h"
#include "usbdac_menu.h"
#include "../dlna/info.h"
#include "../usbdac-dop/dop_display.h"
#include "usbdac_info.h"
#include "../visual-signal/preference.h"
#include "pure_shade.h"
#include "shade_geometry.h"
#include "work_modes_menu.h"
#include "visual_toolbar_edit.h"
static void prepare_frame(void){
 if(!configured||!guarded)return;dsp_ui_poll();unsigned mode=*(unsigned *)0x8e16d4;
 vt_guard(mode);
 pure_prepare(mode);
 work_modes_prepare();
 unsigned stamp=now();if(!usbdac_checked||stamp-usbdac_checked>=500){usbdac_checked=stamp;char capability[32];usbdac_enabled=access("/usr/data/disc-runtime/components/disc-usbdac-visuals/disabled",0)&&access("/tmp/sdcard/DISC_MODULES_SAFE_MODE",0)&&textread("/usr/data/disc-runtime/components/disc-usbdac-visuals/current/FEATURES",capability,sizeof(capability))&&!strcmp(capability,"usbdac-visuals-abi1\n");}
 if(mode!=last_visual_mode){if(xy_cover&&lv_valid(xy_cover))FN(0x4e9994,void (*)(LvObj *))(xy_cover);xy_reset_page();dpu_stop();forget();last_visual_mode=mode;}
 usbdac_menu_visibility(mode);LvObj *bright=LV_GLOBAL(0x8deeb4);
 LvObj *badge=LV_GLOBAL(0x8de3b4);
 LvObj *panel=LV_GLOBAL(dlna_visual()?0x8de290:mode==1?0x8de21c:0x8de3e4);
 LvObj *shade=LV_GLOBAL(0x8deee4);
 LvObj *screen=lv_active_screen(lv_disp_default());
 unsigned shade_height=active?shade_visible_height(shade):0;
 int pure_menu=active&&mode==0&&shade_height>0;
 /* The shade is a global top-layer object. It can outlive the page that
  * opened it; never move its frozen background/black cover onto a new page. */
 if(pure_shade_owner&&(screen!=pure_shade_owner||mode!=0||!active)){
  if(shade&&lv_valid(shade)){
   FN(0x51b4b8,int (*)(void *,void *))(shade,0);
   show(shade,0);FN(0x4eb600,void (*)(LvObj *,int))(shade,-360);
  }
  pure_shade_owner=0;pure_tracking=0;pure_menu=0;shade_height=0;
 }
 if(pure_menu){if(!pure_shade_owner)pure_shade_owner=screen;}else pure_shade_owner=0;
 LvObj *volume=LV_GLOBAL(0x8def7c);native_overlay_open=volume&&lv_valid(volume)&&!(*(unsigned *)((char *)volume+0x24)&1);
 menu_open=active&&visual_mode()&&shade_height>0;
 int visible=panel&&lv_valid(panel)&&lv_active_screen(lv_disp_default())==panel;meter_visible=visible&&active&&visual_mode();
 xy_bind(online_music_session()?0:panel);
 unsigned refresh_period=0;
 if(active&&((mode==0&&shade_height>0)||(visual_mode()&&(menu_open||(((mode==3&&enabled)||xy_progress||xy_dragging||xy_animating)&&visible)))))refresh_period=16;
 else if(active&&mode==1&&usbdac_enabled&&visible&&!native_overlay_open&&DU(0x83abd0))refresh_period=33;
 frameperiod(refresh_period);
 if(fast_ui)refreshes++;
 /* Stock 4813b0 makes the shade a full-width opaque 360px rectangle. Its
  * touch objects stay native; the frozen XY stays in the hardware plane. */
 if(!pure_menu&&(!menu_open||!active||!visual_mode()||!visible))dpu_main_ready=0;
 if(pure_menu&&!dpu_main_ready)shade_capture_main();
 if(menu_open&&active&&visual_mode()&&visible&&!xy_progress&&xy_cover&&!dpu_main_ready)shade_capture_main();
 if(dpu_main_ready)shade_rotate_main();
 dpu_pure_shade=0;
 dpu_main_shade=active&&visual_mode()&&visible&&!xy_progress&&menu_open&&dpu_main_ready&&!native_overlay_open;
 dpu_xy_shade=active&&visual_mode()&&visible&&xy_progress==360&&xy_frame&&menu_open&&!native_overlay_open;
 dpu_shade_height=(dpu_xy_shade||dpu_main_shade)?shade_height:0;
 dpu_dac_home=active&&mode==1&&usbdac_enabled&&visible&&!xy_progress&&!xy_dragging&&!xy_animating&&!menu_open&&!native_overlay_open&&DU(0x83abd0);
 dpu_base_only=dpu_dac_home||(active&&visual_mode()&&visible&&xy_progress&&xy_frame&&(menu_open||native_overlay_open)&&!dpu_xy_shade);
 dpu_begin(active&&(dpu_dac_home||dpu_pure_shade||(visual_mode()&&((mode==3&&enabled)||xy_progress||dpu_main_shade)&&visible&&(!(menu_open||native_overlay_open)||dpu_base_only||dpu_xy_shade||dpu_main_shade)))&&DU(0x83abd0));
 pure_backdrop_prepare(screen,pure_menu&&dpu_main_ready);
 xy_prepare_picture();
 dac_info_prepare(panel,meter_visible);
 if(!active||mode!=3){if(menu_clip&&lv_valid(menu_clip)&&menu_fold_state>=0){FN(0x51b4b8,int (*)(void *,void *))(menu_clip,menu_height);menu_fold_state=-1;}show(menu_clip,0);if(gesture_grid&&lv_valid(gesture_grid)&&!original_grid_bubble)lv_clear_flag(gesture_grid,0x4000);restore_layout();show(layer,0);show(button,0);show(caption,0);show(rate_button,0);show(rate_caption,0);show(eq_button,0);show(eq_caption,0);show(usb_button,0);show(usb_caption,0);show(fold_button,0);if(shifted&&bright&&lv_valid(bright)){lv_clear_flag(bright,0x60000&~original_bright_flags);brightness_layout(bright,1);shifted=0;}if(badge_shifted&&badge&&lv_valid(badge)){lv_align(badge,2,0,311);badge_shifted=0;}usbdac_prepare_menu();if(active&&mode==1&&menu_open&&dac_menu_mode())vt_prepare(1,dac_shade,dac_clip,dac_buttons,dac_captions);return;}
 if(panel&&lv_valid(panel)&&!layer)setupleds(panel);
 int fully_covered=menu_open&&*(int *)((char *)shade+24)>=0;
 show(layer,enabled&&!xy_progress&&!fully_covered&&!dpu_on);
 if(!menu_open&&panel&&lv_valid(panel))compact_layout(panel);
 /* No menu object/layout work for a shade that is fully closed. Build and
  * restore its native positions before the first visible opening frame. */
 if(!menu_open)return;
 ensurefonts();
 if(shade&&lv_valid(shade)&&shade!=gesture_shade){gesture_shade=shade;lv_add_cb(shade,menu_touch,1,0);lv_add_cb(shade,menu_touch,2,0);lv_add_cb(shade,menu_touch,8,0);lv_add_cb(shade,deleted,33,0);}
 LvObj *grid=LV_GLOBAL(0x8dee94);if(grid&&lv_valid(grid)){if(grid!=gesture_grid){gesture_grid=grid;original_grid_bubble=*(unsigned *)((char *)grid+0x24)&0x4000;lv_add_cb(grid,deleted,33,0);}lv_add_flag(grid,0x4000);}
 if(shade&&lv_valid(shade)&&!menu_clip){menu_clip=lv_create(shade);stylezero(menu_clip);lv_add_flag(menu_clip,0x4000);opa(menu_clip,0,0);lv_size(menu_clip,360,folded?96:194);lv_align(menu_clip,1,0,68);lv_add_cb(menu_clip,deleted,33,0);menu_fold_state=-1;}
 show(menu_clip,1);menu_layout();
 if(shade&&lv_valid(shade)&&!button){
  control(menu_clip,&button,&level_glyph,&caption,49,0,toggle,&level_icon_0,"电平");refreshbutton();
 }
 if(shade&&lv_valid(shade)&&!rate_button){
  control(menu_clip,&rate_button,&rate_glyph,&rate_caption,148,0,toggle_rate,&rate_icon_0,"信号 原始");refreshratebutton();
 }
 if(shade&&lv_valid(shade)&&!eq_button){control(menu_clip,&eq_button,&eq_glyph,&eq_caption,247,0,open_eq,"D:/usr/project/config/img/sysui/eq_n.png","均衡器");eq_seen=-1;}
 /* Same preset byte and original image pointers as stock toolbar 0x47fc00.
  * Native zero=off; presets/custom selections are highlighted in red. */
 int eq_mode=*(unsigned char *)0x8e1720;
 if(eq_glyph&&eq_seen!=eq_mode){src(eq_glyph,*(const void **)(eq_mode?0x83a6d8:0x83a6d4));eq_seen=eq_mode;}
 show(eq_button,1);show(eq_caption,1);
 if(shade&&lv_valid(shade)&&!usb_button){control(menu_clip,&usb_button,&usb_glyph,&usb_caption,49,98,usb_click,&usb_icon_0,"USB 输出 关");lv_add_cb(usb_button,usb_deleted,33,0);lv_add_cb(usb_caption,usb_deleted,33,0);usb_text[0]=0;}
 if(shade&&lv_valid(shade)&&!rotate_button){control(menu_clip,&rotate_button,&rotate_glyph,&rotate_caption,148,98,rotate_click,*(const void **)0x83a6f4,"旋转屏幕");rotate_seen=-1;}
 if(shade&&lv_valid(shade)&&!dsp_button){control(menu_clip,&dsp_button,&dsp_glyph,&dsp_caption,247,98,open_dsp,&dsp_icon_0,"DSP");lv_add_cb(dsp_button,dsp_ui_deleted,33,0);lv_add_cb(dsp_caption,dsp_ui_deleted,33,0);}
 show(dsp_button,1);show(dsp_caption,1);
 unsigned rotation=*(unsigned char *)0x8e171b;if(rotate_glyph&&rotate_seen!=(int)rotation){src(rotate_glyph,*(const void **)(0x83a6f4+(rotation&3u)*4));rotate_seen=(int)rotation;}
 show(rotate_button,1);show(rotate_caption,1);
 show(usb_button,1);show(usb_caption,1);
 show(fold_button,0);
 show(rate_button,1);show(rate_caption,1);
 show(button,1);show(caption,1);
 if(bright&&lv_valid(bright)){if(!shifted)original_bright_flags=*(unsigned *)((char *)bright+0x24)&0x60000;lv_add_flag(bright,0x60000);brightness_layout(bright,0);shifted=1;}
 LvObj *sort_buttons[6]={button,rate_button,eq_button,usb_button,rotate_button,dsp_button},*sort_labels[6]={caption,rate_caption,eq_caption,usb_caption,rotate_caption,dsp_caption};vt_prepare(3,shade,menu_clip,sort_buttons,sort_labels);
}
void disc_bt_prepare(void){validation_frame=1;validation_count=0;prepare_frame();validation_frame=0;validation_count=0;}
void disc_bt_poll(void){
 unsigned t=now();usb_debug_poll(t);xy_poll(t);
 unsigned vs_mode=*(unsigned *)0x8e16d4;
 int vs_visible=active&&meter_visible&&!menu_open&&!native_overlay_open&&DU(0x83abd0)&&!dlna_visual()&&(xy_progress||(vs_mode==1&&dac_home_live)||(vs_mode==3&&enabled));
 vs_ui_update(t,visual_post&&vs_visible,vs_mode==1?4:vs_mode==3?2:0);
 if(!active){if(usb_phase){discover(t);usb_poll(t);}return;}
 dop_display_poll(t);
 dac_info_poll(t);
 network_info_poll(t);
 /* Status is RAM-only and written at most once per second. */
 if(t-laststate>=1000){laststate=t;char b[2048];int *bc=button?(int *)((char *)button+20):0,*lc=layer?(int *)((char *)layer+20):0,*rc=LV_GLOBAL(0x8deeb4)?(int *)((char *)LV_GLOBAL(0x8deeb4)+20):0;int n=snprintf(b,sizeof(b),"module=1\ndisplay=%d\nmode=%u\npid=%d\nmem_open=%d\nL=%d\nR=%d\nbits=%d\nlast_age_ms=%u\nbutton=%d,%d,%d,%d\nlayer=%d,%d,%d,%d\nbrightness=%d,%d,%d,%d\n",enabled,*(unsigned *)0x8e16d4,playerpid,memfd>=0,value[0]/256,value[1]/256,lastbits,lastadvance?t-lastadvance:0,bc?bc[0]:0,bc?bc[1]:0,bc?bc[2]:0,bc?bc[3]:0,lc?lc[0]:0,lc?lc[1]:0,lc?lc[2]:0,lc?lc[3]:0,rc?rc[0]:0,rc?rc[1]:0,rc?rc[2]:0,rc?rc[3]:0);
  n+=snprintf(b+n,sizeof(b)-n,"samples=%u\nvalid_samples=%u\ninterval_ms=%u\nmin_interval_ms=%u\nmax_interval_ms=%u\nrefresh_callbacks=%u\nfast_ui=%d\ncodec=%s\nrate=%u\npcm_bits=%u\nbitrate_kbps=%u\n",attempts,valid_samples,last_interval,min_interval==0xffffffffu?0:min_interval,max_interval,refreshes,fast_ui,metacodec,metarate,metabits,metabitrate);
  n+=snprintf(b+n,sizeof(b)-n,"realtime_kbps=%u\nv5_frame_kbps=%u\nmonitor_open=%d\nmonitor_packets=%u\nmonitor_invalid=%u\nmonitor_capped=%u\nusb_phase=%d\nusb_output=%d\nusb_route=%d\nusb_previous=%d\nmenu_open=%d\n",realtime_kbps,v5_bitrate_current(t),monitorfd>=0,monitor_packets,monitor_invalid,monitor_capped,usb_phase,usb_actual,usb_route,usb_previous,menu_open);
  n+=snprintf(b+n,sizeof(b)-n,"usb_retry_count=%u\nusb_source_retry_count=%u\nusb_host_role=%d\nusb_status=%s\n",usb_retries,usb_source_retries,usb_host_ready,usb_text);
  n+=snprintf(b+n,sizeof(b)-n,"v5_restarts=%u\nv5_gaps=%u\nv5_invalid_headers=%u\nv5_ignored=%u\n",v5_restarts,v5_gaps,v5_invalid_headers,v5_ignored);
  dpu_status(1);n+=snprintf(b+n,sizeof(b)-n,"dpu=%d\ndpu_failed=%d\ndpu_frames=%u\nbase_copy_bytes=%u\nbase_fullcopies=%u\n",dpu_on,dpu_failed,dpu_frames,dpu_copied,dpu_fullcopies);dpu_frames=dpu_copied=dpu_fullcopies=0;
  n+=snprintf(b+n,sizeof(b)-n,"shade_hardware=%d\nshade_background=%u\nshade_height=%u\n",dpu_on&&(dpu_xy_shade||dpu_main_shade),dpu_frozen_kind,dpu_shade_height);
  n+=snprintf(b+n,sizeof(b)-n,"dac_home_live=%d\ndac_home_kind=%u\ndac_home_analyses=%u\ndac_home_received=%u\n",dac_home_live,dac_home_kind,dac_home_spectrum.analyses,dac_home_spectrum.received);
  n+=snprintf(b+n,sizeof(b)-n,"visual_source=%s\nvisual_post_blocks=%u\nvisual_post_sequence=%u\nvisual_post_request=%u\n",visual_post?"post_eq_dsp":"raw",vs_ui?vs_ui->blocks:0,vs_ui_last_sequence,vs_ui?vs_ui->request:0);
  n+=snprintf(b+n,sizeof(b)-n,"visual_buffer_ms=%u\n",vs_ui_clock.rate?vs_ui_clock.delay_frames*1000/vs_ui_clock.rate:0);
  attempts=valid_samples=refreshes=0;min_interval=0xffffffffu;max_interval=0;
  int f=open("/tmp/disc-runtime/bt-meter.state",1|0x100|0x200,0600);if(f>=0){write(f,b,n);close(f);}f=open("/tmp/disc-runtime/integrated-disc-bt-meter",1|0x100|0x200,0600);if(f>=0){const char release[]="releases/1.3.220\n";write(f,release,sizeof(release)-1);close(f);}
 }
 if((*(unsigned *)0x8e16d4==1&&usbdac_enabled)||dlna_visual()){
  if(memfd<0)discover(t);int home_fft=dac_home_sampling();if(menu_open||native_overlay_open||!meter_visible||(!xy_progress&&!home_fft)||!DU(0x83abd0))return;
  unsigned period=home_fft?40u:page_kind==DISC_PAGE_SPECTRUM?32u:16u;if(t-lastsample<period)return;lastsample=t;attempts++;discover(t);if(pcm_levels(t))valid_samples++;return;
 }
 if(*(unsigned *)0x8e16d4!=3){if(*(unsigned *)0x8e16d4==0&&usb_phase)discover(t);usb_poll(t);if((*(unsigned *)0x8e16d4!=0||!usb_phase)&&memfd>=0)forget();decoderclose();monitor_close();return;}
 /* The shade owns drawing. Keep the lightweight V5 observer running so
  * hidden pages retain frame/sequence continuity; pause RMS and LDAC memory
  * inspection. The realtime label switch does not disable V5 tier detection. */
 if(menu_open||!meter_visible){if(usb_phase||memfd<0)discover(t);usb_poll(t);if(!strcmp(metacodec,"LHDC V5"))monitor_poll(t,1,metacodec);else if(monitorfd>=0)monitor_close();if(decoderfd>=0)decoderclose();return;}
 discover(t);metadata(t);
 /* Cold LDAC discovery is incremental on this existing visible-page poll,
  * not paced by the slower metadata label refresh. Cached tier validation
  * remains at 250ms; the read-only predicate skips every backoff interval. */
 if(!strcmp(metacodec,"LDAC")&&metarate&&stream_flow&&decoder_probe_pending(now())){
  unsigned current=decoder_bitrate(metarate);if(current)metabitrate=current;
 }
 usb_poll(t);
 monitor_poll(t,rate_enabled||!strcmp(metacodec,"LHDC V5"),metacodec);
 if(!enabled&&!xy_progress){value[0]=value[1]=smooth[0]=smooth[1]=0;lastadvance=0;return;}
 unsigned sample_period=page_kind==DISC_PAGE_SPECTRUM?32u:16u;
 if(t-lastsample<sample_period)return;last_interval=lastsample?t-lastsample:0;lastsample=t;attempts++;if(last_interval&&last_interval<1000){if(last_interval<min_interval)min_interval=last_interval;if(last_interval>max_interval)max_interval=last_interval;}discover(t);
 if(!pcm_levels(t)){value[0]=value[1]=0;lastadvance=0;}else valid_samples++;
 if(xy_progress)return;
 unsigned dt=lastanimation?t-lastanimation:16;lastanimation=t;if(dt>80)dt=80;
 for(unsigned c=0;c<2;c++){int delta=value[c]-smooth[c],tau=delta>0?28:130;
  if(delta){int step=delta*(int)dt/(tau+(int)dt);if(!step)step=delta>0?1:-1;smooth[c]+=step;}
  int fill=(smooth[c]+50)/100;if(fill>256)fill=256;if(fill<0)fill=0;if(fill==drawn[c])continue;
  int old=drawn[c]<0?0:drawn[c],first=drawn[c]<0?0:(old<fill?old:fill)/4,last=drawn[c]<0?63:(old>fill?old:fill)/4;if(last>63)last=63;
  for(int j=first;j<=last;j++){int state=fill-j*4;if(state<0)state=0;if(state>4)state=4;unsigned i=c*64+j;if(!dpu_on&&led[i])src(led[i],&tiles[i].image[state]);}drawn[c]=fill;dpu_fill[c]=fill;}
}

