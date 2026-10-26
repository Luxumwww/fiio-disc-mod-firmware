#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
static char disk[64];static int disk_size=-1,write_bad,sync_bad,rename_bad,hardware_bad,hw,writes,sets,open_bad,rollback_bad;
static unsigned clock_ms;
static int fake_open(const char *p,int flags,...){(void)p;if(flags)return 2;if(open_bad){errno=EACCES;return -1;}if(disk_size<0){errno=ENOENT;return -1;}return 1;}
static int fake_close(int fd){(void)fd;return 0;}
static int fake_read(int fd,void *b,unsigned n){(void)fd;if(disk_size<0)return -1;if(n>(unsigned)disk_size)n=(unsigned)disk_size;memcpy(b,disk,n);return (int)n;}
static int fake_write(int fd,const void *b,unsigned n){(void)fd;(void)b;writes++;return write_bad?-1:(int)n;}
static int fake_sync(int fd){(void)fd;return sync_bad?-1:0;}
static int fake_rename(const char *a,const char *b){(void)a;(void)b;return rename_bad?-1:0;}
static int fake_unlink(const char *p){(void)p;return 0;}
static unsigned milliseconds(void){return clock_ms;}
static int set_hardware(unsigned p){sets++;if(hardware_bad||(rollback_bad&&p==30u))return -1;hw=(int)p;return hw;}
static int get_hardware(void){return hardware_bad?-1:hw;}
static int (*hw_whitepoint_set)(unsigned)=set_hardware;
static int (*hw_whitepoint_get)(void)=get_hardware;
#define open fake_open
#define close fake_close
#define read fake_read
#define write fake_write
#define fsync fake_sync
#define rename fake_rename
#define unlink fake_unlink
#include "whitepoint_control.h"
static void reset(void){
 ds_config_loaded=ds_reduction=ds_apply_failed=ds_save_failed=ds_bad_config=0;ds_applied=-1;ds_retry_at=ds_check_at=0;
 disk_size=-1;write_bad=sync_bad=rename_bad=hardware_bad=hw=writes=sets=open_bad=rollback_bad=0;clock_ms=10;hw_whitepoint_set=set_hardware;
}
int main(void){
 reset();assert(ds_get_reduction()==0&&hw==0&&writes==0);
 assert(ds_set_reduction(40)&&ds_get_reduction()==40&&hw==40&&writes==1);
 assert(!ds_set_reduction(-1)&&!ds_set_reduction(81)&&hw==40);
 for(int kind=0;kind<3;kind++){
  reset();assert(ds_set_reduction(30));write_bad=kind==0;sync_bad=kind==1;rename_bad=kind==2;
  assert(!ds_set_reduction(60)&&ds_get_reduction()==30&&hw==30&&ds_save_failed);
 }
 reset();hardware_bad=1;assert(!ds_set_reduction(40)&&ds_get_reduction()==0&&writes==0&&ds_apply_failed);
 reset();assert(ds_set_reduction(30));write_bad=rollback_bad=1;assert(!ds_set_reduction(60)&&ds_get_reduction()==30&&ds_applied==-1&&ds_apply_failed);
 write_bad=rollback_bad=0;clock_ms+=1001;ds_whitepoint_poll();assert(hw==30&&ds_applied==30&&!ds_apply_failed);
 reset();hw_whitepoint_set=0;assert(!ds_set_reduction(40)&&writes==0&&ds_apply_failed);
 reset();strcpy(disk,"version=1\nreduction=70\n");disk_size=(int)strlen(disk);assert(ds_get_reduction()==70&&hw==0);
 ds_whitepoint_poll();assert(hw==70&&writes==0&&ds_applied==70);
 int previous=sets;clock_ms+=200;ds_whitepoint_poll();assert(sets==previous);
 hw=0;clock_ms+=1001;ds_whitepoint_poll();assert(hw==70&&sets==previous+1&&writes==0);
 reset();strcpy(disk,"version=1\nreduction=99\n");disk_size=(int)strlen(disk);assert(ds_get_reduction()==0&&ds_bad_config&&writes==0);
 assert(ds_set_reduction(20)&&!ds_bad_config&&!ds_error_text()[0]);
 reset();open_bad=1;assert(ds_get_reduction()==0&&ds_bad_config&&writes==0);
 puts("PASS: defaults, restore, real application failure, all save failure rollbacks, unavailable hardware, invalid data and no periodic flash writes");
}
