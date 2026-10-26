/* Included by settings.c after its reviewed hardware bridge and UI helpers.
 * Setting belongs to the integrated host; no worker or audio hook is added. */
#include "whitepoint_config.h"
extern int *__errno_location(void);
static int ds_config_loaded,ds_reduction,ds_applied=-1,ds_apply_failed,ds_save_failed,ds_bad_config;
static unsigned ds_retry_at,ds_check_at;
static const char ds_config_path[]="/usr/data/disc-runtime/module-data/disc-bt-meter/whitepoint.conf";
static void ds_load_config(void){
 if(ds_config_loaded)return;ds_config_loaded=1;
 char b[64];int fd=open(ds_config_path,0);if(fd<0){if(*__errno_location()!=2)ds_bad_config=1;return;}
 int n=read(fd,b,sizeof b);close(fd);
 if(n<=0||!ds_parse_config(b,(unsigned)n,&ds_reduction)){ds_reduction=0;ds_bad_config=1;}
}
static int ds_get_reduction(void){ds_load_config();return ds_reduction;}
static const char *ds_error_text(void){
 if(ds_apply_failed)return "显示调节未就绪，请重试";
 if(ds_save_failed)return "保存失败，请重试";
 if(ds_bad_config)return "原设置读取失败，请重新选择";
 return "";
}
static int ds_write_config(int reduction){
 char b[64];int n=snprintf(b,sizeof b,"version=1\nreduction=%d\n",reduction);
 const char *next="/usr/data/disc-runtime/module-data/disc-bt-meter/whitepoint.conf.new";
 int fd=open(next,1|0x100|0x200,0600);if(fd<0)return 0;
 int ok=write(fd,b,(unsigned)n)==n;
 if(ok)ok=fsync(fd)==0;close(fd);
 if(ok)ok=rename(next,ds_config_path)==0;if(!ok)unlink(next);return ok;
}
static int ds_set_reduction(int reduction){
 ds_load_config();if(reduction<0||reduction>DS_WHITEPOINT_MAX)return 0;
 int previous=ds_reduction;
 if(!hw_whitepoint_set||hw_whitepoint_set((unsigned)reduction)!=reduction){ds_apply_failed=1;return 0;}
 ds_apply_failed=0;ds_applied=reduction;
 if(!ds_write_config(reduction)){
  ds_save_failed=1;
  if(hw_whitepoint_set((unsigned)previous)!=previous){ds_apply_failed=1;ds_applied=-1;}else ds_applied=previous;
  return 0;
 }
 ds_reduction=reduction;ds_save_failed=ds_bad_config=0;return 1;
}
static void ds_whitepoint_poll(void){
 ds_load_config();if(!hw_whitepoint_set)return;
 unsigned now=milliseconds();
 if(ds_applied==ds_reduction&&!ds_apply_failed){
  if(ds_check_at&&now-ds_check_at<1000)return;ds_check_at=now;
  if(hw_whitepoint_get&&hw_whitepoint_get()==ds_reduction)return;
  ds_applied=-1;
 }
 if(ds_retry_at&&now-ds_retry_at<1000)return;ds_retry_at=now;
 int actual=hw_whitepoint_set((unsigned)ds_reduction);
 ds_apply_failed=actual!=ds_reduction;if(!ds_apply_failed)ds_applied=actual;
}
