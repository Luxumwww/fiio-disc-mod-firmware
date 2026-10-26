/* UI thread renders actual remote state; release does not predict success. */
#ifndef BT_BUTTON_WORD
#define BT_BUTTON_WORD(a) (*(unsigned *)(unsigned long)(a))
#define BT_BUTTON_NATIVE_EVENT(e) FN(0x477b00,void (*)(void *))(e)
#endif
static unsigned bt_button_at;static int bt_button_state=-1;
static void bt_button_render(LvObj *image,int state){
 if(!image||!lv_valid(image)||state<0||state>1)return;
 /* Verified stock PNG content: play.png = two bars; pause.png = triangle.
  * Names describe stock status artwork, not the action inferred from them. */
 const char *wanted=state==1?"D:/usr/project/config/img/common/play.png":"D:/usr/project/config/img/common/pause.png";
 const char *current=*(const char **)((char *)image+0x2c);
 if((*(unsigned short *)((char *)image+0x4e)&3)!=1||!current||strcmp(current,wanted))src(image,wanted);
}
static void bt_button_event(void *e){
 BT_BUTTON_NATIVE_EVENT(e);
 if(active&&BT_BUTTON_WORD(0x8e16d4)==3&&lv_event_code(e)==8){
  /* Native release flips the image even if no playback acknowledgement. */
  LvObj *image=LV_GLOBAL(0x8de3c8);int state=bt_button_state;
  if(state<0)state=BT_BUTTON_WORD(0x8da030)==0?1:0;
  bt_button_render(image,state);
 }
}
static void bt_button_bind(void){
 LvObj *button=LV_GLOBAL(0x8de3d4);if(!button||!lv_valid(button))return;
 typedef __UINTPTR_TYPE__ Word;Word *spec=*(Word **)((char *)button+8);if(!spec)return;
 unsigned count=(spec[8]>>16)&63;Word *events=(Word *)spec[3];if(!events||count>32)return;
 /* Only replace verified stock callbacks. XY handlers and userdata remain. */
 for(unsigned i=0;i<count;i++){unsigned code=events[i*3+2]&255;if(code!=1&&code!=2&&code!=8)continue;
  if(events[i*3]==0x477b00)events[i*3]=(Word)bt_button_event;
 }
}
static void bt_button_sync(void){
 bt_button_bind();unsigned t=now();if(t-bt_button_at<200)return;bt_button_at=t;
 char b[320];int pid,connected,state,pending,intent,attempts,result,requests,queries,events;
 if(!textread("/tmp/disc-runtime/bt-playback.state",b,sizeof(b)))return;
 if(sscanf(b,"version=1.3.32\npid=%d\nconnected=%d\nremote_state=%d\npending=%d\nintent=%d\nattempts=%d\nresult=%d\nrequests=%d\nqueries=%d\nevents=%d",&pid,&connected,&state,&pending,&intent,&attempts,&result,&requests,&queries,&events)!=10)return;
 if(pid!=playerpid)return;
 if(!connected||state<0||state>1){bt_button_state=-1;return;}
 bt_button_state=state;BT_BUTTON_WORD(0x8da030)=state==1?0:1;
 bt_button_render(LV_GLOBAL(0x8de3c8),state);
}
