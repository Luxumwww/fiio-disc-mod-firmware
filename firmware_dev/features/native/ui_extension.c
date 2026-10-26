#include "stock_lvgl.h"
#include "spectrum_view.h"
#include "ui_guard_generated.h"
extern void *dlsym(void *,const char *),*malloc(unsigned long),*memcpy(void *,const void *,unsigned long);
extern void free(void *);
extern int memcmp(const void *,const void *,unsigned long),strcmp(const char *,const char *);
extern unsigned long getauxval(unsigned long);
extern int open(const char *,int,...),close(int),*__errno_location(void),clock_gettime(int,void *);
extern long read(int,void *,unsigned long),write(int,const void *,unsigned long);
extern int pthread_mutex_lock(void *),pthread_mutex_unlock(void *),snprintf(char *,unsigned long,const char *,...);
static int (*original_sleep)(unsigned);
static unsigned guarded,enabled,last_frame,loop_seen;
static LvObj *image,*button,*button_text,*labels[5];
static unsigned ref_pixels[360*360],frame_pixels[360*360];
typedef struct {unsigned dest;unsigned short band,delta;short cos,sin;} Warp;
static Warp *warps;
static struct {unsigned header,size;const void *data;} image_desc={4u|(360u<<10)|(360u<<21),360*360*4,frame_pixels};
static DiscView latest;
static DiscView drawn_view;static unsigned drawn,placed_mode=2;static char drawn_volume[32],label_values[5][64];
static unsigned now_ms(void){long t[2];if(clock_gettime(1,t))return 0;return (unsigned)t[0]*1000+(unsigned)t[1]/1000000;}
static void message(const char *s){unsigned n=0;while(s[n])n++;write(2,s,n);}
static int read_exact(const char *path,void *data,unsigned bytes){
 int fd=open(path,0);if(fd<0)return 0;
 unsigned done=0;while(done<bytes){long n=read(fd,(char *)data+done,bytes-done);if(n<=0)break;done+=(unsigned)n;}
 char extra;long more=read(fd,&extra,1);close(fd);return done==bytes&&more==0;
}
static int executable_matches(void){
 const char *path=(const char *)getauxval(31);if(!path)return 0;
 const char *base=path;for(const char *p=path;*p;p++)if(*p=='/')base=p+1;
 const char expected[]="mq_ui";unsigned i=0;while(base[i]&&base[i]==expected[i])i++;if(base[i]||expected[i])return 0;
 int fd=open(path,0);if(fd<0)return 0;
 unsigned hash=2166136261u,total=0;char bytes[4096];long n;
 while((n=read(fd,bytes,sizeof(bytes)))>0){for(long j=0;j<n;j++){hash^=(unsigned char)bytes[j];hash*=16777619u;}total+=(unsigned)n;}
 close(fd);if(n<0||hash!=DISC_UI_FNV32||total!=DISC_UI_SIZE)return 0;
 for(unsigned j=0;j<DISC_UI_GUARDS;j++){
  const unsigned char *code=(const unsigned char *)(unsigned long)disc_ui_guards[j].address;
  for(unsigned k=0;k<16;k++)if(code[k]!=disc_ui_guards[j].bytes[k])return 0;
 }
 return 1;
}
__attribute__((constructor)) static void initialize(void){
 original_sleep=dlsym((void *)-1,"usleep");
 int flag=open("/usr/data/fiio/disc-spectrum.activate",0);if(flag<0)return;close(flag);
 if(!executable_matches()){message("DISC_UI_GUARD_REJECTED\n");return;}
 warps=malloc(DISC_WARP_COUNT*sizeof(Warp));if(!warps)return;
 if(!read_exact("/usr/project/disc-spectrum/spectrum.bgrx",ref_pixels,sizeof(ref_pixels))||
    !read_exact("/usr/project/disc-spectrum/spectrum-warp.bin",warps,DISC_WARP_COUNT*sizeof(Warp))){free(warps);warps=(void *)0;message("DISC_UI_ARTWORK_MISSING\n");return;}
 for(unsigned j=0;j<DISC_WARP_COUNT;j++)if(warps[j].dest>=360*360||warps[j].band>=72){free(warps);warps=(void *)0;return;}
 memcpy(frame_pixels,ref_pixels,sizeof(frame_pixels));
 flag=open("/usr/data/fiio/disc-spectrum.enabled",0);if(flag>=0){char v=0;read(flag,&v,1);enabled=v=='1';close(flag);}
 if(!disc_audio_start()){free(warps);warps=(void *)0;message("DISC_UI_AUDIO_WORKER_FAILED\n");return;}
 guarded=1;message("DISC_UI_GUARD_OK\n");
}
__attribute__((destructor)) static void finish(void){if(guarded)disc_audio_stop();if(warps)free(warps);}
static void show(LvObj *obj,unsigned yes){if(obj&&lv_valid(obj)){unsigned hidden=(*(const unsigned *)((const char *)obj+0x24)&1)!=0;if(hidden==!yes)return;if(yes)lv_clear_flag(obj,DISC_LV_HIDDEN);else lv_add_flag(obj,DISC_LV_HIDDEN);}}
static void deleted(void *event){
 LvObj *target=lv_event_target(event);
 if(target==image){image=(void *)0;for(unsigned i=0;i<5;i++)labels[i]=(void *)0;}
 if(target==button){button=button_text=(void *)0;}
}
static void button_refresh(void){
 lv_text(button_text,enabled?"频谱\n开":"频谱\n关");
 lv_bg(button,enabled?0xff176c94:0xff2a2a2a,0);
}
static void toggle(void *event){
 if(lv_event_code(event)!=DISC_LV_CLICKED)return;
 enabled=!enabled;button_refresh();show(image,enabled);
 /* Persist only a user click, outside the audio thread. Failure keeps session choice. */
 int fd=open("/usr/data/fiio/disc-spectrum.enabled",0x301,0644);
 if(fd>=0){char v=enabled?'1':'0';write(fd,&v,1);close(fd);}
 last_frame=0;message(enabled?"DISC_UI_SPECTRUM_ON\n":"DISC_UI_SPECTRUM_OFF\n");
}
static LvObj *text(LvObj *parent,unsigned size,unsigned color,int y,const char *value){
 LvObj *obj=lv_label(parent);if(!obj)return (void *)0;
 void **style=lv_font_style(size);if(style&&*style)lv_add_style(obj,*style,0);
 lv_color(obj,color,0);lv_clear_flag(obj,DISC_LV_CLICKABLE);lv_text(obj,value);lv_align(obj,9,0,y);return obj;
}
static void setup_button(unsigned usb){
 LvObj *pc=LV_GLOBAL(0x8deec0),*phone=LV_GLOBAL(0x8deebc);
 if(!pc||!phone||!lv_valid(pc)||!lv_valid(phone))return;
 if(!button){
  LvObj *parent=lv_parent(pc);if(!parent||!lv_valid(parent))return;
  button=lv_btn(parent);if(!button)return;
  lv_size(button,64,64);lv_align(button,1,247,84);lv_radius(button,32,0);lv_border(button,0,0);
  button_text=text(button,16,0xffb9e8ff,0,"频谱\n关");
  lv_add_cb(button,toggle,DISC_LV_CLICKED,(void *)0);lv_add_cb(button,deleted,DISC_LV_DELETE,(void *)0);
  button_refresh();message("DISC_UI_SHADE_BUTTON_CREATED\n");
  placed_mode=2;
 }
 show(button,usb);
 if(placed_mode!=usb){lv_align(pc,1,usb?49:100,84);lv_align(phone,1,usb?148:196,84);placed_mode=usb;}
}
static void setup_image(LvObj *panel){
 image=lv_img(panel);if(!image)return;
 lv_img_src(image,&image_desc);lv_align(image,9,0,0);lv_clear_flag(image,DISC_LV_CLICKABLE|DISC_LV_SCROLLABLE);
 lv_add_cb(image,deleted,DISC_LV_DELETE,(void *)0);
 labels[0]=text(image,20,0xffdcefff,-30,"-- kHz");
 labels[1]=text(image,16,0xff439bdd,-10,"-- bit");
 labels[2]=text(image,24,0xff53f4ff,20,"VOL --");
 labels[3]=text(image,12,0xffc2e3ff,48,"L --     -- R");
 labels[4]=text(image,12,0xff8abff0,84,"未连接");
 show(image,enabled);message("DISC_UI_IMAGE_CREATED\n");
 drawn=0;for(unsigned i=0;i<5;i++)label_values[i][0]=0;
}
static void parameter(unsigned index,const char *value,int y){if(strcmp(label_values[index],value)){snprintf(label_values[index],64,"%s",value);lv_text(labels[index],value);lv_align(labels[index],9,0,y);}}
static void render(void){
 LvObj *volume=LV_GLOBAL(0x8dee9c);const char *v=volume&&lv_valid(volume)?*(const char **)((const char *)volume+0x2c):"--";if(!v)v="--";
 if(drawn&&!memcmp(&drawn_view,&latest,sizeof(latest))&&!strcmp(drawn_volume,v))return;
 drawn=1;drawn_view=latest;snprintf(drawn_volume,sizeof(drawn_volume),"%s",v);
 memcpy(frame_pixels,ref_pixels,sizeof(frame_pixels));
 for(unsigned j=0;j<DISC_WARP_COUNT;j++){
  Warp *w=&warps[j];unsigned level=latest.live&&!latest.dop?latest.levels[w->band]:0;
  unsigned stretch=38+level*294/255,sr=119*256+(unsigned)w->delta*256/stretch;if(sr>165*256)sr=167*256;
  int x=(179*256+128+(int)w->cos*(int)sr/32768)/256;
  int y=(179*256+128+(int)w->sin*(int)sr/32768)/256;
  if(x<0)x=0;if(x>359)x=359;if(y<0)y=0;if(y>359)y=359;
  frame_pixels[w->dest]=ref_pixels[y*360+x];
 }
 for(unsigned c=0;c<2;c++){
  int amount=latest.live&&!latest.dop?(latest.db[c]+60)*16/60:0;if(amount<0)amount=0;if(amount>16)amount=16;
  for(unsigned k=0;k<16;k++)for(unsigned y=241;y<247;y++)for(unsigned dx=0;dx<3;dx++)
   frame_pixels[y*360+116+c*64+k*4+dx]=(int)k<amount?(c?0xffbb28ef:0xff00dcea):0xff26303c;
 }
 char value[64];
 if(latest.live){
  if(latest.rate%1000)snprintf(value,sizeof(value),"%u.%u kHz",latest.rate/1000,latest.rate%1000/100);
  else snprintf(value,sizeof(value),"%u kHz",latest.rate/1000);
 }else snprintf(value,sizeof(value),"-- kHz");
 parameter(0,value,-30);
 if(latest.live&&!latest.dop)snprintf(value,sizeof(value),"%u bit",latest.format==2?16:latest.format==10?32:24);
 else snprintf(value,sizeof(value),"-- bit");parameter(1,value,-10);
 snprintf(value,sizeof(value),"VOL %s",v);parameter(2,value,20);
 if(latest.live&&!latest.dop)snprintf(value,sizeof(value),"L %d     %d R",latest.db[0],latest.db[1]);else snprintf(value,sizeof(value),"L --     -- R");
 parameter(3,value,48);
 parameter(4,latest.live?(latest.dop?"DSD / DoP":"PCM"):"未连接",84);
 lv_invalidate(image);
}
static void frame(void){
 unsigned time=now_ms();if(time-last_frame<40)return;last_frame=time;
 pthread_mutex_lock((void *)0x8d9e70);
 if(!loop_seen){loop_seen=1;message("DISC_UI_MAIN_LOOP_HOOK_OK\n");}
 unsigned usb=*(unsigned *)0x8e16d4==1;setup_button(usb);
 LvObj *screen=LV_GLOBAL(0x8de21c),*panel=LV_GLOBAL(0x8de218);
 if(usb&&screen&&panel&&lv_valid(screen)&&lv_valid(panel)&&lv_active_screen(lv_disp_default())==screen){
  if(!image)setup_image(panel);show(image,enabled);
  if(image&&enabled){disc_audio_snapshot(&latest);render();}
 }else show(image,0);
 pthread_mutex_unlock((void *)0x8d9e70);
}
int usleep(unsigned microseconds){
 unsigned long caller=(unsigned long)__builtin_return_address(0);
 int result=original_sleep?original_sleep(microseconds):-1,saved=*__errno_location();
 if(guarded&&caller==0x40397c)frame();
 *__errno_location()=saved;return result;
}
