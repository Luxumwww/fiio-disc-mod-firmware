#ifndef DISC_BT_WIFI_POLICY_H
#define DISC_BT_WIFI_POLICY_H
/* Native Wi-Fi commands stay on the UI thread. This model owns no radio API. */
typedef struct {
 unsigned active,restore,pending,manual,attempts,next;
} DiscBtWifiPolicy;
/* pending: 0 none, 1 close on entry, 2 restore on exit. */
static int disc_bt_wifi_valid(const DiscBtWifiPolicy *p){
 return p->active<=1&&p->restore<=1&&p->pending<=2&&p->manual<=1&&p->attempts<=3&&
  !(p->active&&p->pending==2)&&!(!p->active&&p->pending==1);
}
static void disc_bt_wifi_manual(DiscBtWifiPolicy *p){
 p->pending=0;p->restore=0;p->manual=1;p->attempts=0;p->next=0;
}
static int disc_bt_wifi_step(DiscBtWifiPolicy *p,unsigned mode,unsigned wifi,unsigned busy,unsigned now){
 if(mode>4||wifi>1)return -1;
 if(mode==3&&!p->active){
  unsigned inherited=p->pending==2;
  p->active=1;p->restore=wifi||inherited;p->manual=0;
  p->pending=wifi?1:0;p->attempts=0;p->next=0;
 }else if(mode!=3&&p->active){
  p->active=0;p->pending=p->restore&&!p->manual&&!wifi?2:0;
  p->restore=0;p->manual=0;p->attempts=0;p->next=0;
 }
 if(!p->pending){
  /* A later native ON is a manual override. Do not keep fighting the user. */
  if(p->active&&wifi&&!p->manual)disc_bt_wifi_manual(p);
  return -1;
 }
 unsigned want=p->pending==2;
 if(wifi==want){p->pending=0;p->attempts=0;p->next=0;return -1;}
 if(busy||p->attempts>=3||(p->next&&(int)(now-p->next)<0))return -1;
 p->attempts++;p->next=now+2000;return (int)want;
}
#endif
