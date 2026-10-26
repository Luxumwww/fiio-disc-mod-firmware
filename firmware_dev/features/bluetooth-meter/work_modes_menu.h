/* Hide only native USB AUDIO ID2. Native row indices/data remain intact. */
static LvObj *work_modes_page,*work_modes_rows[5],*work_modes_footer;
static void work_modes_deleted(void *e){if(lv_event_target(e)==work_modes_page){work_modes_page=work_modes_footer=0;for(unsigned i=0;i<5;i++)work_modes_rows[i]=0;}validation_count=0;}
static int work_modes_row_id(LvObj *o){unsigned *s=*(unsigned **)((char *)o+8);if(!s)return -1;unsigned count=s[8]>>16,*events=(unsigned *)s[3];if(count>32)return -1;
 for(unsigned i=0;events&&i<count;i++)if(events[i*3]==0x46b9cc&&(events[i*3+2]&255)==7&&events[i*3+1]<5)return (int)events[i*3+1];return -1;
}
static void work_modes_prepare(void){
 if(!active){if(work_modes_page&&lv_valid(work_modes_page)){for(unsigned i=0;i<5;i++)if(work_modes_rows[i]){show(work_modes_rows[i],1);lv_align(work_modes_rows[i],1,0,57+59*(int)i);}if(work_modes_footer)lv_align(work_modes_footer,1,0,352);}return;}
 LvObj *screen=lv_active_screen(lv_disp_default());if(!screen||!lv_valid(screen))return;unsigned count=FN(0x4f889c,unsigned (*)(LvObj *))(screen);if(count!=7)return;
 LvObj *rows[5]={0};unsigned found=0;for(unsigned i=1;i<count;i++){LvObj *row=FN(0x4f8850,LvObj *(*)(LvObj *,int))(screen,(int)i);int id=row?work_modes_row_id(row):-1;if(id>=0&&!rows[id]){rows[id]=row;found++;}}
 if(found!=5)return;
 if(screen!=work_modes_page){work_modes_page=screen;lv_add_cb(screen,work_modes_deleted,33,0);}
 for(unsigned i=0;i<5;i++){work_modes_rows[i]=rows[i];show(rows[i],i!=2);if(i!=2)lv_align(rows[i],1,0,57+59*((int)i-(i>2)));}
 work_modes_footer=FN(0x4f8850,LvObj *(*)(LvObj *,int))(screen,6);if(work_modes_footer)lv_align(work_modes_footer,1,0,293);
}
