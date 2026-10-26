/* Factory library-row constructor, verified callback identities and UI thread. */
#define ls_native_row FN(0x46d5c0,LvObj *(*)(LvObj *,const char *,unsigned,void (*)(void *)))
static LvObj *ls_origin,*ls_entry,*ls_page,*ls_status_bar;
static unsigned ls_status_hidden,ls_status_mode;
static LibraryScope ls_editor;
static int ls_editor_ok,ls_view,ls_pending,ls_pending_index,ls_offset,ls_more,ls_count,ls_skip;
static void *ls_directory;
static char ls_current[LS_PATH],ls_selected[LS_PATH],ls_names[32][LS_PATH];
static char ls_mounts[8][LS_PATH];
static int ls_mount_count;
static const char *ls_message;
enum {LS_OPEN=1,LS_ADD,LS_MOUNT,LS_FOLDER,LS_PREV,LS_NEXT,LS_BACK,LS_SELECT,LS_REMOVE,LS_CONFIRM_ADD,LS_CONFIRM_REMOVE};
static void ls_make(int);
static void ls_finish_listing(int);
static void ls_cancel_list(void){if(ls_directory){closedir(ls_directory);ls_directory=0;}}
static void ls_deleted(void *e){LvObj *p=lv_event_target(e);if(p==ls_origin){ls_origin=ls_entry=0;}if(p==ls_page){ls_page=0;ls_cancel_list();}}
static void ls_click(void *e){
 if(!stationary_clicked(e))return;unsigned v=(unsigned long)FN(0x4e394c,void *(*)(void *))(e);ls_pending=(int)(v&255);ls_pending_index=(int)(v>>8);
}
static void ls_back(void *e){if(FN(0x488860,int (*)(void *))(e)==3)ls_pending=LS_BACK;}
static LvObj *ls_row(LvObj *p,const char *text,int action,int index){
 LvObj *o=ls_native_row(p,text,(unsigned)action|((unsigned)index<<8),ls_click);if(!o)return 0;
 /* Hide the factory radio indicator; the entire original row is retained. */
 unsigned n=child_count(o);for(unsigned i=0;i<n&&i<6;i++){LvObj *c=child(o,(int)i);if(c&&*(unsigned *)c==0x7c1d80)lv_add_flag(c,DISC_LV_HIDDEN);}
 if(action)protect_tap(o);else lv_clear_flag(o,DISC_LV_CLICKABLE);
 return o;
}
static int ls_has_event(LvObj *p,unsigned callback,unsigned data){
 unsigned *s=*(unsigned **)((char *)p+8);if(!s)return 0;unsigned count=s[8]>>16,*ev=(unsigned *)s[3];if(count>32)return 0;
 for(unsigned k=0;ev&&k<count;k++)if(ev[k*3]==callback&&ev[k*3+1]==data&&(ev[k*3+2]&255)==7)return 1;return 0;
}
static void library_scope_prepare(void){
 LvObj *p=lv_active_screen(lv_disp_default());if(!p)return;
 if(p==ls_page){toolbox_clear_native_layers();return;}
 if(ls_origin&&lv_valid(ls_origin)&&ls_entry&&lv_valid(ls_entry))return;
 /* Native update page global plus its two distinct original actions. Text is
  * not used as the identity (the factory supports several UI languages). */
 if(p!=LV_GLOBAL(0x8dd86c))return;unsigned n=child_count(p);if(n>12)return;
 int now=0,automatic=0;for(unsigned i=0;i<n;i++){LvObj *o=child(p,(int)i);if(o){now|=ls_has_event(o,0x4618a8,0);automatic|=ls_has_event(o,0x4618a8,1);}}
 if(!now||!automatic)return;
 ls_origin=p;lv_add_cb(p,ls_deleted,DISC_LV_DELETE,0);ls_entry=ls_row(p,"排除文件夹",LS_OPEN,0);
}
static LvObj *ls_screen(const char *title){
 LvObj *p=lv_create(0);if(!p)return 0;screen_register(p);lv_size(p,360,360);
 lv_radius(p,0,0);lv_border(p,0,0);lv_bg(p,0xff000000,0);bg_opa(p,255,0);
 pad_left(p,0,0);pad_right(p,0,0);pad_top(p,0,0);pad_bottom(p,0,0);
 FN(0x4f6b30,void (*)(LvObj *,int,unsigned))(p,5,0);
 FN(0x5030a0,void (*)(LvObj *,int))(p,1);FN(0x5030e4,void (*)(LvObj *,int,int,int))(p,0,0,2);
 native_title(p,title,0xff7e00);lv_add_cb(p,ls_deleted,DISC_LV_DELETE,0);
 lv_add_cb(p,ls_back,1,0);lv_add_cb(p,ls_back,2,0);lv_add_cb(p,ls_back,8,0);return p;
}
static const char *ls_display(const char *path,char *out,unsigned size){
 unsigned root=ls_root_length(path);snprintf(out,size,"%s%s",root==11&&!memcmp(path,"/tmp/sdcard",11)?"TF":"USB",root?path+root:path);return out;
}
static void ls_find_mounts(void){
 ls_mount_count=0;int fd=open("/proc/mounts",0);if(fd<0)return;char block[256],line[4096];unsigned used=0;int n,overflow=0;
 while((n=read(fd,block,sizeof block))>0)for(int j=0;j<n;j++){
  char c=block[j];if(c!='\n'){if(used<sizeof line-1)line[used++]=c;else overflow=1;continue;}
  line[used]=0;if(!overflow&&ls_mount_count<8){
   unsigned a=0;while(line[a]&&line[a]!=' ')a++;unsigned b=a+1;while(line[b]&&line[b]!=' ')b++;
   if(a<used&&b>a+1&&b-a-1<LS_PATH&&((a>=9&&!memcmp(line,"/dev/mmc",8))||(a>=7&&!memcmp(line,"/dev/sd",7)))){
    line[b]=0;char *p=line+a+1;unsigned root=ls_root_length(p);if(root&&!p[root]){snprintf(ls_mounts[ls_mount_count++],LS_PATH,"%s",p);}
   }
  }used=0;overflow=0;
 }
 close(fd);
}
static void ls_make(int view){
 ls_cancel_list();LvObj *old=ls_page,*p=ls_screen(view==0?"排除文件夹":view==1?"选择文件夹":view==2?"文件夹":view==3?"取消排除":"确认排除");if(!p)return;
 ls_page=p;ls_view=view;char text[LS_PATH+32];
 if(view==0){
  ls_editor_ok=ls_load(&ls_editor);
  ls_row(p,ls_editor_ok?"添加文件夹":"读取设置失败",ls_editor_ok?LS_ADD:0,0);
  if(ls_message)ls_row(p,ls_message,0,0);
  for(unsigned i=0;i<ls_editor.count;i++)ls_row(p,ls_display(ls_editor.path[i],text,sizeof text),LS_REMOVE,(int)i);
 }else if(view==1){
  ls_find_mounts();for(int i=0;i<ls_mount_count;i++)ls_row(p,!strcmp(ls_mounts[i],"/tmp/sdcard")?"TF 卡":ls_mounts[i]+5,LS_MOUNT,i);
  if(!ls_mount_count)ls_row(p,"未发现存储设备",0,0);
 }else if(view==2){
  ls_row(p,ls_display(ls_current,text,sizeof text),0,0);
  if(ls_valid_path(ls_current))ls_row(p,ls_excluded(&ls_editor,ls_current)?"此文件夹已被排除":"排除此文件夹",ls_excluded(&ls_editor,ls_current)?0:LS_SELECT,0);
  ls_row(p,"返回上级",LS_BACK,0);ls_row(p,"正在读取文件夹",0,0);
  ls_count=ls_more=0;ls_skip=ls_offset;ls_directory=opendir(ls_current);
  if(!ls_directory)ls_message="文件夹不可访问";
 }else{
  ls_row(p,ls_display(ls_selected,text,sizeof text),0,0);
  ls_row(p,view==3?"取消排除此文件夹":"排除此文件夹",view==3?LS_CONFIRM_REMOVE:LS_CONFIRM_ADD,0);
  ls_row(p,"包含全部子文件夹",0,0);ls_row(p,"返回",LS_BACK,0);
 }
 toolbox_clear_native_layers();screen_load(p);if(old)screen_delete(&old);
 if(view==2&&!ls_directory)ls_finish_listing(1);
}
static void ls_finish_listing(int failed){
 ls_cancel_list();LvObj *old=ls_page,*p=ls_screen("选择文件夹");if(!p)return;ls_page=p;ls_view=2;char text[LS_PATH+32];
 ls_row(p,ls_display(ls_current,text,sizeof text),0,0);
 if(ls_valid_path(ls_current))ls_row(p,ls_excluded(&ls_editor,ls_current)?"此文件夹已被排除":"排除此文件夹",ls_excluded(&ls_editor,ls_current)?0:LS_SELECT,0);
 ls_row(p,"返回上级",LS_BACK,0);
 for(int i=0;i<ls_count;i++)ls_row(p,ls_names[i],LS_FOLDER,i);
 if(failed)ls_row(p,"读取文件夹失败",0,0);else if(!ls_count)ls_row(p,"没有子文件夹",0,0);
 if(ls_offset)ls_row(p,"上一页",LS_PREV,0);if(ls_more)ls_row(p,"下一页",LS_NEXT,0);
 screen_load(p);if(old)screen_delete(&old);
}
static void ls_list_tick(void){
 if(!ls_directory)return;
 /* Bounded work on each native 200 ms poll; large TF directories yield. */
 for(int k=0;k<96;k++){
  unsigned char *e=readdir(ls_directory);if(!e){ls_finish_listing(0);return;}
  const char *name=(char *)e+11;unsigned type=e[10];
  if(!strcmp(name,".")||!strcmp(name,"..")||(type!=4&&type!=0))continue;
  char path[LS_PATH],canonical[LS_PATH];int n=snprintf(path,sizeof path,"%s/%s",ls_current,name);if(n<0||n>=(int)sizeof path||!ls_normalize(path,canonical))continue;
  if(type==0){void *d=opendir(path);if(!d)continue;closedir(d);}
  if(ls_skip){ls_skip--;continue;}
  if(ls_count==32){ls_more=1;ls_finish_listing(0);return;}
  snprintf(ls_names[ls_count++],LS_PATH,"%s",name);
 }
}
static void ls_restore_status(void){
 LvObj *b=LV_GLOBAL(0x8de408);if(b&&b==ls_status_bar&&lv_valid(b)&&*(unsigned *)0x8e16d4==ls_status_mode&&(*(unsigned *)((char *)b+36)&1)!=ls_status_hidden)FN(ls_status_hidden?0x479090:0x4790b0,void (*)(void))();
}
static void ls_close(int restore){
 ls_cancel_list();LvObj *old=ls_page;ls_page=0;if(restore&&ls_origin&&lv_valid(ls_origin)){screen_load(ls_origin);ls_restore_status();}if(old)screen_delete(&old);
}
static void ls_go_back(void){
 if(ls_view==0){ls_close(1);return;}
 if(ls_view==1||ls_view==3){ls_make(0);return;}
 if(ls_view==4){ls_make(2);return;}
 unsigned n=ls_len(ls_current),root=ls_root_length(ls_current);if(n<=root){ls_make(1);return;}
 while(n&&ls_current[n-1]!='/')n--;if(n)ls_current[n-1]=0;ls_offset=0;ls_make(2);
}
static void library_scope_poll(void){
 library_scope_prepare();if(ls_page&&lv_active_screen(lv_disp_default())!=ls_page){ls_pending=0;ls_close(0);}
 int action=ls_pending,index=ls_pending_index;ls_pending=0;
 if(action==LS_OPEN&&ls_origin&&lv_active_screen(lv_disp_default())==ls_origin){
  ls_status_bar=LV_GLOBAL(0x8de408);ls_status_mode=*(unsigned *)0x8e16d4;ls_status_hidden=ls_status_bar&&lv_valid(ls_status_bar)?*(unsigned *)((char *)ls_status_bar+36)&1:0;ls_message=0;ls_make(0);
 }else if(ls_page){
  if(action==LS_ADD&&ls_view==0&&ls_editor_ok){ls_message=0;ls_make(1);}
  else if(action==LS_MOUNT&&ls_view==1&&index>=0&&index<ls_mount_count){snprintf(ls_current,LS_PATH,"%s",ls_mounts[index]);ls_offset=0;ls_make(2);}
  else if(action==LS_FOLDER&&ls_view==2&&!ls_directory&&index>=0&&index<ls_count){
   char path[LS_PATH],canonical[4096];int n=snprintf(path,sizeof path,"%s/%s",ls_current,ls_names[index]);
   if(n>0&&n<LS_PATH&&realpath(path,canonical)&&ls_len(canonical)<LS_PATH&&ls_valid_path(canonical)&&ls_prefix(canonical,ls_current)){snprintf(ls_current,LS_PATH,"%s",canonical);ls_offset=0;ls_make(2);}
  }else if(action==LS_PREV&&ls_view==2){ls_offset=ls_offset>=32?ls_offset-32:0;ls_make(2);}
  else if(action==LS_NEXT&&ls_view==2&&ls_more){ls_offset+=32;ls_make(2);}
  else if(action==LS_BACK)ls_go_back();
  else if(action==LS_SELECT&&ls_view==2&&ls_valid_path(ls_current)){snprintf(ls_selected,LS_PATH,"%s",ls_current);ls_make(4);}
  else if(action==LS_REMOVE&&ls_view==0&&index>=0&&index<(int)ls_editor.count){snprintf(ls_selected,LS_PATH,"%s",ls_editor.path[index]);ls_make(3);}
  else if((action==LS_CONFIRM_ADD&&ls_view==4)||(action==LS_CONFIRM_REMOVE&&ls_view==3)){
   LibraryScope *next=malloc(sizeof *next);int ok=next&&ls_load(next);
   if(ok&&action==LS_CONFIRM_ADD){char canonical[4096];ok=realpath(ls_selected,canonical)&&!strcmp(canonical,ls_selected)&&ls_valid_path(canonical)&&next->count<LS_MAX&&!ls_excluded(next,canonical);if(ok)snprintf(next->path[next->count++],LS_PATH,"%s",canonical);}
   if(ok&&action==LS_CONFIRM_REMOVE){int at=ls_exact(next,ls_selected);ok=at>=0;if(ok){for(unsigned i=(unsigned)at+1;i<next->count;i++)memcpy(next->path[i-1],next->path[i],LS_PATH);next->count--;}}
   if(ok)ok=ls_save(next);free(next);ls_message=ok?0:"未保存，请重试";ls_make(0);
  }
 }
 if(ls_page&&ls_view==2&&ls_directory)ls_list_tick();
}
