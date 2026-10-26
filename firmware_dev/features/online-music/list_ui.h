/* A logical long list with thirty recycled native rows. Append responses keep
 * the same screen/scroll position. Each item retains its own backend token. */
static unsigned online_response_total;
static char online_drawn_tail[100];
static void online_virtual_refresh(int force){
 OnlineCatalog *cat=online_catalog();if(!cat||!online_page||!online_row_title[0])return;
 int scroll=online_scroll_position();int base=online_view==5?154:108;
 unsigned first=scroll>base+288?(unsigned)((scroll-base)/72-4):0;first=first/10*10;
 if(cat->count<=30)first=0;else if(first>cat->count-30)first=cat->count-30;
 if(force||first!=cat->first){online_drawn_tail[0]=0;cat->first=first;online_thumb_loaded=0;online_art_generation=online_art_generation_next=0;
  online_count=cat->count-first;if(online_count>30)online_count=30;
  for(unsigned i=0;i<30;i++){
   LvObj *row=online_actions[i];if(!row)continue;
   if(i>=online_count){lv_add_flag(row,1);online_results[i][0]=0;continue;}
   const OnlineItem *item=&cat->items[first+i];online_row_generation[i]=item->generation;online_row_index[i]=item->index;
   snprintf(online_results[i],804,"%s",item->text);char title[404],artist[404];unsigned n=0;
   while(item->text[n]&&item->text[n]!='\n'&&n<403){title[n]=item->text[n];n++;}title[n]=0;snprintf(artist,404,"%s",item->text[n]=='\n'?item->text+n+1:"");
   lv_text(online_row_title[i],title);lv_text(online_row_detail[i],artist);
   if(online_view==5){online_marquee(online_row_title[i],title,184);online_marquee(online_row_detail[i],artist,184);}
   lv_align(row,1,30,base+(int)(first+i)*72);lv_clear_flag(row,1);if(online_thumbs[i])lv_add_flag(online_thumbs[i],1);
  }
 }
 if(online_list_tail){lv_align(online_list_tail,1,80,base+(int)cat->count*72+8);char text[100];
  if(online_loading_more)snprintf(text,100,"正在加载…");else if(online_load_failed)snprintf(text,100,"加载失败 · 点此重试");else if(cat->more)snprintf(text,100,"继续向下 · 已加载 %u",cat->count);else snprintf(text,100,"共 %u %s",cat->count,online_view==4?"个歌单":"首歌曲");
  LvObj *label=child(online_list_tail,0);if(label&&strcmp(online_drawn_tail,text)){lv_text(label,text);snprintf(online_drawn_tail,100,"%s",text);}
 }
 if(online_list_batch)lv_align(online_list_batch,1,106,base+(int)cat->count*72+60);
}
static void online_restore_catalog(int view){
 OnlineCatalog *cat=&online_catalogs[view==4?0:1];online_source=cat->source;online_list_favorite=cat->favorite;online_library_recommend=cat->recommend;online_more=cat->more;
 if(view==4){online_list_page=cat->page;snprintf(online_list_title,320,"%s",cat->title);}else{online_track_page=cat->page;snprintf(online_collection_title,320,"%s",cat->title);}
 online_make(view);FN(0x4f15ac,void (*)(LvObj *,int,int,int))(online_page,0,-cat->scroll,0);online_virtual_refresh(1);
}
static void online_catalog_accept(int view,unsigned generation,const char *title){
 OnlineCatalog *cat=&online_catalogs[view==4?0:1];unsigned page=view==4?online_list_page:online_track_page,count=online_count;
 int append=online_loading_more&&online_view==view&&page==cat->page+1;
 if(!append){cat->count=0;cat->scroll=0;cat->source=online_source;cat->favorite=online_list_favorite;cat->recommend=online_library_recommend;}
 if(cat->count+count>10000){online_loading_more=0;online_load_failed=1;if(online_status)lv_text(online_status,"目录过大，请使用搜索定位");return;}
 if(cat->capacity<cat->count+count){unsigned capacity=cat->capacity?cat->capacity*2:64;while(capacity<cat->count+count)capacity*=2;if(capacity>10000)capacity=10000;
  OnlineItem *items=realloc(cat->items,capacity*(unsigned)sizeof(OnlineItem));if(!items){online_loading_more=0;online_load_failed=1;if(online_status)lv_text(online_status,"内存不足，暂缓加载");return;}cat->items=items;cat->capacity=capacity;
 }
 for(unsigned i=0;i<count;i++){OnlineItem *item=&cat->items[cat->count++];item->generation=generation;item->index=i;item->page=page;snprintf(item->text,804,"%s",online_results[i]);}
 cat->page=page;cat->total=online_response_total;cat->more=online_more;snprintf(cat->title,320,"%s",title);online_loading_more=online_load_failed=0;
 if(append){cat->first=~0u;online_virtual_refresh(1);}else online_make(view);
}
static void online_list_continue(void){
 OnlineCatalog *cat=online_catalog();if(!cat||online_seq||online_loading_more||online_load_failed||!cat->more||!online_page)return;
 int base=online_view==5?154:108,scroll=online_scroll_position();
 if(scroll+520<base+(int)cat->count*72)return;
 cat->scroll=scroll;online_loading_more=1;
 if(!online_library_send(online_view==4?"lists":cat->favorite?"favorites":"tracks",cat->page+1)){online_loading_more=0;online_load_failed=1;}
}
static void online_download_confirmed(unsigned yes){if(yes)online_download_pending=1;else if(online_page)screen_load(online_page);}
static void online_request_download(int current){
 online_download_current=current;online_download_return_view=online_view;
 FN(0x483170,void (*)(void (*)(unsigned),unsigned))(online_download_confirmed,4);
 LvObj *dialog=LV_GLOBAL(0x8def44);if(dialog&&lv_valid(dialog)){LvObj *label=child(dialog,0);if(label){const char *dirs[3]={"网易云","QQ音乐","酷狗音乐"};char prompt[160];snprintf(prompt,160,"确认下载？\n保存到流媒体下载/%s",dirs[online_source]);lv_text(label,prompt);}}
}
static void online_download_return(void){
 int view=online_download_return_view;
 if(view==4||view==5)online_restore_catalog(view);else if(view==2||view==0)online_return_browser();else online_make(view==14||view==10||view==9||view==8?view:8);
}
static void online_open_batch(void){
 online_save_browser();online_player_origin_view=online_view;online_selected_mask=0;
 for(unsigned i=0;i<online_count;i++){online_batch_generation[i]=online_view==5?online_row_generation[i]:online_generation;online_batch_index[i]=online_view==5?online_row_index[i]:i;}
 online_make(10);
}
