/* The UI owns only the request/lease/mode words. PLAYER replaces the inode
 * at restart; old mappings remain valid and expire rather than being shrunk. */
extern void *mmap(void *,unsigned,int,int,int,long);
extern int munmap(void *,unsigned);
extern long lseek(int,long,int);
static DiscVisualRing *vs_ui;
static unsigned vs_ui_checked,vs_ui_lease,vs_ui_last_sequence;
static int vs_ui_wanted,vs_ui_mode;
static DiscVisualClock vs_ui_clock;
static void vs_ui_release(void){
 if(vs_ui){vs_ui->request=(vs_ui->request+2)&~1u;__sync_synchronize();munmap(vs_ui,sizeof(*vs_ui));vs_ui=0;}
 vs_ui_wanted=vs_ui_mode=0;vs_ui_last_sequence=0;vs_ui_clock=(DiscVisualClock){0};
}
static void vs_ui_update(unsigned t,int wanted,unsigned mode){
 if(vs_ui&&vs_ui->owner!=(unsigned)playerpid)vs_ui_release();
 if(!wanted){if(vs_ui&&vs_ui_wanted){vs_ui->request=(vs_ui->request+2)&~1u;__sync_synchronize();}vs_ui_wanted=0;return;}
 if(!vs_ui&&playerpid>0&&(!vs_ui_checked||(unsigned)(t-vs_ui_checked)>=1000)){
  vs_ui_checked=t;int fd=open(DISC_VS_PATH,2);
  if(fd>=0){long bytes=lseek(fd,0,2);DiscVisualRing *r=bytes==(long)sizeof(DiscVisualRing)?mmap(0,sizeof(*r),3,1,fd,0):(void *)-1;close(fd);
   if(r!=(void *)-1){if(r->magic==DISC_VS_MAGIC&&r->abi==1&&r->bytes==sizeof(*r)&&r->owner==(unsigned)playerpid)vs_ui=r;else munmap(r,sizeof(*r));}
  }
 }
 if(!vs_ui)return;
 if(!vs_ui_wanted||vs_ui_mode!=(int)mode){
  vs_ui->ui_mode=mode;vs_ui->lease=t;__sync_synchronize();vs_ui->request=((vs_ui->request+2)&~1u)|1u;
  vs_ui_mode=(int)mode;vs_ui_wanted=1;vs_ui_last_sequence=0;vs_ui_lease=t;vs_ui_clock=(DiscVisualClock){0};
 }else if((unsigned)(t-vs_ui_lease)>=200){vs_ui->lease=t;__sync_synchronize();vs_ui_lease=t;}
}
