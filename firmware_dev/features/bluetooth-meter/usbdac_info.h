/* Native 360px USB DAC home; original input handling and layer ownership. */
#include "system_metrics.h"
#include "usbdac_volume.h"
#include "../usbdac-visuals/home/spectrum.h"
extern void *mmap(void *,unsigned,int,int,int,long);
extern int munmap(void *,unsigned);
extern void *localtime_r(const long *,void *);
static LvObj *dac_info_card,*dac_home_details,*dac_home_head[4],*dac_home_clock;
static LvObj *dac_home_cpu,*dac_home_mhz,*dac_home_ram,*dac_home_mb,*dac_home_volume;
static LvObj *dac_home_cpu_fill,*dac_home_ram_fill,*dac_home_hint,*dac_home_bar[14];
static LvObj *dac_home_stock[3];static unsigned dac_home_hidden[3];
static void *dac_home_fonts[7];
static unsigned dac_info_at,dac_freq_at,dac_memory_at,dac_cached_mhz,dac_clock_at;
static unsigned dac_home_kind,dac_home_rate,dac_home_bits;
static int dac_home_live,dac_home_heights[14],dac_home_info_seen=-1;
static DiscCpuSample dac_cpu_sample;
static DiscDacHomeSpectrum dac_home_spectrum;
static DiscVisualGain dac_home_gain;
static void dac_home_text(LvObj *o,const char *s){if(o&&strcmp(DISC_DOP_DISPLAY_TEXT(o),s))lv_text(o,s);}
static LvObj *dac_home_box(LvObj *p,int x,int y,int w,int h,unsigned color,int radius){
 LvObj *o=lv_create(p);stylezero(o);lv_size(o,w,h);lv_align(o,1,x,y);lv_bg(o,color,0);opa(o,255,0);lv_radius(o,radius,0);return o;
}
static LvObj *dac_home_label(LvObj *p,int x,int y,int w,int h,unsigned f,unsigned color,const char *s){
 LvObj *o=lv_label(p);stylezero(o);smallfont(o,dac_home_fonts[f]);FN(0x52f4f0,void (*)(LvObj *,unsigned))(o,4);textalign(o,1,0);lv_color(o,color,0);lv_size(o,w,h);lv_align(o,1,x,y);lv_text(o,s);return o;
}
static void dac_home_restore(void){
 for(unsigned i=0;i<3;i++){if(dac_home_stock[i]&&lv_valid(dac_home_stock[i]))show(dac_home_stock[i],!dac_home_hidden[i]);dac_home_stock[i]=0;}
}
static void dac_info_deleted(void *e){
 if(lv_event_target(e)!=dac_info_card)return;
 dac_info_card=0;dac_home_live=0;for(unsigned i=0;i<3;i++)dac_home_stock[i]=0;
 dac_info_at=dac_freq_at=dac_memory_at=dac_clock_at=0;dac_cpu_sample=(DiscCpuSample){0};
 disc_dac_home_spectrum_reset(&dac_home_spectrum);dac_home_gain=(DiscVisualGain){0};
}
static int dac_home_width(LvObj *o,const char *s){
 void *font=0;int letter=0,line=0,sz[2]={0};
 FN(0x4f3f10,void (*)(void *,LvObj *,unsigned,unsigned))(&font,o,0,0x57);
 FN(0x4f3f10,void (*)(void *,LvObj *,unsigned,unsigned))(&letter,o,0,0x58);
 FN(0x4f3f10,void (*)(void *,LvObj *,unsigned,unsigned))(&line,o,0,0x59);
 if(font)FN(0x52607c,void (*)(int *,const char *,const void *,int,int,int,unsigned))(sz,s,font,letter,line,0x1fffffff,2);
 return sz[0]>0&&sz[0]<350?sz[0]:0;
}
static void dac_home_metric(LvObj *o,const char *s,int y){
 if(!strcmp(DISC_DOP_DISPLAY_TEXT(o),s))return;
 lv_text(o,s);int w=dac_home_width(o,s);if(w>0&&w<=70){lv_size(o,w+1,33);lv_align(o,1,134-w,y);}
}
static void dac_home_header(unsigned kind,unsigned rate,unsigned bits){
 if(kind==dac_home_kind&&rate==dac_home_rate&&bits==dac_home_bits&&dac_home_info_seen==dac_info_enabled)return;
 dac_home_info_seen=dac_info_enabled;
 dac_home_kind=kind;dac_home_rate=rate;dac_home_bits=bits;
 char number[24],depth[24];const char *parts[4]={"PCM",number,"kHz",depth};
 if(kind==DISC_DAC_HOME_SPECTRUM_PCM){
  if(rate%1000)snprintf(number,sizeof(number),"%u.%u",rate/1000,rate%1000/100);else snprintf(number,sizeof(number),"%u",rate/1000);
  snprintf(depth,sizeof(depth),"%u bit",bits);if(!dac_info_enabled)parts[3]="";
 }else if(kind==DISC_DAC_HOME_SPECTRUM_DOP){parts[0]="DSD";snprintf(number,sizeof(number),"%u",rate==352800?128u:64u);parts[2]="DoP";parts[3]="";}
 else if(kind==DISC_DAC_HOME_SPECTRUM_DSD){parts[0]="DSD";parts[1]="输入";parts[2]=parts[3]="";}
 else {parts[0]="等待输入";parts[1]=parts[2]=parts[3]="";}
 smallfont(dac_home_head[1],dac_home_fonts[6]);smallfont(dac_home_head[3],dac_home_fonts[1]);
 int width[4],sum=0;for(unsigned i=0;i<4;i++){dac_home_text(dac_home_head[i],parts[i]);width[i]=*parts[i]?dac_home_width(dac_home_head[i],parts[i]):0;sum+=width[i]+(width[i]?7:0);}sum-=7;
 if(sum>280){smallfont(dac_home_head[1],dac_home_fonts[4]);smallfont(dac_home_head[3],dac_home_fonts[1]);sum=0;for(unsigned i=0;i<4;i++){width[i]=*parts[i]?dac_home_width(dac_home_head[i],parts[i]):0;sum+=width[i]+(width[i]?7:0);}sum-=7;}
 int x=(360-sum)/2;const int y[4]={76,55,81,79},height[4]={31,56,26,28};
 for(unsigned i=0;i<4;i++){show(dac_home_head[i],width[i]&&(i!=3||dac_info_enabled));if(width[i]){lv_size(dac_home_head[i],width[i]+1,height[i]);lv_align(dac_home_head[i],1,x,y[i]);x+=width[i]+7;}}
 dac_home_text(dac_home_hint,kind==DISC_DAC_HOME_SPECTRUM_PCM?"":kind==DISC_DAC_HOME_SPECTRUM_DOP?"DoP 输入":kind==DISC_DAC_HOME_SPECTRUM_DSD?"DSD 输入":"等待音频");
}
static void dac_info_prepare(LvObj *panel,int visible){
 int home=active&&usbdac_enabled&&*(unsigned *)0x8e16d4==1&&visible&&!xy_progress;
 dac_home_live=home&&!menu_open&&!native_overlay_open&&DU(0x83abd0);
 if(home&&!dac_info_card){
  const unsigned sizes[7]={14,16,20,22,28,34,40};for(unsigned i=0;i<7;i++)if(!dac_home_fonts[i]){void **f=fontstyle(sizes[i]);if(f)dac_home_fonts[i]=*f;}
  dac_info_card=dac_home_box(panel,0,0,360,360,0xff080d11,0);lv_add_cb(dac_info_card,dac_info_deleted,33,0);
  LvObj *title=dac_home_label(dac_info_card,95,29,170,22,0,0xff909da6,"USB DAC");textalign(title,2,0);
  for(unsigned i=0;i<4;i++)dac_home_head[i]=dac_home_label(dac_info_card,0,60,80,56,i==1?6:i==0?2:i==2?0:1,i==1?0xff89bdcb:i==3?0xff909da6:0xffe4eaed,"");
  dac_home_details=lv_create(dac_info_card);stylezero(dac_home_details);lv_size(dac_home_details,360,280);lv_align(dac_home_details,1,0,0);opa(dac_home_details,0,0);
  dac_home_label(dac_home_details,43,129,48,23,0,0xffa0adb5,"RAM");dac_home_ram=dac_home_label(dac_home_details,80,119,54,32,3,0xffe4eaed,"--%");
  dac_home_box(dac_home_details,43,153,91,4,0xff1d282e,2);dac_home_ram_fill=dac_home_box(dac_home_details,43,153,1,4,0xff89bdcb,2);
  dac_home_mb=dac_home_label(dac_home_details,43,163,91,23,0,0xffa0adb5,"--/-- MB");
  dac_home_label(dac_home_details,43,193,48,23,0,0xffa0adb5,"CPU");dac_home_cpu=dac_home_label(dac_home_details,80,183,54,32,3,0xffe4eaed,"--%");
  dac_home_box(dac_home_details,43,217,91,4,0xff1d282e,2);dac_home_cpu_fill=dac_home_box(dac_home_details,43,217,1,4,0xff89bdcb,2);
  dac_home_mhz=dac_home_label(dac_home_details,43,227,91,23,0,0xffa0adb5,"-- MHz");
  dac_home_label(dac_home_details,43,252,48,23,0,0xffa0adb5,"音量");dac_home_volume=dac_home_label(dac_home_details,91,243,43,33,3,0xffe4eaed,"--");
  LvObj *spectrum=dac_home_box(dac_info_card,149,120,169,130,0xff223039,12);dac_home_box(spectrum,1,1,167,128,0xff0b1319,11);
  dac_home_box(spectrum,11,117,147,1,0xff1d282e,0);
  for(unsigned i=0;i<14;i++){dac_home_bar[i]=dac_home_box(spectrum,disc_dac_home_spectrum_bar_x(i)+1,116,6,1,0xff89bdcb,2);show(dac_home_bar[i],0);dac_home_heights[i]=-1;}
  dac_home_hint=dac_home_label(spectrum,10,52,149,24,0,0xff909da6,"等待音频");textalign(dac_home_hint,2,0);
  dac_home_box(dac_info_card,68,282,224,1,0xff1d282e,0);
  dac_home_clock=dac_home_label(dac_info_card,58,289,244,50,5,0xffe4eaed,"--:--:--");textalign(dac_home_clock,2,0);
  dac_home_kind=99;dac_home_header(0,0,0);disc_dac_home_spectrum_reset(&dac_home_spectrum);
 }
 /* The native connection/format labels are still data sources for the mode
  * controller. Keep their drawing hidden throughout our DAC page, including
  * visualizations, and restore ownership only when actually leaving it. */
 if(active&&usbdac_enabled&&*(unsigned *)0x8e16d4==1&&visible){const unsigned addresses[3]={0x8de214,0x8de210,0x8de20c};for(unsigned i=0;i<3;i++){LvObj *o=LV_GLOBAL(addresses[i]);if(o&&lv_valid(o)){if(o!=dac_home_stock[i]){dac_home_stock[i]=o;dac_home_hidden[i]=*(unsigned *)((char *)o+36)&1;}show(o,0);}}}
 else dac_home_restore();
 show(dac_info_card,home);if(dac_info_card){show(dac_home_details,dac_info_enabled);show(dac_home_head[3],dac_info_enabled&&dac_home_bits&&dac_home_kind==1);}
 disc_dac_home_spectrum_gate(&dac_home_spectrum,dac_home_live,dac_home_kind,now());
}
static int dac_home_sampling(void){return dac_home_live&&disc_dac_home_spectrum_due(&dac_home_spectrum,now());}
static void dac_home_feed(const unsigned char *data,unsigned bits,unsigned channels,unsigned rate,unsigned t,unsigned samples,unsigned step){
 if(!dac_home_live)return;disc_visual_spectrum_convert(&dac_home_gain,data,bits,channels,samples,step,t,visual_normalized(),xy_pcm);
 disc_dac_home_spectrum_process(&dac_home_spectrum,xy_pcm,samples,rate/step,t);
}
static void dac_info_poll(unsigned t){
 if(!dac_home_live||!dac_info_card)return;
 disc_dac_home_spectrum_tick(&dac_home_spectrum,t);
 for(unsigned i=0;i<14;i++){int h=dac_home_spectrum.height[i];if(h==dac_home_heights[i])continue;dac_home_heights[i]=h;show(dac_home_bar[i],h!=0);if(h){lv_size(dac_home_bar[i],6,h);lv_align(dac_home_bar[i],1,disc_dac_home_spectrum_bar_x(i)+1,117-h);}}
 if(!dac_clock_at||t-dac_clock_at>=200){dac_clock_at=t;long ts[2];int tm[16];char b[24];if(!clock_gettime(0,ts)&&localtime_r(ts,tm)&&tm[2]>=0&&tm[2]<24&&tm[1]>=0&&tm[1]<60&&tm[0]>=0&&tm[0]<=60){snprintf(b,sizeof(b),"%02d:%02d:%02d",tm[2],tm[1],tm[0]);dac_home_text(dac_home_clock,b);}}
 if(dac_info_at&&t-dac_info_at<500)return;dac_info_at=t;
 char next[96],data[3072];unsigned p,h[40],mp,mode,digital,rate=0,bits=0,channels=0,kind=0;
 if(memfd<0)discover(t);
 LvObj *stock=LV_GLOBAL(0x8de210);const char *native=stock&&lv_valid(stock)?DISC_DOP_DISPLAY_TEXT(stock):0;
 if(copy(0x83dfc4,&p,4)&&copy(p,h,sizeof(h))&&copy(0x83dfc0,&mp,4)&&copy(mp,&mode,4)&&copy(0x839a74,&digital,4)&&native&&strcmp(native,"未连接")&&mode==4&&digital==1){
  if(disc_uac_format(h,mode,digital,&rate,&bits,&channels))kind=DISC_DAC_HOME_SPECTRUM_PCM;
  else if(h[11]==2){rate=h[3];kind=h[6]==0&&h[4]==2&&h[5]==32&&h[22]==1&&(rate==176400||rate==352800)?DISC_DAC_HOME_SPECTRUM_DOP:DISC_DAC_HOME_SPECTRUM_DSD;}
 }
 dac_home_header(kind,rate,bits);disc_dac_home_spectrum_gate(&dac_home_spectrum,dac_home_live,kind,t);
 unsigned volume=disc_uac_volume(*(volatile unsigned char *)0x8e1716,*(volatile unsigned char *)0x8e1717,*(volatile unsigned char *)0x8e1718);
 if(volume<=120)snprintf(next,sizeof(next),"%u",volume);else snprintf(next,sizeof(next),"--");dac_home_metric(dac_home_volume,next,243);
 unsigned v[8];int percent=-1;if(textread("/proc/stat",data,sizeof(data))&&sscanf(data,"cpu %u %u %u %u %u %u %u %u",v,v+1,v+2,v+3,v+4,v+5,v+6,v+7)==8)percent=disc_cpu_percent(&dac_cpu_sample,v);
 if(!dac_freq_at||t-dac_freq_at>=2000){dac_freq_at=t;int f=open("/dev/mem",0);if(f>=0){volatile unsigned *regs=mmap(0,4096,1,1,f,0x10000000);if(regs!=(void *)-1){dac_cached_mhz=disc_cpu_mhz(regs[0],regs[4]);munmap((void *)regs,4096);}close(f);}}
 if(percent>=0){snprintf(next,sizeof(next),"%d%%",percent);dac_home_metric(dac_home_cpu,next,183);lv_size(dac_home_cpu_fill,percent?91*percent/100:1,4);}
 if(dac_cached_mhz)snprintf(next,sizeof(next),"%u MHz",dac_cached_mhz);else snprintf(next,sizeof(next),"-- MHz");dac_home_text(dac_home_mhz,next);
 if((!dac_memory_at||t-dac_memory_at>=1000)&&textread("/proc/meminfo",data,sizeof(data))){dac_memory_at=t;unsigned total=0,available=0;int got=0;char *line=data;
  while(*line){if(sscanf(line,"MemTotal: %u",&total)!=1&&sscanf(line,"MemAvailable: %u",&available)==1)got=1;while(*line&&*line!='\n')line++;if(*line)line++;}
  if(total&&got&&available<=total){unsigned used=disc_memory_used(total,available),pc=used*100/total;snprintf(next,sizeof(next),"%u%%",pc);dac_home_metric(dac_home_ram,next,119);lv_size(dac_home_ram_fill,pc?91*pc/100:1,4);snprintf(next,sizeof(next),"%u/%u MB",(used+512)/1024,(total+512)/1024);dac_home_text(dac_home_mb,next);}
 }
}

