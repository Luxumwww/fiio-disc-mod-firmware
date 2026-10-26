/* Native fonts and large touch targets shared by online pages only. */
#include "icons.h"
static void online_font(LvObj *o,unsigned size){void **f=FN(0x48831c,void **(*)(unsigned))(size);if(f&&*f)lv_add_style(o,*f,0);}
/* Verified native lv_txt_get_size ABI from label refresh at 0x52ea6c.
 * Only horizontal overflow enables circular scrolling; short text stays CLIP,
 * even when a fixed row is shorter than the font's native line height. */
static void online_marquee(LvObj *o,const char *text,int width){
 void *font=0;int letter=0,line=0,measured[2]={0,0};
 FN(0x4f3f10,void (*)(void *,LvObj *,unsigned,unsigned))(&font,o,0,0x57);
 FN(0x4f3f10,void (*)(void *,LvObj *,unsigned,unsigned))(&letter,o,0,0x58);
 FN(0x4f3f10,void (*)(void *,LvObj *,unsigned,unsigned))(&line,o,0,0x59);
 if(font)FN(0x52607c,void (*)(int *,const char *,const void *,int,int,int,unsigned))(measured,text,font,letter,line,0x1fffffff,2);
 FN(0x52f4f0,void (*)(LvObj *,int))(o,measured[0]>width?3:4);
}
static LvObj *online_label(LvObj *parent,const char *text,int x,int y,int w,int h,unsigned font,unsigned color,int center){
 LvObj *o=lv_label(parent);online_font(o,font);lv_size(o,w,h);lv_text(o,text);lv_color(o,color,0);
 FN(0x52f4f0,void (*)(LvObj *,int))(o,4);FN(0x4f6e50,void (*)(LvObj *,int,unsigned))(o,center?2:0,0);
 lv_align(o,1,x,y);lv_clear_flag(o,2|16);return o;
}
static LvObj *online_image(LvObj *parent,const OnlineImage *image,int x,int y){LvObj *o=FN(0x52cb10,LvObj *(*)(LvObj *))(parent);FN(0x52cb44,void (*)(LvObj *,const void *))(o,image);lv_align(o,1,x,y);lv_clear_flag(o,2|16);return o;}
static void online_button_style(LvObj *o){lv_border(o,0,0);pad_top(o,0,0);pad_bottom(o,0,0);pad_left(o,0,0);pad_right(o,0,0);lv_radius(o,18,0);lv_bg(o,0xff171e2a,0);}
static void online_card_text(LvObj *b,const char *title,const char *detail,const OnlineImage *icon,unsigned color){
 LvObj *t=child(b,0);online_font(t,20);lv_size(t,206,27);lv_align(t,1,64,10);FN(0x4f6e50,void (*)(LvObj *,int,unsigned))(t,0,0);lv_text(t,title);
 online_label(b,detail,64,39,206,21,16,0xffadb5c4,0);
 if(icon){LvObj *tile=lv_create(b);online_button_style(tile);lv_size(tile,44,44);lv_align(tile,1,10,12);lv_bg(tile,color,0);lv_clear_flag(tile,2|16);online_image(tile,icon,6,6);}
 online_label(b,">",275,22,14,22,18,0xffadb5c4,1);
}
