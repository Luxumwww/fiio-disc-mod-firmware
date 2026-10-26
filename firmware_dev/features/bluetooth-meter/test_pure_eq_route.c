#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef void LvObj;
static unsigned mode,editing;static int original_calls,constructor_calls,fallback_calls,other_calls,skip_native;
static unsigned char origin[96],second[96],eq[96],grid[96],stock_button[96],statbar[96];
static unsigned origin_spec[12],origin_events[12],stock_button_spec[12],stock_button_events[12];
static LvObj *current=origin,*eq_global;static int origin_valid=1;
static void (*native_return)(void);
struct event {LvObj *target;unsigned code;};
static LvObj *lv_event_target(void *e){return ((struct event *)e)->target;}
static LvObj *lv_active_screen(void *d){(void)d;return current;}
static void *lv_disp_default(void){return grid;}
static int lv_valid(LvObj *o){return o&&!(o==origin&&!origin_valid);}
static LvObj *parent(LvObj *o){return o==stock_button?grid:0;}
static void load(LvObj *o){current=o;}
static void home(void){fallback_calls++;current=second;}
static void eq_return(void){other_calls++;}
static void original(void *e){(void)e;original_calls++;if(!skip_native){eq_global=eq;current=eq;*(unsigned *)(void *)(statbar+36)|=1;}}
static void hide_status(void){*(unsigned *)(void *)(statbar+36)|=1;}
static void show_status(void){*(unsigned *)(void *)(statbar+36)&=~1u;}
static void construct(void (*back)(void)){constructor_calls++;native_return=back;current=eq_global;}
static LvObj *child(LvObj *g,int i){(void)g;assert(i==1);return stock_button;}
static void lv_add_cb(LvObj *o,void (*cb)(void *),unsigned code,void *data){
 assert(o==origin);unsigned n=origin_spec[8]>>16;assert(n<4);origin_events[n*3]=(unsigned)(uintptr_t)cb;origin_events[n*3+1]=(unsigned)(uintptr_t)data;origin_events[n*3+2]=code;origin_spec[8]=(n+1)<<16;
}
static void *fn(unsigned a){switch(a){case 0x479090:return hide_status;case 0x4790b0:return show_status;case 0x4e2734:return load;case 0x435bf0:return home;case 0x480228:return original;case 0x4670ac:return construct;case 0x4f8850:return child;}assert(0);return 0;}
#define FN(a,t) ((t)fn(a))
#define LV_GLOBAL(a) ((a)==0x8de408?statbar:(a)==0x8deed0?stock_button:eq_global)
#define pure_editing editing
static unsigned char mode_bytes[4];
#include "reports/pure-eq-127/toolbar_status_test.h"
/* Redirect only the native mode global in the included production header. */
#include "reports/pure-eq-127/pure_eq_route_test.h"
int main(void){
 memcpy(origin+8,&(unsigned *){origin_spec},sizeof(void *));origin_spec[3]=(unsigned)(uintptr_t)origin_events;
 memcpy(stock_button+8,&(unsigned *){stock_button_spec},sizeof(void *));stock_button_spec[3]=(unsigned)(uintptr_t)stock_button_events;stock_button_spec[8]=2<<16;
 stock_button_events[0]=0x480228;stock_button_events[2]=7;stock_button_events[3]=0x47f8f0;stock_button_events[5]=5;
 pure_eq_bind(grid);assert(stock_button_events[0]==(unsigned)(uintptr_t)pure_eq_click&&stock_button_events[3]==0x47f8f0);
 struct event e={stock_button,7};mode=0;memcpy(mode_bytes,&mode,4);
 for(int i=0;i<1000;i++){current=origin;pure_eq_click(&e);assert(current==eq&&pure_eq_origin==origin);native_return();assert(current==origin&&!pure_eq_origin&&!(*(unsigned *)(void *)(statbar+36)&1));}
 assert(origin_spec[8]>>16==1&&original_calls==1000&&constructor_calls==1000);
 editing=1;pure_eq_click(&e);assert(original_calls==1000);editing=0;
 skip_native=1;current=origin;pure_eq_click(&e);assert(current==origin&&constructor_calls==1000);skip_native=0;
 pure_eq_click(&e);struct event del={origin,33};pure_eq_origin_deleted(&del);origin_valid=0;native_return();assert(current==second&&fallback_calls==1);
 origin_valid=1;current=origin;pure_eq_click(&e);mode=3;memcpy(mode_bytes,&mode,4);native_return();assert(other_calls==1);
 puts("PASS 1000 origin returns; no duplicate watchers; native long press/edit guard; source deletion fallback; mode change fallback");
}
