#ifndef DISC_SETTINGS_UI_API_H
#define DISC_SETTINGS_UI_API_H
/* Verified original V2.57 LVGL ABI only. All used text entries are guarded. */
typedef void LvObj;
#define LV_PTR(a,t) ((t)(unsigned long)(a))
#define lv_valid LV_PTR(0x4e9a70,int (*)(const LvObj *))
#define lv_create LV_PTR(0x4e969c,LvObj *(*)(LvObj *))
#define lv_btn LV_PTR(0x52a8a4,LvObj *(*)(LvObj *))
#define lv_label LV_PTR(0x52df64,LvObj *(*)(LvObj *))
#define lv_text LV_PTR(0x52f2bc,void (*)(LvObj *,const char *))
#define lv_size LV_PTR(0x4eb680,void (*)(LvObj *,int,int))
#define lv_align LV_PTR(0x4ebc14,void (*)(LvObj *,int,int,int))
#define lv_add_flag LV_PTR(0x4e96d0,void (*)(LvObj *,unsigned))
#define lv_clear_flag LV_PTR(0x4e97f8,void (*)(LvObj *,unsigned))
#define lv_add_style LV_PTR(0x4f45d0,void (*)(LvObj *,const void *,unsigned))
#define lv_color LV_PTR(0x4f6df0,void (*)(LvObj *,unsigned,unsigned))
#define lv_bg LV_PTR(0x4f6b50,void (*)(LvObj *,unsigned,unsigned))
#define lv_radius LV_PTR(0x4f6e60,void (*)(LvObj *,int,unsigned))
#define lv_border LV_PTR(0x4f6c40,void (*)(LvObj *,int,unsigned))
#define lv_add_cb LV_PTR(0x4e39ec,void *(*)(LvObj *,void (*)(void *),unsigned,void *))
#define lv_event_code LV_PTR(0x4e3934,unsigned (*)(const void *))
#define lv_event_target LV_PTR(0x4e3924,LvObj *(*)(const void *))
#define lv_disp_default LV_PTR(0x51a7e0,void *(*)(void))
#define lv_active_screen LV_PTR(0x4e26b4,LvObj *(*)(void *))
#define LV_GLOBAL(a) (*(LvObj **)(unsigned long)(a))
enum {DISC_LV_HIDDEN=1,DISC_LV_CLICKABLE=2,DISC_LV_SCROLLABLE=16,DISC_LV_CLICKED=7,DISC_LV_DELETE=33};
#endif
